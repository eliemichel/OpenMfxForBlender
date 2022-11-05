#pragma once

#include "MfxSuiteException.h"

#include <array>

/**
 * Utility macro to wrap suite calls. It prints a message if the call returned
 * something different from \ref kOfxStatOK. Example:
 * ```
 * MFX_CHECK(propertySuite->propSetInt(...));
 * ```
 */
#define MFX_CHECK(call) \
status = call; \
if (kOfxStatOK != status) { \
  printf("[MFX] Suite method call '" #call "' returned status %d (%s)\n", status, getOfxStateName(status)); \
}

/**
 * Like \ref MFX_CHECK but throwing an \ref MfxSuiteException exception if status
 * is not \ref kOfxStatOK
 */
#define MFX_ENSURE(call) { \
	OfxStatus status = this->host().call; \
	if (kOfxStatOK != status) { \
		throw MfxSuiteException(status, #call); \
	} \
}

// Typedefs for value types

using int2 = std::array<int,2>;
using int3 = std::array<int,3>;
using float2 = std::array<float, 2>;
using float3 = std::array<float, 3>;
using double2 = std::array<double,2>;
using double3 = std::array<double,3>;
