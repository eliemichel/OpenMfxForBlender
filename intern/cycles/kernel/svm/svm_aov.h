#ifndef __SVM_AOV_H__
#define __SVM_AOV_H__

CCL_NAMESPACE_BEGIN

ccl_device_inline void kernel_write_pass_float(ccl_global float *buffer, int sample, float value);
ccl_device_inline void kernel_write_pass_float3(ccl_global float *buffer, int sample, float3 value);

ccl_device void svm_node_aov_write_float3(KernelGlobals *kg,
                                          PathState *state,
                                          float *stack,
                                          int offset,
                                          int aov,
                                          ccl_global float *buffer,
                                          int sample) {
	if(state->written_aovs & (1 << aov)) {
		return;
	}

	float3 val = stack_load_float3(stack, offset);

	int pass_offset = (kernel_data.film.pass_aov[aov] & ~(1 << 31));
	kernel_assert(kernel_data.film.pass_aov[aov] & (1 << 31));

	kernel_write_pass_float3(buffer + pass_offset, sample, val);
	state->written_aovs |= (1 << aov);
}

ccl_device void svm_node_aov_write_float(KernelGlobals *kg,
                                          PathState *state,
                                          float *stack,
                                          int offset,
                                          int aov,
                                          ccl_global float *buffer,
                                          int sample) {
	if(state->written_aovs & (1 << aov)) {
		return;
	}

	float val = stack_load_float(stack, offset);

	int pass_offset = (kernel_data.film.pass_aov[aov] & ~(1 << 31));
	kernel_assert((kernel_data.film.pass_aov[aov] & (1 << 31)) == 0);

	kernel_write_pass_float(buffer + pass_offset, sample, val);
	state->written_aovs |= (1 << aov);
}

CCL_NAMESPACE_END

#endif /* __SVM_AOV_H__ */
