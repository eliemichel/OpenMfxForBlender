/*
 * Copyright 2011-2013 Blender Foundation
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

#ifndef __FILM_H__
#define __FILM_H__

#include "util_string.h"
#include "util_vector.h"

#include "kernel_types.h"

#include "node.h"

CCL_NAMESPACE_BEGIN

class Device;
class DeviceScene;
class Scene;

typedef enum FilterType {
	FILTER_BOX,
	FILTER_GAUSSIAN,
	FILTER_BLACKMAN_HARRIS,

	FILTER_NUM_TYPES,
} FilterType;

template<typename T>
bool operator==(const array<T>& A, const array<T>& B)
{
	if(A.size() != B.size())
		return false;

	for(int i = 0; i < A.size(); i++)
		if(A[i].type != B[i].type)
			return false;
}

struct Pass {
public:
	PassType type;
	int components;
	bool filter;
	bool exposure;
	PassType divide_type;
	bool is_virtual;
};

struct AOV {
public:
	ustring name;
	int index;
	bool is_color;
};

class PassSettings {
public:
	PassSettings();
	bool modified(const PassSettings& other) const;

	int get_size() const;
	Pass* get_pass(PassType type, int &offset);
	AOV* get_aov(ustring name, int &offset);

	bool contains(PassType type) const;
	void add(PassType type);
	void add(AOV aov);

protected:
	array<Pass> passes;
	array<AOV> aovs;

	friend class Film;
};

class Film : public Node {
public:
	NODE_DECLARE;

	float exposure;
	PassSettings passes;
	float pass_alpha_threshold;

	FilterType filter_type;
	float filter_width;
	size_t filter_table_offset;

	float mist_start;
	float mist_depth;
	float mist_falloff;

	bool use_light_visibility;
	bool use_sample_clamp;

	bool need_update;

	Film();
	~Film();

	void device_update(Device *device, DeviceScene *dscene, Scene *scene);
	void device_free(Device *device, DeviceScene *dscene, Scene *scene);

	bool modified(const Film& film);
	void tag_passes_update(Scene *scene, const PassSettings& passes);
	void tag_update(Scene *scene);
};

CCL_NAMESPACE_END

#endif /* __FILM_H__ */

