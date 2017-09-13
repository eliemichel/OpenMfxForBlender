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

#include "bvh/bvh_embree.h"

#include "render/mesh.h"
#include "render/object.h"
#include "util/util_progress.h"
#include "util/util_foreach.h"
#include "util/util_logging.h"

#include "embree2/rtcore_geometry.h"

/* kernel includes are necessary so that the filter function for embree can access the packed BVH */
#include "kernel/kernel_compat_cpu.h"
#include "kernel/split/kernel_split_data_types.h"
#include "kernel/kernel_globals.h"
#include "kernel/kernel_random.h"
#include "kernel/bvh/bvh_embree_traversal.h"

#include "xmmintrin.h"
#include "pmmintrin.h"

/* this doesn't work with refitting unfortunately
 * #define EMBREE_SHARED_MEM 1
 */

/* this should eventually come from render settings. */
//#define HAIR_CURVES

CCL_NAMESPACE_BEGIN

/* This gets called by embree at every valid ray/object intersection.
 * Things like recording subsurface or shadow hits for later evaluation
 * as well as filtering for volume objects happen here.
 * Cycles' own BVH does that directly inside the traversal calls.
 */


void rtc_filter_func(void*, RTCRay& ray_);
void rtc_filter_func(void*, RTCRay& ray_)
{
	CCLRay &ray = (CCLRay&)ray_;
	KernelGlobals *kg = ray.kg;

	if(ray.type == CCLRay::RAY_REGULAR) {
		return;
	}
	else if(ray.type == CCLRay::RAY_SHADOW_ALL) {
		// append the intersection to the end of the array
		if(ray.num_hits < ray.max_hits) {
			Intersection *isect = &ray.isect_s[ray.num_hits];
			ray.num_hits++;
			ray.isect_to_ccl(isect);
			int prim = kernel_tex_fetch(__prim_index, isect->prim);
			int shader = 0;
			if(kernel_tex_fetch(__prim_type, isect->prim) & PRIMITIVE_ALL_TRIANGLE)
			{
				shader = kernel_tex_fetch(__tri_shader, prim);
			}
			else {
				float4 str = kernel_tex_fetch(__curves, prim);
				shader = __float_as_int(str.z);
			}
			int flag = kernel_tex_fetch(__shader_flag, (shader & SHADER_MASK)*SHADER_SIZE);
			/* if no transparent shadows, all light is blocked */
			if(flag & (SD_SHADER_HAS_TRANSPARENT_SHADOW | SD_SHADER_USE_UNIFORM_ALPHA)) {
				/* this tells embree to continue tracing */
				ray.geomID = RTC_INVALID_GEOMETRY_ID;
			}
			else {
				ray.num_hits = ray.max_hits+1;
			}
		}
		else {
			/* Increase the number of hits beyond ray.max_hits
			 * so that the caller can detect this as opaque. */
			ray.num_hits++;
		}
		return;
	}
	else if(ray.type == CCLRay::RAY_SSS) {
		/* only accept hits from the same object and triangles */
		if(ray.instID/2 != ray.sss_object_id || ray.geomID & 1) {
			/* this tells embree to continue tracing */
			ray.geomID = RTC_INVALID_GEOMETRY_ID;
			return;
		}

		/* see triangle_intersect_subsurface() for the native equivalent */
		for(int i = min(ray.max_hits, ray.ss_isect->num_hits) - 1; i >= 0; --i) {
			if(ray.ss_isect->hits[i].t == ray.tfar) {
				/* this tells embree to continue tracing */
				ray.geomID = RTC_INVALID_GEOMETRY_ID;
				return;
			}
		}

		ray.ss_isect->num_hits++;
		int hit;

		if(ray.ss_isect->num_hits <= ray.max_hits) {
			hit = ray.ss_isect->num_hits - 1;
		}
		else {
			/* reservoir sampling: if we are at the maximum number of
			 * hits, randomly replace element or skip it */
			hit = lcg_step_uint(ray.lcg_state) % ray.ss_isect->num_hits;

			if(hit >= ray.max_hits) {
				/* this tells embree to continue tracing */
				ray.geomID = RTC_INVALID_GEOMETRY_ID;
				return;
			}
		}
		/* record intersection */
		ray.isect_to_ccl(&ray.ss_isect->hits[hit]);
		ray.ss_isect->Ng[hit].x = -ray.Ng[0];
		ray.ss_isect->Ng[hit].y = -ray.Ng[1];
		ray.ss_isect->Ng[hit].z = -ray.Ng[2];
		ray.ss_isect->Ng[hit] = normalize(ray.ss_isect->Ng[hit]);
		/* this tells embree to continue tracing */
		ray.geomID = RTC_INVALID_GEOMETRY_ID;
		return;
	} else if(ray.type == CCLRay::RAY_VOLUME_ALL) {
		// append the intersection to the end of the array
		if(ray.num_hits < ray.max_hits) {
			Intersection *isect = &ray.isect_s[ray.num_hits];
			ray.num_hits++;
			ray.isect_to_ccl(isect);
			/* only primitives from volume object */
			uint tri_object = kernel_tex_fetch(__prim_object, isect->prim);
			int object_flag = kernel_tex_fetch(__object_flag, tri_object);
			if((object_flag & SD_OBJECT_OBJECT_HAS_VOLUME) == 0) {
				ray.num_hits--;
			}
			/* this tells embree to continue tracing */
			ray.geomID = RTC_INVALID_GEOMETRY_ID;
			return;
		}
		return;
	}
 	return;
}
bool rtc_memory_monitor_func(void* userPtr, const ssize_t bytes, const bool post);
bool rtc_memory_monitor_func(void* userPtr, const ssize_t bytes, const bool)
{
	BVHEmbree *bvh = (BVHEmbree*)userPtr;
	if(bvh) {
		bvh->mem_monitor(bytes);
	}
	return true;
}

static double progress_start_time = 0.0f;

bool rtc_progress_func(void* user_ptr, const double n);
bool rtc_progress_func(void* user_ptr, const double n)
{
	Progress *progress = (Progress*)user_ptr;

	if(time_dt() - progress_start_time < 0.25)
		return true;

	string msg = string_printf("Building BVH %.0f%%", n * 100.0);

	progress->set_substatus(msg);
	progress_start_time = time_dt();

	return !progress->get_cancel();
}

/* This is to have a shared device between all BVH instances */
RTCDevice BVHEmbree::rtc_shared_device = NULL;
int BVHEmbree::rtc_shared_users = 0;
thread_mutex BVHEmbree::rtc_shared_mutex;

BVHEmbree::BVHEmbree(const BVHParams& params_, const vector<Object*>& objects_)
: BVH(params_, objects_), scene(NULL), mem_used(0), top_level(NULL), stats(NULL)
{
	_MM_SET_FLUSH_ZERO_MODE(_MM_FLUSH_ZERO_ON);
	_MM_SET_DENORMALS_ZERO_MODE(_MM_DENORMALS_ZERO_ON);
	thread_scoped_lock lock(rtc_shared_mutex);
	if(rtc_shared_users == 0) {
		rtc_shared_device = rtcNewDevice("verbose=1");

		/* Check here if Embree was built with the correct flags. */
		ssize_t ret = rtcDeviceGetParameter1i(rtc_shared_device, RTC_CONFIG_RAY_MASK);
		if(ret != 1) {
			assert(0);
			VLOG(1) << "Embree is compiled without the EMBREE_RAY_MASK flag. Ray visiblity will not work.";
		}
		ret = rtcDeviceGetParameter1i(rtc_shared_device, RTC_CONFIG_INTERSECTION_FILTER);
		if(ret != 1) {
			assert(0);
			VLOG(1) << "Embree is compiled without the EMBREE_INTERSECTION_FILTER flag. Renders may not look as expected.";
		}
		ret = rtcDeviceGetParameter1i(rtc_shared_device, RTC_CONFIG_LINE_GEOMETRY);
		if(ret != 1) {
			assert(0);
			VLOG(1) << "Embree is compiled without the EMBREE_GEOMETRY_LINES flag. Hair primitives will not be rendered.";
		}
		ret = rtcDeviceGetParameter1i(rtc_shared_device, RTC_CONFIG_TRIANGLE_GEOMETRY);
		if(ret != 1) {
			assert(0);
			VLOG(1) << "Embree is compiled without the EMBREE_GEOMETRY_TRIANGLES flag. Triangle primitives will not be rendered.";
		}
		ret = rtcDeviceGetParameter1i(rtc_shared_device, RTC_CONFIG_BACKFACE_CULLING);
		if(ret != 0) {
			assert(0);
			VLOG(1) << "Embree is compiled with the EMBREE_BACKFACE_CULLING flag. Renders may not look as expected.";
		}
	}
	rtc_shared_users++;

	rtcDeviceSetMemoryMonitorFunction2(rtc_shared_device, rtc_memory_monitor_func, this);

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
		/* When this BVH is used as an instance in a top level BVH, don't delete now
		 * Let the top_level BVH know that it should delete it later */
		if(top_level) {
			top_level->add_delayed_delete_scene(scene);
		}
		else {
			rtcDeleteScene(scene);
			if(delayed_delete_scenes.size()) {
				foreach(RTCScene s, delayed_delete_scenes) {
					rtcDeleteScene(s);
				}
			}
			delayed_delete_scenes.clear();
		}
		scene = NULL;
	}
}

void BVHEmbree::mem_monitor(ssize_t bytes)
{
	if(stats) {
		if(bytes > 0) {
			stats->mem_alloc(bytes);
		} else {
			stats->mem_free(-bytes);
		}
	}
	mem_used += bytes;
}

void BVHEmbree::build(Progress& progress, Stats *stats_)
{
	stats = stats_;

	progress.set_substatus("Building BVH");

	if (scene) {
		rtcDeleteScene(scene);
		scene = NULL;
	}

	RTCSceneFlags flags = RTC_SCENE_DYNAMIC|RTC_SCENE_COMPACT|RTC_SCENE_HIGH_QUALITY|RTC_SCENE_ROBUST;
	if(params.use_spatial_split) {
		flags = flags|RTC_SCENE_HIGH_QUALITY;
	}
	scene = rtcDeviceNewScene(rtc_shared_device, flags, RTC_INTERSECT1);

	int i = 0;

	pack.object_node.clear();

	foreach(Object *ob, objects) {
		if(params.top_level) {
			if(!ob->is_traceable()) {
				++i;
				continue;
			}
			if(!ob->mesh->is_instanced()) {
				add_object(ob, i);
			}
			else {
				add_instance(ob, i);
			}
		}
		else {
			add_object(ob, i);
		}
		++i;
		if(progress.get_cancel()) return;
	}

	if(progress.get_cancel()) {
		delete_rtcScene();
		stats = NULL;
		return;
	}

	rtcSetProgressMonitorFunction(scene, rtc_progress_func, &progress);
	rtcCommit(scene);

	pack_primitives();

	if(progress.get_cancel()) {
		delete_rtcScene();
		stats = NULL;
		return;
	}

	progress.set_substatus("Packing geometry");
	pack_nodes(NULL);

	stats = NULL;
}

unsigned BVHEmbree::add_object(Object *ob, int i)
{
	Mesh *mesh = ob->mesh;
	unsigned geom_id = RTC_INVALID_GEOMETRY_ID;
	if(params.primitive_mask & PRIMITIVE_ALL_TRIANGLE && mesh->num_triangles() > 0) {
		size_t prim_offset = pack.prim_index.size();
		geom_id = add_triangles(mesh, i);
		rtcSetUserData(scene, geom_id, (void*)prim_offset);
		rtcSetOcclusionFilterFunction(scene, geom_id, rtc_filter_func);
		rtcSetMask(scene, geom_id, ob->visibility);
	}
	if(params.primitive_mask & PRIMITIVE_ALL_CURVE && mesh->num_curves() > 0) {
		size_t prim_offset = pack.prim_index.size();
		geom_id = add_curves(mesh, i);
		rtcSetUserData(scene, geom_id, (void*)prim_offset);
		rtcSetOcclusionFilterFunction(scene, geom_id, rtc_filter_func);
		rtcSetMask(scene, geom_id, ob->visibility);
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

	if(instance_bvh->top_level != this) {
		instance_bvh->top_level = this;
	}

	const size_t num_motion_steps = ob->use_motion ? 3 : 1;
	unsigned geom_id = rtcNewInstance3(scene, instance_bvh->scene, num_motion_steps, i*2);

	if(ob->use_motion) {
		rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->motion.pre, 0);
		rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->tfm, 1);
		rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->motion.post, 2);
	} else {
		rtcSetTransform2(scene, geom_id, RTC_MATRIX_ROW_MAJOR, (const float*)&ob->tfm);
	}

	rtcSetUserData(scene, geom_id, (void*)instance_bvh->scene);
	rtcSetMask(scene, geom_id, ob->visibility);

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
	if(mesh->has_motion_blur()) {
		attr_mP = mesh->attributes.find(ATTR_STD_MOTION_VERTEX_POSITION);
		if(attr_mP) {
			num_motion_steps = mesh->motion_steps;
			if(num_motion_steps > RTC_MAX_TIME_STEPS) {
				assert(0);
				num_motion_steps = RTC_MAX_TIME_STEPS;
			}
		}
	}

	const size_t num_triangles = mesh->num_triangles();
	const size_t num_verts = mesh->verts.size();
	unsigned geom_id = rtcNewTriangleMesh2(scene,
						RTC_GEOMETRY_DEFORMABLE,
						num_triangles,
						num_verts,
						num_motion_steps,
						i*2);

#ifdef EMBREE_SHARED_MEM
	/* embree and Cycles use the same memory layout, so we can conveniently use the rtcSetBuffer2 calls */
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

	update_tri_vertex_buffer(geom_id, mesh);

	pack.prim_object.reserve(pack.prim_object.size() + num_triangles);
	pack.prim_type.reserve(pack.prim_type.size() + num_triangles);
	pack.prim_index.reserve(pack.prim_index.size() + num_triangles);
	pack.prim_tri_index.reserve(pack.prim_index.size() + num_triangles);
	for(size_t j = 0; j < num_triangles; j++) {
		pack.prim_object.push_back_reserved(i);
		pack.prim_type.push_back_reserved(num_motion_steps > 1 ? PRIMITIVE_MOTION_TRIANGLE : PRIMITIVE_TRIANGLE);
		pack.prim_index.push_back_reserved(j);
		pack.prim_tri_index.push_back_reserved(j);
	}

	return geom_id;
}

void BVHEmbree::update_tri_vertex_buffer(unsigned geom_id, const Mesh* mesh)
{
	const Attribute *attr_mP = NULL;
	size_t num_motion_steps = 1;
	int t_mid = 0;
	if(mesh->has_motion_blur()) {
		attr_mP = mesh->attributes.find(ATTR_STD_MOTION_VERTEX_POSITION);
		if(attr_mP) {
			num_motion_steps = mesh->motion_steps;
			t_mid = (num_motion_steps - 1) / 2;
			if(num_motion_steps > RTC_MAX_TIME_STEPS) {
				assert(0);
				num_motion_steps = RTC_MAX_TIME_STEPS;
			}
		}
	}
	const size_t num_verts = mesh->verts.size();


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
		void *raw_buffer = rtcMapBuffer(scene, geom_id, buffer_type);
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
}

void BVHEmbree::update_curve_vertex_buffer(unsigned geom_id, const Mesh* mesh)
{
	const Attribute *attr_mP = NULL;
	size_t num_motion_steps = 1;
	if(mesh->has_motion_blur()) {
		attr_mP = mesh->curve_attributes.find(ATTR_STD_MOTION_VERTEX_POSITION);
		if(attr_mP) {
			num_motion_steps = mesh->motion_steps;
		}
	}

	const size_t num_keys = mesh->curve_keys.size();

	/* Copy the CV data to embree */
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

#ifdef EMBREE_SHARED_MEM
		if(t != t_mid) {
			rtcSetBuffer(scene, geom_id, buffer_type, verts, 0, sizeof(float4));
		} else
#endif
		{
			void *raw_buffer = rtcMapBuffer(scene, geom_id, buffer_type);
			float *rtc_verts = (float*) raw_buffer;
#ifdef HAIR_CURVES
			rtc_verts[0] = verts[0].x;
			rtc_verts[1] = verts[0].y;
			rtc_verts[2] = verts[0].z;
			rtc_verts[3] = curve_radius[0];
			rtc_verts += 4;
#endif
			for(size_t j = 0; j < num_keys; j++) {
				rtc_verts[0] = verts[j].x;
				rtc_verts[1] = verts[j].y;
				rtc_verts[2] = verts[j].z;
				rtc_verts[3] = curve_radius[j];
				rtc_verts += 4;
			}
#ifdef HAIR_CURVES
			rtc_verts[0] = verts[num_keys-1].x;
			rtc_verts[1] = verts[num_keys-1].y;
			rtc_verts[2] = verts[num_keys-1].z;
			rtc_verts[3] = curve_radius[num_keys-1];
#endif
			rtcUnmapBuffer(scene, geom_id, buffer_type);
		}
	}
}

unsigned BVHEmbree::add_curves(Mesh *mesh, int i)
{
	const Attribute *attr_mP = NULL;
	size_t num_motion_steps = 1;
	if(mesh->has_motion_blur()) {
		attr_mP = mesh->curve_attributes.find(ATTR_STD_MOTION_VERTEX_POSITION);
		if(attr_mP) {
			num_motion_steps = mesh->motion_steps;
		}
	}

	const size_t num_curves = mesh->num_curves();
	size_t num_segments = 0;
	for(size_t j = 0; j < num_curves; j++) {
		Mesh::Curve c = mesh->get_curve(j);
		num_segments += c.num_segments();
	}

	const size_t num_keys = mesh->curve_keys.size();

	/* Make room for Cycles specific data */
	pack.prim_object.reserve(pack.prim_object.size() + num_segments);
	pack.prim_type.reserve(pack.prim_type.size() + num_segments);
	pack.prim_index.reserve(pack.prim_index.size() + num_segments);
	pack.prim_tri_index.reserve(pack.prim_index.size() + num_segments);

#ifndef HAIR_CURVES /* line segments */
	unsigned geom_id = rtcNewLineSegments2(scene,
												 RTC_GEOMETRY_DEFORMABLE,
												 num_segments,
												 num_keys,
												 num_motion_steps,
												 i*2+1);


	/* Split the Cycles curves into embree line segments, each with 2 CVs */
	void* raw_buffer = rtcMapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
	unsigned *rtc_indices = (unsigned*) raw_buffer;
	size_t rtc_index = 0;
	for(size_t j = 0; j < num_curves; j++) {
		Mesh::Curve c = mesh->get_curve(j);
		for(size_t k = 0; k < c.num_segments(); k++) {
			rtc_indices[rtc_index] = c.first_key + k;

			/* Cycles specific data */
			pack.prim_object.push_back_reserved(i);
			pack.prim_type.push_back_reserved(PRIMITIVE_PACK_SEGMENT(num_motion_steps > 1 ? PRIMITIVE_MOTION_CURVE : PRIMITIVE_CURVE, k));
			pack.prim_index.push_back_reserved(j);
			pack.prim_tri_index.push_back_reserved(rtc_index);

			rtc_index++;
		}
	}
	rtcUnmapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
#else
	/* curve segments */
	num_segments = num_segments / 2;
	unsigned geom_id = rtcNewBezierHairGeometry2(scene,
												 RTC_GEOMETRY_DEFORMABLE,
												 num_segments,
												 num_keys + 2 * num_curves,
												 num_motion_steps,
												 i*2+1);


	/* Split the Cycles curves into embree hair segments, each with 4 CVs */
	void* raw_buffer = rtcMapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
	unsigned *rtc_indices = (unsigned*) raw_buffer;
	size_t rtc_index = 0;
	for(size_t j = 0; j < num_curves; j++) {
		Mesh::Curve c = mesh->get_curve(j);
		for(size_t k = 0; k < c.num_segments(); k+=3) {
			rtc_indices[rtc_index] = c.first_key + k;

			/* Cycles specific data */
			pack.prim_object.push_back_reserved(i);
			pack.prim_type.push_back_reserved(PRIMITIVE_PACK_SEGMENT(num_motion_steps > 1 ? PRIMITIVE_MOTION_CURVE : PRIMITIVE_CURVE, k));
			pack.prim_index.push_back_reserved(j);
			pack.prim_tri_index.push_back_reserved(rtc_index);

			rtc_index++;
		}
	}
	rtcUnmapBuffer(scene, geom_id, RTC_INDEX_BUFFER);
#endif

	update_curve_vertex_buffer(geom_id, mesh);

	return geom_id;
}
 
void BVHEmbree::pack_nodes(const BVHNode *)
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

		BVHEmbree *bvh = (BVHEmbree*)mesh->bvh;

		mem_monitor(bvh->mem_used);

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
	unsigned geom_id = 0;

	foreach(Object *ob, objects) {
		if(!params.top_level || (ob->is_traceable() && !ob->mesh->is_instanced())) {
			if(params.primitive_mask & PRIMITIVE_ALL_TRIANGLE && ob->mesh->num_triangles() > 0) {
				update_tri_vertex_buffer(geom_id, ob->mesh);
				rtcUpdate(scene, geom_id);
			}

			if(params.primitive_mask & PRIMITIVE_ALL_CURVE && ob->mesh->num_curves() > 0) {
				update_curve_vertex_buffer(geom_id+1, ob->mesh);
				rtcUpdate(scene, geom_id+1);
			}
		}
		geom_id += 2;
	}
	rtcCommit(scene);
}
CCL_NAMESPACE_END

#endif /* WITH_EMBREE */
