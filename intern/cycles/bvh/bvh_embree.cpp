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

CCL_NAMESPACE_BEGIN

RTCDevice BVHEmbree::rtc_shared_device = NULL;
int BVHEmbree::rtc_shared_users = 0;
thread_mutex BVHEmbree::rtc_shared_mutex;

BVHEmbree::BVHEmbree(const BVHParams& params_, const vector<Object*>& objects_)
: BVH(params_, objects_), scene(NULL)
{
	thread_scoped_lock lock(rtc_shared_mutex);
	if(rtc_shared_users == 0) {
		rtc_shared_device = rtcNewDevice("verbose=2");
	}
	rtc_shared_users++;

	pack.root_index = BVH_CUSTOM;
}

BVHEmbree::~BVHEmbree()
{
	if(scene) {
		rtcDeleteScene(scene);
		scene = NULL;
	}
	thread_scoped_lock lock(rtc_shared_mutex);
	rtc_shared_users--;
	if(rtc_shared_users == 0) {
		rtcDeleteDevice(rtc_shared_device);
		rtc_shared_device = NULL;
	}
}
void BVHEmbree::build(Progress& progress)
{
	progress.set_substatus("Building BVH");

	if (scene) {
		rtcDeleteScene(scene);
		scene = NULL;
	}

	scene = rtcDeviceNewScene(rtc_shared_device, RTC_SCENE_DYNAMIC|RTC_SCENE_ROBUST, RTC_INTERSECT1);

	int i = 0;

	pack.object_node.resize(objects.size());

	foreach(Object *ob, objects) {
		if(params.top_level) {
			if(!ob->is_traceable()) {
				++i;
				continue;
			}
			if(!ob->mesh->is_instanced())
				add_reference_mesh(ob->mesh, i);
			else
				add_reference_object(ob, i);
		}
		else
			add_reference_mesh(ob->mesh, i);

		i++;

		if(progress.get_cancel()) return;
	}

	if(progress.get_cancel()) {
	//	if(root) root->deleteSubtree();
		return;
	}

	progress.set_substatus("Packing BVH triangles and strands");
	rtcCommit(scene);

	pack_primitives();

	if(progress.get_cancel()) {
	//	root->deleteSubtree();
		return;
	}

	progress.set_substatus("Packing BVH nodes");
	pack_nodes(NULL);

	//root->deleteSubtree();
}

void BVHEmbree::add_reference_mesh(Mesh *mesh, int i)
{
	if(params.primitive_mask & PRIMITIVE_ALL_TRIANGLE) {
		add_reference_triangles(mesh, i);
	}
	if(params.primitive_mask & PRIMITIVE_ALL_CURVE) {
		add_reference_curves(mesh, i);
	}
}

void BVHEmbree::add_reference_object(Object *ob, int i)
{
	if(!ob || !ob->mesh) {
		return;
	}
	BVHEmbree *instance_bvh = (BVHEmbree*)(ob->mesh->bvh);
	unsigned geom_id = rtcNewInstance2(scene, instance_bvh->scene);
	rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->tfm);

	size_t start = 0;
	if(i > 0) {
		start = pack.object_node[i-1];
	}
	if(i < pack.object_node.size()) {
		pack.object_node[i] = start;
	}
}

void BVHEmbree::add_reference_triangles(Mesh *mesh, int i)
{
	const Attribute *attr_mP = NULL;
	size_t num_motion_steps = 1;
	if(mesh->has_motion_blur()) {
		attr_mP = mesh->attributes.find(ATTR_STD_MOTION_VERTEX_POSITION);
		num_motion_steps = mesh->motion_steps;
	}

	const size_t num_triangles = mesh->num_triangles();
	const size_t num_verts = mesh->verts.size();
	unsigned geom_id = rtcNewTriangleMesh2(scene,
						RTC_GEOMETRY_DEFORMABLE,
						num_triangles,
						num_verts,
						num_motion_steps,
						i);

	void* raw_buffer = rtcMapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
	unsigned *rtc_indices = (unsigned*) raw_buffer;
	for(size_t j = 0; j < num_triangles; j++) {
		Mesh::Triangle t = mesh->get_triangle(j);
		rtc_indices[j*3] = t.v[0];
		rtc_indices[j*3+1] = t.v[1];
		rtc_indices[j*3+2] = t.v[2];
	}
	rtcUnmapBuffer(scene, geom_id, RTC_INDEX_BUFFER);

	int t_mid = (num_motion_steps - 1) / 2;
	for(int t = 0; t < num_motion_steps; t++) {
		RTCBufferType buffer_type = (RTCBufferType)(RTC_VERTEX_BUFFER+t);
		const float3 *verts;
		if(t == t_mid) {
			verts = &mesh->verts[0];
		} else {
			int t_ = (t > t_mid) ? (t - 1) : t;
			verts = &attr_mP->data_float3()[t_ * num_verts];
		}
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
	}

	size_t start = 0;
	if(i > 0) {
		start = pack.object_node[i-1];
	}
	if(i < pack.object_node.size()) {
		pack.object_node[i] = start + num_triangles;
	}

	pack.prim_object.resize(pack.prim_object.size() + num_triangles);
	pack.prim_type.resize(pack.prim_type.size() + num_triangles);
	pack.prim_index.resize(pack.prim_index.size() + num_triangles);
	pack.prim_tri_index.resize(pack.prim_index.size() + num_triangles);
	for(size_t j = 0; j < num_triangles; j++) {
		pack.prim_object[start + j] = geom_id;
		pack.prim_type[start + j] = PRIMITIVE_TRIANGLE;
		pack.prim_index[start + j] = j;
		pack.prim_tri_index[start + j] = j;
	}
}

void BVHEmbree::add_reference_curves(Mesh *mesh, int i)
{
	// TODO Stefan
}
 
void BVHEmbree::pack_nodes(const BVHNode *root)
{
	if(params.top_level) {
		for(size_t i = 0; i < pack.prim_index.size(); i++) {
			if(pack.prim_index[i] != -1) {
				if(pack.prim_type[i] & PRIMITIVE_ALL_CURVE)
					pack.prim_index[i] += objects[pack.prim_object[i]]->mesh->curve_offset;
				else
					pack.prim_index[i] += objects[pack.prim_object[i]]->mesh->tri_offset;
			}
		}
	}

	/* reserve */
	size_t prim_index_size = pack.prim_index.size();
	size_t prim_tri_verts_size = pack.prim_tri_verts.size();

//	size_t pack_prim_index_offset = prim_index_size;
//	size_t pack_prim_tri_verts_offset = prim_tri_verts_size;
//	size_t object_offset = 0;

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

	// TODO Stefan
	// probably lots more to do here
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
