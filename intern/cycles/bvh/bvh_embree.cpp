/*
 * Copyright 2017, Blender Foundation.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifdef WITH_EMBREE

#include "bvh_embree.h"

#include "util_types.h"
#include "mesh.h"
#include "object.h"
#include "util_progress.h"
#include "util_foreach.h"

#include "embree2/rtcore_geometry.h"
#include "bvh/bvh_embree_traversal.h"

#include "xmmintrin.h"
#include "pmmintrin.h"

// #define EMBREE_CURVES
#define EMBREE_SHARED_MEM 1

CCL_NAMESPACE_BEGIN

void cclFilterFunc(void* userDataPtr, RTCRay& ray)
{
	return;
}

/* This is to have a shared device between all BVH instances */
RTCDevice BVHEmbree::rtc_shared_device = NULL;
int BVHEmbree::rtc_shared_users = 0;
thread_mutex BVHEmbree::rtc_shared_mutex;

BVHEmbree::BVHEmbree(const BVHParams& params_, const vector<Object*>& objects_)
: BVH(params_, objects_), scene(NULL)
{
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
	thread_scoped_lock lock(rtc_shared_mutex);
	if(rtc_shared_users == 0) {
		rtc_shared_device = rtcNewDevice("verbose=1");
	}
	rtc_shared_users++;

	/* BVH_CUSTOM as root index signals to the rest of the code that this is not Cycle's own BVH */
	pack.root_index = BVH_CUSTOM;
}

BVHEmbree::~BVHEmbree()
{
	delete_rtcScene();
	thread_scoped_lock lock(rtc_shared_mutex);
	rtc_shared_users--;
	if(rtc_shared_users == 0) {
		rtcDeleteDevice(rtc_shared_device);
		rtc_shared_device = NULL;
	}
}

void BVHEmbree::delete_rtcScene()
{
	if(scene) {
		rtcDeleteScene(scene);
		scene = NULL;
	}
}

void BVHEmbree::build(Progress& progress)
{
	progress.set_substatus("Building BVH");

	if (scene) {
		rtcDeleteScene(scene);
		scene = NULL;
	}

	scene = rtcDeviceNewScene(rtc_shared_device, RTC_SCENE_STATIC|RTC_SCENE_INCOHERENT|RTC_SCENE_HIGH_QUALITY|RTC_SCENE_ROBUST, RTC_INTERSECT1);

	int i = 0;

	pack.object_node.clear();

	foreach(Object *ob, objects) {
		unsigned geom_id;

		size_t prim_index_offset = pack.prim_index.size();
		if(params.top_level) {
			if(!ob->is_traceable()) {
				++i;
				continue;
			}
			if(!ob->mesh->is_instanced())
				geom_id = add_mesh(ob->mesh, i);
			else
				geom_id = add_instance(ob, i);

		}
		else
			geom_id = add_mesh(ob->mesh, i);

		if(geom_id != RTC_INVALID_GEOMETRY_ID) {
			if(!ob->mesh->is_instanced()) {
				rtcSetUserData(scene, geom_id, (void*)prim_index_offset);
				rtcSetIntersectionFilterFunction(scene, geom_id, cclFilterFunc);
			}
			else {
				rtcSetUserData(scene, geom_id, (void*)0);
			}
		}
		i++;

		if(progress.get_cancel()) return;
	}

	if(progress.get_cancel()) {
		delete_rtcScene();
		return;
	}

	progress.set_substatus("Building embree acceleration structure");
	rtcCommit(scene);

	pack_primitives();

	if(progress.get_cancel()) {
		delete_rtcScene();
		return;
	}

	progress.set_substatus("Packing geometry");
	pack_nodes(NULL);
}

unsigned BVHEmbree::add_mesh(Mesh *mesh, int i)
{
	unsigned geom_id = RTC_INVALID_GEOMETRY_ID;
	if(params.primitive_mask & PRIMITIVE_ALL_TRIANGLE && mesh->num_triangles() > 0) {
		geom_id = add_triangles(mesh, i);
	}
	if(params.primitive_mask & PRIMITIVE_ALL_CURVE && mesh->num_curves() > 0) {
		geom_id = add_curves(mesh, i);
	}
	return geom_id;
}

unsigned BVHEmbree::add_instance(Object *ob, int i)
{
	if(!ob || !ob->mesh) {
		assert(0);
		return RTC_INVALID_GEOMETRY_ID;
	}
	BVHEmbree *instance_bvh = (BVHEmbree*)(ob->mesh->bvh);

	const size_t num_motion_steps = ob->use_motion ? 3 : 1;
	unsigned geom_id = rtcNewInstance3(scene, instance_bvh->scene, num_motion_steps, i*2);

	if(ob->use_motion) {
		rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->motion.pre, 0);
		rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->tfm, 1);
		rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->motion.post, 2);
	} else {
		rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->tfm);
	}

	pack.prim_index.push_back_slow(-1);
	pack.prim_object.push_back_slow(i);
	pack.prim_type.push_back_slow(PRIMITIVE_NONE);
	pack.prim_tri_index.push_back_slow(-1);
	return geom_id;
}

unsigned BVHEmbree::add_triangles(Mesh *mesh, int i)
{
	const Attribute *attr_mP = NULL;
	size_t num_motion_steps = 1;
	size_t t_mid = 0;
	if(mesh->has_motion_blur()) {
		attr_mP = mesh->attributes.find(ATTR_STD_MOTION_VERTEX_POSITION);
		num_motion_steps = mesh->motion_steps;
		t_mid = (num_motion_steps - 1) / 2;
		if(num_motion_steps > RTC_MAX_TIME_STEPS) {
			assert(0);
			num_motion_steps = RTC_MAX_TIME_STEPS;
		}
	}

	const size_t num_triangles = mesh->num_triangles();
	const size_t num_verts = mesh->verts.size();
	unsigned geom_id = rtcNewTriangleMesh2(scene,
						RTC_GEOMETRY_STATIC,
						num_triangles,
						num_verts,
						num_motion_steps,
						i*2);

#ifdef EMBREE_SHARED_MEM
	// embree and Cycles use the same memory layout, so we can conveniently use the rtcSetBuffer2 calls
	rtcSetBuffer2(scene, geom_id, RTC_INDEX_BUFFER, &mesh->triangles[0], 0, sizeof(int) * 3);
#else
	void* raw_buffer = rtcMapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
	unsigned *rtc_indices = (unsigned*) raw_buffer;
	for(size_t j = 0; j < num_triangles; j++) {
		Mesh::Triangle t = mesh->get_triangle(j);
		rtc_indices[j*3] = t.v[0];
		rtc_indices[j*3+1] = t.v[1];
		rtc_indices[j*3+2] = t.v[2];
	}
	rtcUnmapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
#endif

	for(int t = 0; t < num_motion_steps; t++) {
		RTCBufferType buffer_type = (RTCBufferType)(RTC_VERTEX_BUFFER+t);
		const float3 *verts;
		if(t == t_mid) {
			verts = &mesh->verts[0];
		} else {
			int t_ = (t > t_mid) ? (t - 1) : t;
			verts = &attr_mP->data_float3()[t_ * num_verts];
		}
#ifdef EMBREE_SHARED_MEM
		rtcSetBuffer(scene, geom_id, buffer_type, verts, 0, sizeof(float3));
#else
		raw_buffer = rtcMapBuffer(scene, geom_id, buffer_type);
		float *rtc_verts = (float*) raw_buffer;
		for(size_t j = 0; j < num_verts; j++) {
			rtc_verts[0] = verts[j].x;
			rtc_verts[1] = verts[j].y;
			rtc_verts[2] = verts[j].z;
			rtc_verts[3] = 0.0f;
			rtc_verts += 4;
		}
		rtcUnmapBuffer(scene, geom_id, buffer_type);
#endif
	}

	pack.prim_object.reserve(pack.prim_object.size() + num_triangles);
	pack.prim_type.reserve(pack.prim_type.size() + num_triangles);
	pack.prim_index.reserve(pack.prim_index.size() + num_triangles);
	pack.prim_tri_index.reserve(pack.prim_index.size() + num_triangles);
	for(size_t j = 0; j < num_triangles; j++) {
		pack.prim_object.push_back_reserved(i);
		pack.prim_type.push_back_reserved(PRIMITIVE_TRIANGLE);
		pack.prim_index.push_back_reserved(j);
		pack.prim_tri_index.push_back_reserved(j);
	}

	return geom_id;
}

unsigned BVHEmbree::add_curves(Mesh *mesh, int i)
{
#ifndef EMBREE_CURVES
	return RTC_INVALID_GEOMETRY_ID;
#endif
	const Attribute *attr_mP = NULL;
	size_t num_motion_steps = 1;
	if(mesh->has_motion_blur()) {
		attr_mP = mesh->attributes.find(ATTR_STD_MOTION_VERTEX_POSITION);
		num_motion_steps = mesh->motion_steps;
	}

	const size_t num_curves = mesh->num_curves();
	size_t num_segments = 0;
	for(size_t j = 0; j < num_curves; j++) {
		Mesh::Curve c = mesh->get_curve(j);
		num_segments += c.num_segments();
	}
	const size_t num_keys = mesh->curve_keys.size();
	unsigned geom_id = rtcNewBezierHairGeometry2(scene,
												 RTC_GEOMETRY_STATIC,
												 num_curves,
												 num_keys,
												 num_motion_steps,
												 i*2+1);

	void* raw_buffer = rtcMapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
	unsigned *rtc_indices = (unsigned*) raw_buffer;
	size_t rtc_index = 0;
	for(size_t j = 0; j < num_curves; j++) {
		Mesh::Curve c = mesh->get_curve(j);
				rtc_indices[rtc_index] = max(c.first_key - 1,c.first_key);
				rtc_index++;
	}
	rtcUnmapBuffer(scene, geom_id, RTC_INDEX_BUFFER);

	int t_mid = (num_motion_steps - 1) / 2;
	const float *curve_radius = &mesh->curve_radius[0];
	for(int t = 0; t < num_motion_steps; t++) {
		RTCBufferType buffer_type = (RTCBufferType)(RTC_VERTEX_BUFFER+t);
		const float3 *verts;
		if(t == t_mid) {
			verts = &mesh->curve_keys[0];
		} else {
			int t_ = (t > t_mid) ? (t - 1) : t;
			verts = &attr_mP->data_float3()[t_ * num_keys];
		}
		raw_buffer = rtcMapBuffer(scene, geom_id, buffer_type);
		float *rtc_verts = (float*) raw_buffer;
		for(size_t j = 0; j < num_keys; j++) {
			rtc_verts[0] = verts[j].x;
			rtc_verts[1] = verts[j].y;
			rtc_verts[2] = verts[j].z;
			rtc_verts[3] = curve_radius[j];
			rtc_verts += 4;
		}
		rtcUnmapBuffer(scene, geom_id, buffer_type);
	}

	pack.prim_object.reserve(pack.prim_object.size() + num_curves);
	pack.prim_type.reserve(pack.prim_type.size() + num_curves);
	pack.prim_index.reserve(pack.prim_index.size() + num_curves);
	pack.prim_tri_index.reserve(pack.prim_index.size() + num_curves);
	for(size_t j = 0; j < num_curves; j++) {
		pack.prim_object.push_back_reserved(i);
		pack.prim_type.push_back_reserved(PRIMITIVE_CURVE);
		pack.prim_index.push_back_reserved(j);
		pack.prim_tri_index.push_back_reserved(j);
	}
	
	return geom_id;
}
 
void BVHEmbree::pack_nodes(const BVHNode *root)
{
	if(!params.top_level) {
		return;
	}

	for(size_t i = 0; i < pack.prim_index.size(); i++) {
		if(pack.prim_index[i] != -1) {
			if(pack.prim_type[i] & PRIMITIVE_ALL_CURVE)
				pack.prim_index[i] += objects[pack.prim_object[i]]->mesh->curve_offset;
			else
				pack.prim_index[i] += objects[pack.prim_object[i]]->mesh->tri_offset;
		}
	}

	size_t prim_offset = pack.prim_index.size();

	/* reserve */
	size_t prim_index_size = pack.prim_index.size();
	size_t prim_tri_verts_size = pack.prim_tri_verts.size();

	size_t pack_prim_index_offset = prim_index_size;
	size_t pack_prim_tri_verts_offset = prim_tri_verts_size;
	size_t object_offset = 0;

	map<Mesh*, int> mesh_map;

	foreach(Object *ob, objects) {
		Mesh *mesh = ob->mesh;
		BVH *bvh = mesh->bvh;

		if(mesh->need_build_bvh()) {
			if(mesh_map.find(mesh) == mesh_map.end()) {
				prim_index_size += bvh->pack.prim_index.size();
				prim_tri_verts_size += bvh->pack.prim_tri_verts.size();
				mesh_map[mesh] = 1;
			}
		}
	}

	mesh_map.clear();

	pack.prim_index.resize(prim_index_size);
	pack.prim_type.resize(prim_index_size);
	pack.prim_object.resize(prim_index_size);
	pack.prim_visibility.resize(prim_index_size);
	pack.prim_tri_verts.resize(prim_tri_verts_size);
	pack.prim_tri_index.resize(prim_index_size);
	pack.object_node.resize(objects.size());

	int *pack_prim_index = (pack.prim_index.size())? &pack.prim_index[0]: NULL;
	int *pack_prim_type = (pack.prim_type.size())? &pack.prim_type[0]: NULL;
	int *pack_prim_object = (pack.prim_object.size())? &pack.prim_object[0]: NULL;
	uint *pack_prim_visibility = (pack.prim_visibility.size())? &pack.prim_visibility[0]: NULL;
	float4 *pack_prim_tri_verts = (pack.prim_tri_verts.size())? &pack.prim_tri_verts[0]: NULL;
	uint *pack_prim_tri_index = (pack.prim_tri_index.size())? &pack.prim_tri_index[0]: NULL;

	/* merge */
	unsigned geom_id = 0;
	foreach(Object *ob, objects) {
		geom_id += 2;
		Mesh *mesh = ob->mesh;

		/* We assume that if mesh doesn't need own BVH it was already included
		 * into a top-level BVH and no packing here is needed.
		 */
		if(!mesh->need_build_bvh()) {
			pack.object_node[object_offset++] = prim_offset;
			continue;
		}

		/* if mesh already added once, don't add it again, but used set
		 * node offset for this object */
		map<Mesh*, int>::iterator it = mesh_map.find(mesh);

		if(mesh_map.find(mesh) != mesh_map.end()) {
			int noffset = it->second;
			pack.object_node[object_offset++] = noffset;
			continue;
		}

		BVH *bvh = mesh->bvh;

		int mesh_tri_offset = mesh->tri_offset;
		int mesh_curve_offset = mesh->curve_offset;

		/* fill in node indexes for instances */
		if(bvh->pack.root_index == -1)
			pack.object_node[object_offset++] = prim_offset;//todo (Stefan)
		else
			pack.object_node[object_offset++] = prim_offset; // todo (Stefan)

		mesh_map[mesh] = pack.object_node[object_offset-1];

		/* merge primitive, object and triangle indexes */
		if(bvh->pack.prim_index.size()) {
			size_t bvh_prim_index_size = bvh->pack.prim_index.size();
			int *bvh_prim_index = &bvh->pack.prim_index[0];
			int *bvh_prim_type = &bvh->pack.prim_type[0];
			uint *bvh_prim_visibility = &bvh->pack.prim_visibility[0];
			uint *bvh_prim_tri_index = &bvh->pack.prim_tri_index[0];

			for(size_t i = 0; i < bvh_prim_index_size; i++) {
				if(bvh->pack.prim_type[i] & PRIMITIVE_ALL_CURVE) {
					pack_prim_index[pack_prim_index_offset] = bvh_prim_index[i] + mesh_curve_offset;
					pack_prim_tri_index[pack_prim_index_offset] = -1;
				}
				else {
					pack_prim_index[pack_prim_index_offset] = bvh_prim_index[i] + mesh_tri_offset;
					pack_prim_tri_index[pack_prim_index_offset] =
					bvh_prim_tri_index[i] + pack_prim_tri_verts_offset;
				}

				pack_prim_type[pack_prim_index_offset] = bvh_prim_type[i];
				pack_prim_visibility[pack_prim_index_offset] = bvh_prim_visibility[i];
				pack_prim_object[pack_prim_index_offset] = 0;  // unused for instances

				pack_prim_index_offset++;
			}
		}

		/* Merge triangle vertices data. */
		if(bvh->pack.prim_tri_verts.size()) {
			const size_t prim_tri_size = bvh->pack.prim_tri_verts.size();
			memcpy(pack_prim_tri_verts + pack_prim_tri_verts_offset,
				   &bvh->pack.prim_tri_verts[0],
				   prim_tri_size*sizeof(float4));
			pack_prim_tri_verts_offset += prim_tri_size;
		}

		prim_offset += bvh->pack.prim_index.size();
	}
}

void BVHEmbree::refit_nodes()
{
#if 0
	unsigned geom_id = 0;
	foreach(Object *ob, objects) {
		void *raw_buffer = rtcMapBuffer(scene, geom_id, RTC_VERTEX_BUFFER);
		float *rtc_verts = (float*) raw_buffer;
		rtcUnmapBuffer(scene, geom_id, RTC_VERTEX_BUFFER);
		rtcUpdate(scene, geom_id);
		geom_id++;
	}
#endif
}
CCL_NAMESPACE_END

#endif /* WITH_EMBREE */
