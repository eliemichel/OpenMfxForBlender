/*
 * Copyright 2019 - 2020 Elie Michel
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

/**
 * Test plugin using all supported parameters types.
 */

#include "util/ofx_util.h"

#include "ofxCore.h"
#include "ofxParam.h"
#include "ofxMeshEffect.h"
#include "ofxMessage.h"

#include <stdio.h>
#include <stdbool.h>
#include <string.h>


#define MFX_CHECK(expr) \
  status = runtime->expr; \
  if (kOfxStatOK != status) { \
    printf("%s returned error status: %d (%s)\n", #expr, status, getOfxStateName(status)); \
  }

typedef struct PluginRuntime {
    OfxHost *host;
    OfxPropertySuiteV1 *propertySuite;
    OfxParameterSuiteV1 *parameterSuite;
    OfxMeshEffectSuiteV1 *meshEffectSuite;
    OfxMessageSuiteV2 *messageSuite;
} PluginRuntime;

PluginRuntime plugin0_runtime;
PluginRuntime plugin1_runtime;

static OfxStatus plugin0_load(PluginRuntime *runtime) {
    OfxHost *h = runtime->host;
    runtime->propertySuite = (OfxPropertySuiteV1*)h->fetchSuite(h->host, kOfxPropertySuite, 1);
    runtime->parameterSuite = (OfxParameterSuiteV1*)h->fetchSuite(h->host, kOfxParameterSuite, 1);
    runtime->meshEffectSuite = (OfxMeshEffectSuiteV1*)h->fetchSuite(h->host, kOfxMeshEffectSuite, 1);
    runtime->messageSuite = (OfxMessageSuiteV2*)h->fetchSuite(h->host, kOfxMessageSuite, 2);
    return kOfxStatOK;
}

static OfxStatus plugin0_describe(const PluginRuntime *runtime, OfxMeshEffectHandle meshEffect) {
    if (NULL == runtime->propertySuite || NULL == runtime->meshEffectSuite) {
        return kOfxStatErrMissingHostFeature;
    }

    OfxStatus status;
    OfxPropertySetHandle propHandle;

    MFX_CHECK(meshEffectSuite->getPropertySet(meshEffect, &propHandle));

    MFX_CHECK(propertySuite->propSetString(propHandle, kOfxMeshEffectPropContext, 0, kOfxMeshEffectContextFilter));

    // Shall move into "describe in context" when it will exist
    OfxPropertySetHandle inputProperties;
    MFX_CHECK(meshEffectSuite->inputDefine(meshEffect, kOfxMeshMainInput, NULL, &inputProperties));

    MFX_CHECK(propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input"));

    //MFX_CHECK(propertySuite->propSetInt(inputProperties, kOfxInputPropRequireTransformMatrix, 0, 1));

    OfxPropertySetHandle outputProperties;
    MFX_CHECK(meshEffectSuite->inputDefine(meshEffect, kOfxMeshMainOutput, NULL, &outputProperties));

    MFX_CHECK(propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output"));

    // Declare parameters
    OfxParamSetHandle parameters;
    MFX_CHECK(meshEffectSuite->getParamSet(meshEffect, &parameters));

    OfxPropertySetHandle paramProps;

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeInteger, "Count", &paramProps));
    runtime->propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 0, 15);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeInteger2D, "Count2D", &paramProps));
    int int2D[2] = { 25, 87 };
    runtime->propertySuite->propSetIntN(paramProps, kOfxParamPropDefault, 2, int2D);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeInteger3D, "Count3D", &paramProps));
    int int3D[3] = { 12, 45, 1569 };
    runtime->propertySuite->propSetIntN(paramProps, kOfxParamPropDefault, 3, int3D);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeDouble, "Distance", &paramProps));
    runtime->propertySuite->propSetDouble(paramProps, kOfxParamPropDefault, 0, 17.0);
    runtime->propertySuite->propSetDouble(paramProps, kOfxParamPropMin, 0, -10.0);
    runtime->propertySuite->propSetDouble(paramProps, kOfxParamPropMax, 0, 110.0);
    runtime->propertySuite->propSetDouble(paramProps, kOfxParamPropDisplayMin, 0, 0.0);
    runtime->propertySuite->propSetDouble(paramProps, kOfxParamPropDisplayMax, 0, 100.0);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeDouble2D, "Vector2D", &paramProps));
    double double2D[2] = { 12.89, 1.02369 };
    runtime->propertySuite->propSetDoubleN(paramProps, kOfxParamPropDefault, 2, double2D);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeDouble3D, "Vector3D", &paramProps));
    double double3D[3] = { -159.51, 0.00416, 2257896.123 };
    runtime->propertySuite->propSetDoubleN(paramProps, kOfxParamPropDefault, 3, double3D);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeRGB, "Color", &paramProps));
    double rgb[3] = { 0.025363, 0.608, 0.62 };
    runtime->propertySuite->propSetDoubleN(paramProps, kOfxParamPropDefault, 3, rgb);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeRGBA, "RGBA Color", &paramProps));
    double rgba[4] = { 0.72, 0.058068, 0.14, 0.5 };
    runtime->propertySuite->propSetDoubleN(paramProps, kOfxParamPropDefault, 4, rgba);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeBoolean, "Enable Option", &paramProps));
    runtime->propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 0, (int)true);

    MFX_CHECK(parameterSuite->paramDefine(parameters, kOfxParamTypeString, "Description", &paramProps));
    runtime->propertySuite->propSetString(paramProps, kOfxParamPropDefault, 0, "Description here!");

    return kOfxStatOK;
}

static OfxStatus plugin0_cook(PluginRuntime *runtime, OfxMeshEffectHandle meshEffect) {
    OfxStatus status;
    OfxParamSetHandle parameters;
    OfxParamHandle param;
    int i1, i2, i3;
    double d1, d2, d3, d4;
    bool b;
    char *str = NULL;

    printf("Cooking Test Parameters Plugin...\n");

    OfxMeshInputHandle input;
    OfxPropertySetHandle inputMeshProperties;
    OfxTime time = 0;
    OfxMeshHandle inputMesh;
    MFX_CHECK(meshEffectSuite->inputGetHandle(meshEffect, kOfxMeshMainInput, &input, NULL));
    MFX_CHECK(meshEffectSuite->inputGetMesh(input, time, &inputMesh, &inputMeshProperties));
    double *matrix;
    MFX_CHECK(propertySuite->propGetPointer(inputMeshProperties, kOfxMeshPropTransformMatrix, 0, (void **)&matrix));
    printf("Input transform matrix: [\n");
    for (int i = 0; i < 4; ++i) {
      printf("[%f, %f, %f, %f],\n",
              matrix[4 * i + 0],
              matrix[4 * i + 1],
              matrix[4 * i + 2],
              matrix[4 * i + 3]);
    }
    printf("]\n");

    MFX_CHECK(meshEffectSuite->getParamSet(meshEffect, &parameters));

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Count", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &i1));
    printf(" - Parameter 'Count' (kOfxParamTypeInteger): %d\n", i1);

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Count2D", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &i1, &i2));
    printf(" - Parameter 'Count2D' (kOfxParamTypeInteger2D): (%d, %d)\n", i1, i2);

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Count3D", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &i1, &i2, &i3));
    printf(" - Parameter 'Count3D' (kOfxParamTypeInteger3D): (%d, %d, %d)\n", i1, i2, i3);

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Distance", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &d1));
    printf(" - Parameter 'Distance' (kOfxParamTypeDouble): %f\n", d1);

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Vector2D", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &d1, &d2));
    printf(" - Parameter 'Vector2D' (kOfxParamTypeDouble2D): (%f, %f)\n", d1, d2);

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Vector3D", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &d1, &d2, &d3));
    printf(" - Parameter 'Vector3D' (kOfxParamTypeDouble3D): (%f, %f, %f)\n", d1, d2, d3);

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Color", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &d1, &d2, &d3));
    printf(" - Parameter 'Color' (kOfxParamTypeRGB): (%f, %f, %f)\n", d1, d2, d3);

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "RGBA Color", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &d1, &d2, &d3, &d4));
    printf(" - Parameter 'RGBA Color' (kOfxParamTypeRGBA): (%f, %f, %f, %f)\n", d1, d2, d3, d4);

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Enable Option", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &b));
    printf(" - Parameter 'Enable Option' (kOfxParamTypeBoolean): (%s)\n", b ? "true" : "false");

    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Description", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &str));
    printf(" - Parameter 'Description' (kOfxParamTypeString): (%s)\n", str);

    // Also test messaging system
    runtime->messageSuite->message(
      meshEffect, kOfxMessageMessage, NULL, "mfx_test_parameters_plugin cooked successfully.");
    runtime->messageSuite->setPersistentMessage(
        meshEffect, kOfxMessageMessage, NULL, "mfx_test_parameters_plugin cooked successfully!");
    runtime->messageSuite->setPersistentMessage(
        meshEffect, kOfxMessageError, NULL, "oh no, an error!");
    
    return kOfxStatOK;
}

static OfxStatus plugin0_mainEntry(const char *action,
                                   const void *handle,
                                   OfxPropertySetHandle inArgs,
                                   OfxPropertySetHandle outArgs) {
    if (0 == strcmp(action, kOfxActionLoad)) {
        return plugin0_load(&plugin0_runtime);
    }
    if (0 == strcmp(action, kOfxActionDescribe)) {
        return plugin0_describe(&plugin0_runtime, (OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxActionCreateInstance)) {
        return kOfxStatOK;
    }
    if (0 == strcmp(action, kOfxActionDestroyInstance)) {
        return kOfxStatOK;
    }
    if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
        return plugin0_cook(&plugin0_runtime, (OfxMeshEffectHandle)handle);
    }
    return kOfxStatReplyDefault;
}

static void plugin0_setHost(OfxHost *host) {
    plugin0_runtime.host = host;
}

OfxExport int OfxGetNumberOfPlugins(void) {
    return 1;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
    static OfxPlugin plugins[] = {
        {
        /* pluginApi */          kOfxMeshEffectPluginApi,
        /* apiVersion */         kOfxMeshEffectPluginApiVersion,
        /* pluginIdentifier */   "MfxTestParameters",
        /* pluginVersionMajor */ 1,
        /* pluginVersionMinor */ 0,
        /* setHost */            plugin0_setHost,
        /* mainEntry */          plugin0_mainEntry
        },
    };
    return plugins + nth;
}
