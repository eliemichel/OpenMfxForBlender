#include "runtime.h"

void mfxRuntimeSetHost(MfxPluginRuntime *runtime, OfxHost *host) {
    runtime->host = host;
    runtime->meshEffectSuite = host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);
    runtime->propertySuite = host->fetchSuite(host->host, kOfxPropertySuite, 1);
    runtime->parameterSuite = host->fetchSuite(host->host, kOfxParameterSuite, 1);
}

OfxStatus mfxRuntimeCheckSuites(const MfxPluginRuntime *runtime) {
    if (NULL == runtime->meshEffectSuite ||
        NULL == runtime->propertySuite ||
        NULL == runtime->parameterSuite) {
        return kOfxStatErrMissingHostFeature;
    }
    return kOfxStatOK;
}
