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

CCL_NAMESPACE_BEGIN

ccl_device_inline void kernel_write_pass_float(ccl_global float *buffer, int sample, float value)
{
	ccl_global float *buf = buffer;
#if defined(__SPLIT_KERNEL__) && defined(__WORK_STEALING__)
	atomic_add_and_fetch_float(buf, value);
#else
	*buf = (sample == 0)? value: *buf + value;
#endif // __SPLIT_KERNEL__ && __WORK_STEALING__
}

ccl_device_inline void kernel_write_pass_float3(ccl_global float *buffer, int sample, float3 value)
{
#if defined(__SPLIT_KERNEL__) && defined(__WORK_STEALING__)
	ccl_global float *buf_x = buffer + 0;
	ccl_global float *buf_y = buffer + 1;
	ccl_global float *buf_z = buffer + 2;

	atomic_add_and_fetch_float(buf_x, value.x);
	atomic_add_and_fetch_float(buf_y, value.y);
	atomic_add_and_fetch_float(buf_z, value.z);
#else
	ccl_global float3 *buf = (ccl_global float3*)buffer;
	*buf = (sample == 0)? value: *buf + value;
#endif // __SPLIT_KERNEL__ && __WORK_STEALING__
}

ccl_device_inline void kernel_write_pass_float4(ccl_global float *buffer, int sample, float4 value)
{
#if defined(__SPLIT_KERNEL__) && defined(__WORK_STEALING__)
	ccl_global float *buf_x = buffer + 0;
	ccl_global float *buf_y = buffer + 1;
	ccl_global float *buf_z = buffer + 2;
	ccl_global float *buf_w = buffer + 3;

	atomic_add_and_fetch_float(buf_x, value.x);
	atomic_add_and_fetch_float(buf_y, value.y);
	atomic_add_and_fetch_float(buf_z, value.z);
	atomic_add_and_fetch_float(buf_w, value.w);
#else
	ccl_global float4 *buf = (ccl_global float4*)buffer;
	*buf = (sample == 0)? value: *buf + value;
#endif // __SPLIT_KERNEL__ && __WORK_STEALING__
}

ccl_device_inline void kernel_write_id_slots(ccl_global float *buffer, int num_slots, float id, float weight, bool init)
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

ccl_device_inline void kernel_write_data_passes(KernelGlobals *kg, ccl_global float *buffer, PathRadiance *L,
	ShaderData *sd, int sample, ccl_addr_space PathState *state, float3 throughput)
{
#ifdef __PASSES__
	int path_flag = state->flag;

	if(!(path_flag & PATH_RAY_CAMERA))
		return;

	int flag = kernel_data.film.pass_flag;

	if(!(flag & PASS_ALL))
		return;
	
	if(!(path_flag & PATH_RAY_SINGLE_PASS_DONE)) {
		if(!(ccl_fetch(sd, runtime_flag) & SD_RUNTIME_TRANSPARENT) ||
		   kernel_data.film.pass_alpha_threshold == 0.0f ||
		   average(shader_bsdf_alpha(kg, sd)) >= kernel_data.film.pass_alpha_threshold)
		{

			if(sample == 0) {
				if(flag & PASS_DEPTH) {
					float depth = camera_distance(kg, ccl_fetch(sd, P));
					kernel_write_pass_float(buffer + kernel_data.film.pass_depth, sample, depth);
				}
				if(flag & PASS_OBJECT_ID) {
					float id = object_pass_id(kg, ccl_fetch(sd, object));
					kernel_write_pass_float(buffer + kernel_data.film.pass_object_id, sample, id);
				}
				if(flag & PASS_MATERIAL_ID) {
					float id = shader_pass_id(kg, sd);
					kernel_write_pass_float(buffer + kernel_data.film.pass_material_id, sample, id);
				}
			}

			if(flag & PASS_NORMAL) {
				float3 normal = ccl_fetch(sd, N);
				kernel_write_pass_float3(buffer + kernel_data.film.pass_normal, sample, normal);
			}
			if(flag & PASS_UV) {
				float3 uv = primitive_uv(kg, sd);
				kernel_write_pass_float3(buffer + kernel_data.film.pass_uv, sample, uv);
			}
			if(flag & PASS_MOTION) {
				float4 speed = primitive_motion_vector(kg, sd);
				kernel_write_pass_float4(buffer + kernel_data.film.pass_motion, sample, speed);
				kernel_write_pass_float(buffer + kernel_data.film.pass_motion_weight, sample, 1.0f);
			}
			
			for(int i = 1; kernel_data.film.pass_aov[i]; i++) {
				if((state->written_aovs & (1 << i)) == 0) {
					bool is_color = (kernel_data.film.pass_aov[i] & (1 << 31));
					int pass_offset = (kernel_data.film.pass_aov[i] & ~(1 << 31));
					if(is_color) {
						kernel_write_pass_float3(buffer + pass_offset, sample, make_float3(0.0f, 0.0f, 0.0f));
					}
					else {
						kernel_write_pass_float(buffer + pass_offset, sample, 0.0f);
					}
				}
			}
			state->written_aovs = ~0;

			state->flag |= PATH_RAY_SINGLE_PASS_DONE;
		}
	}
	
	// TODO: Write cryptomatte AOV
	int aov_count = 0;
	if(kernel_data.film.use_cryptomatte & CRYPT_OBJECT) {
		float matte_weight = state->matte_weight * (1.0f - average(shader_bsdf_transparency(kg, sd)));
		bool initialize_slots = (sample == 0) && (state->transparent_bounce == 0);
		float id = object_cryptomatte_id(kg, ccl_fetch(sd, object));
		int pass_offset = (kernel_data.film.pass_aov[0] & ~(1 << 31));
		kernel_assert(kernel_data.film.pass_aov[0] & (1 << 31));
		kernel_write_id_slots(buffer + pass_offset, kernel_data.film.use_cryptomatte & 255, id, matte_weight, initialize_slots);
		state->written_aovs |= (1 << 0);
		aov_count++;
	}
	if(kernel_data.film.use_cryptomatte & CRYPT_MATERIAL) {
		float matte_weight = state->matte_weight * (1.0f - average(shader_bsdf_transparency(kg, sd)));
		bool initialize_slots = (sample == 0) && (state->transparent_bounce == 0);
		float id = shader_cryptomatte_id(kg, ccl_fetch(sd, shader));
		int pass_offset = (kernel_data.film.pass_aov[aov_count] & ~(1 << 31));
		kernel_assert(kernel_data.film.pass_aov[aov_count] & (1 << 31));
		kernel_write_id_slots(buffer + pass_offset, kernel_data.film.use_cryptomatte & 255, id, matte_weight, initialize_slots);
		state->written_aovs |= (1 << aov_count);
		aov_count++;
	}
	// end TODO
	
	if(flag & (PASS_DIFFUSE_INDIRECT|PASS_DIFFUSE_COLOR|PASS_DIFFUSE_DIRECT))
		L->color_diffuse += shader_bsdf_diffuse(kg, sd)*throughput;
	if(flag & (PASS_GLOSSY_INDIRECT|PASS_GLOSSY_COLOR|PASS_GLOSSY_DIRECT))
		L->color_glossy += shader_bsdf_glossy(kg, sd)*throughput;
	if(flag & (PASS_TRANSMISSION_INDIRECT|PASS_TRANSMISSION_COLOR|PASS_TRANSMISSION_DIRECT))
		L->color_transmission += shader_bsdf_transmission(kg, sd)*throughput;
	if(flag & (PASS_SUBSURFACE_INDIRECT|PASS_SUBSURFACE_COLOR|PASS_SUBSURFACE_DIRECT))
		L->color_subsurface += shader_bsdf_subsurface(kg, sd)*throughput;

	if(flag & PASS_MIST) {
		/* bring depth into 0..1 range */
		float mist_start = kernel_data.film.mist_start;
		float mist_inv_depth = kernel_data.film.mist_inv_depth;

		float depth = camera_distance(kg, ccl_fetch(sd, P));
		float mist = saturate((depth - mist_start)*mist_inv_depth);

		/* falloff */
		float mist_falloff = kernel_data.film.mist_falloff;

		if(mist_falloff == 1.0f)
			;
		else if(mist_falloff == 2.0f)
			mist = mist*mist;
		else if(mist_falloff == 0.5f)
			mist = sqrtf(mist);
		else
			mist = powf(mist, mist_falloff);

		/* modulate by transparency */
		float3 alpha = shader_bsdf_alpha(kg, sd);
		L->mist += (1.0f - mist)*average(throughput*alpha);
	}
#endif
}

ccl_device_inline void kernel_write_light_passes(KernelGlobals *kg, ccl_global float *buffer, PathRadiance *L, int sample)
{
#ifdef __PASSES__
	int flag = kernel_data.film.pass_flag;

	if(!kernel_data.film.use_light_pass)
		return;
	
	if(flag & PASS_DIFFUSE_INDIRECT)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_diffuse_indirect, sample, L->indirect_diffuse);
	if(flag & PASS_GLOSSY_INDIRECT)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_glossy_indirect, sample, L->indirect_glossy);
	if(flag & PASS_TRANSMISSION_INDIRECT)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_transmission_indirect, sample, L->indirect_transmission);
	if(flag & PASS_SUBSURFACE_INDIRECT)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_subsurface_indirect, sample, L->indirect_subsurface);
	if(flag & PASS_DIFFUSE_DIRECT)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_diffuse_direct, sample, L->direct_diffuse);
	if(flag & PASS_GLOSSY_DIRECT)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_glossy_direct, sample, L->direct_glossy);
	if(flag & PASS_TRANSMISSION_DIRECT)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_transmission_direct, sample, L->direct_transmission);
	if(flag & PASS_SUBSURFACE_DIRECT)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_subsurface_direct, sample, L->direct_subsurface);

	if(flag & PASS_EMISSION)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_emission, sample, L->emission);
	if(flag & PASS_BACKGROUND)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_background, sample, L->background);
	if(flag & PASS_AO)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_ao, sample, L->ao);

	if(flag & PASS_DIFFUSE_COLOR)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_diffuse_color, sample, L->color_diffuse);
	if(flag & PASS_GLOSSY_COLOR)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_glossy_color, sample, L->color_glossy);
	if(flag & PASS_TRANSMISSION_COLOR)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_transmission_color, sample, L->color_transmission);
	if(flag & PASS_SUBSURFACE_COLOR)
		kernel_write_pass_float3(buffer + kernel_data.film.pass_subsurface_color, sample, L->color_subsurface);
	if(flag & PASS_SHADOW) {
		float4 shadow = L->shadow;
		shadow.w = kernel_data.film.pass_shadow_scale;
		kernel_write_pass_float4(buffer + kernel_data.film.pass_shadow, sample, shadow);
	}
	if(flag & PASS_MIST)
		kernel_write_pass_float(buffer + kernel_data.film.pass_mist, sample, 1.0f - L->mist);
#endif
}

CCL_NAMESPACE_END

