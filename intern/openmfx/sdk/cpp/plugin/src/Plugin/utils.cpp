#include "utils.h"

float3 to_float3(const double3& valued) {
	std::array<float,3> value;
	value[0] = static_cast<float>(valued[0]);
	value[1] = static_cast<float>(valued[1]);
	value[2] = static_cast<float>(valued[2]);
	return value;
}
