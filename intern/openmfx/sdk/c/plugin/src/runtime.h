#ifndef __MFX_SDK_PLUGIN_RUNTIME__
#define __MFX_SDK_PLUGIN_RUNTIME__

#include <ofxCore.h>
#include <ofxMeshEffect.h>
#include <ofxProperty.h>
#include <ofxParam.h>

typedef struct MfxPluginRuntime {
	OfxHost* host;
	const OfxPropertySuiteV1* propertySuite;
	const OfxParameterSuiteV1* parameterSuite;
	const OfxMeshEffectSuiteV1* meshEffectSuite;
} MfxPluginRuntime;

void mfxRuntimeSetHost(MfxPluginRuntime *runtime, OfxHost *host);

OfxStatus mfxRuntimeCheckSuites(const MfxPluginRuntime *runtime);

#endif // __MFX_SDK_PLUGIN_RUNTIME__
