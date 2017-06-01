/*
 * Modifications Copyright 2017, Blender Foundation.
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

#ifndef __BVH_EMBREE_H__
#define __BVH_EMBREE_H__

#ifdef WITH_EMBREE

#include "bvh.h"
#include "bvh_params.h"

#include "util_types.h"
#include "util_vector.h"
#include "util_thread.h"

#include "embree2/rtcore.h"
#include "embree2/rtcore_scene.h"

CCL_NAMESPACE_BEGIN

class Mesh;

class BVHEmbree : public BVH
{
public:
	void build(Progress& progress);
	virtual ~BVHEmbree();
	RTCScene scene;
protected:
	/* constructor */
	friend class BVH;
	BVHEmbree(const BVHParams& params, const vector<Object*>& objects);

	virtual void pack_nodes(const BVHNode *root);
	virtual void refit_nodes();

	void add_reference_mesh(Mesh *mesh, int i);
	void add_reference_object(Object *ob, int i);
	void add_reference_curves(Mesh *mesh, int i);
	void add_reference_triangles(Mesh *mesh, int i);
private:
	static RTCDevice rtc_shared_device;
	static int rtc_shared_users;
	static thread_mutex rtc_shared_mutex;
};

CCL_NAMESPACE_END

#endif /* WITH_EMBREE */

#endif /* __BVH_EMBREE_H__ */
