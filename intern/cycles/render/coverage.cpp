/*
 * Copyright 2017 Blender Foundation
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

#include "kernel/kernel_compat_cpu.h"
#include "kernel/kernel_types.h"
#include "kernel/split/kernel_split_data_types.h"
#include "kernel/kernel_globals.h"
#include "util/util_vector.h"
#include "render/coverage.h"

CCL_NAMESPACE_BEGIN

static inline void kernel_write_id_slots(float *buffer, int num_slots, float id, float weight, bool init)
{
	kernel_assert(id != ID_NONE);

	if(weight == 0.0f) {
		return;
	}

	if(init) {
		for(int slot = 0; slot < num_slots; slot++) {
			buffer[slot*ID_SLOT_SIZE + 0] = ID_NONE;
			buffer[slot*ID_SLOT_SIZE + 1] = 0.0f;
		}
	} else {
		init = false;
	}

	for(int slot = 0; slot < num_slots; slot++) {
		float *slot_id = (&buffer[slot*ID_SLOT_SIZE + 0]);
		float *slot_weight = &buffer[slot*ID_SLOT_SIZE + 1];

		/* If the loop reaches an empty slot, the ID isn't in any slot yet - so add it! */
		if(*slot_weight == 0.0f) {
			kernel_assert(*slot_id == ID_NONE);
			*slot_id = id;
			*slot_weight = weight;
			break;
		}
		/* If there already is a slot for that ID, add the weight. */
		else if(*slot_id == id) {
			*slot_weight += weight;
			break;
		}
	}
}

static bool crypomatte_comp(const std::pair<float, float>& i, const std::pair<float, float> j) { return i.first > j.first; }

void flatten_coverage(KernelGlobals *kg, vector<map<float, float> > & coverage, const RenderTile &tile)
{
	/* sort the coverage map and write it to the output */
	int index = 0;
	for(int y = 0; y < tile.h; y++) {
		for(int x = 0; x < tile.w; x++) {
			const map<float, float>& pixel = coverage[index];
			if(!pixel.empty()) {
				/* buffer offset */
				int index = x + y*tile.stride;
				int pass_stride = kg->__data.film.pass_stride;
				float *buffer = (float*)tile.buffer + index*pass_stride;

				/* sort the cryptomatte pixel */
				vector<std::pair<float, float> > sorted_pixel;
				for(map<float, float>::const_iterator it = pixel.begin(); it != pixel.end(); ++it) {
					sorted_pixel.push_back(std::make_pair(it->second, it->first));
				}
				sort(sorted_pixel.begin(), sorted_pixel.end(), crypomatte_comp);
				int num_slots = 2 * (kg->__data.film.use_cryptomatte & 255);
				if(sorted_pixel.size() > num_slots) {
					float leftover = 0.0f;
					for(vector<pair<float, float> >::iterator it = sorted_pixel.begin()+num_slots; it != sorted_pixel.end(); it++) {
						leftover += it->first;
					}
					sorted_pixel[num_slots-1].first += leftover;
				}
				int limit = min(num_slots, sorted_pixel.size());
				for(int i = 0; i < limit; i++) {
					int pass_offset = (kg->__data.film.pass_aov[0] & ~(1 << 31));
					kernel_write_id_slots(buffer + pass_offset, 2 * (kg->__data.film.use_cryptomatte & 255), sorted_pixel[i].second, sorted_pixel[i].first, i == 0);
				}
			}
			index++;
		}
	}
}

CCL_NAMESPACE_END
