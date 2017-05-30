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

CCL_NAMESPACE_BEGIN

BVHEmbree::BVHEmbree(const BVHParams& params_, const vector<Object*>& objects_)
: BVH(params_, objects_)
{

}

void BVHEmbree::pack_nodes(const BVHNode *root)
{

}

void BVHEmbree::refit_nodes()
{

}
CCL_NAMESPACE_END

#endif /* WITH_EMBREE */
