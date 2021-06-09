/*
 * Copyright 2019-2021 Elie Michel
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

#include "util/ofx_util.h"

#include "ofxCore.h"
#include "ofxParam.h"
#include "ofxMeshEffect.h"

#include <stdio.h>
#include <string.h>

#define MFX_CHECK(op) status = op; \
if (kOfxStatOK != status) \
    printf("Suite method '" #op "' returned status %d (%s)\n", status, getOfxStateName(status));

typedef struct PluginRuntime {
    OfxHost *host;
    OfxPropertySuiteV1 *propertySuite;
    OfxParameterSuiteV1 *parameterSuite;
    OfxMeshEffectSuiteV1 *meshEffectSuite;
} PluginRuntime;

PluginRuntime plugin0_runtime;
PluginRuntime plugin1_runtime;

static OfxStatus plugin0_load(PluginRuntime *runtime) {
    OfxHost *h = runtime->host;
    runtime->propertySuite = (OfxPropertySuiteV1*)h->fetchSuite(h->host, kOfxPropertySuite, 1);
    runtime->parameterSuite = (OfxParameterSuiteV1*)h->fetchSuite(h->host, kOfxParameterSuite, 1);
    runtime->meshEffectSuite = (OfxMeshEffectSuiteV1*)h->fetchSuite(h->host, kOfxMeshEffectSuite, 1);
    return kOfxStatOK;
}

static OfxStatus plugin0_describe(const PluginRuntime *runtime, OfxMeshEffectHandle meshEffect) {
    if (NULL == runtime->propertySuite || NULL == runtime->meshEffectSuite) {
        return kOfxStatErrMissingHostFeature;
    }

    OfxStatus status;
    OfxPropertySetHandle propHandle;

    MFX_CHECK(runtime->meshEffectSuite->getPropertySet(meshEffect, &propHandle));

    MFX_CHECK(runtime->propertySuite->propSetString(propHandle, kOfxMeshEffectPropContext, 0, kOfxMeshEffectContextFilter));

    // Shall move into "describe in context" when it will exist
    OfxPropertySetHandle inputProperties;
    MFX_CHECK(runtime->meshEffectSuite->inputDefine(meshEffect, kOfxMeshMainInput, NULL, &inputProperties));
    
    MFX_CHECK(runtime->propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input"));
    
    OfxPropertySetHandle outputProperties;
    MFX_CHECK(runtime->meshEffectSuite->inputDefine(meshEffect, kOfxMeshMainOutput, NULL, &outputProperties)); // yes, output are also "inputs", I should change this name in the API

    MFX_CHECK(runtime->propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output"));

    // Declare parameters
    OfxParamSetHandle parameters;
    OfxParamHandle param;
    MFX_CHECK(runtime->meshEffectSuite->getParamSet(meshEffect, &parameters));

    MFX_CHECK(runtime->parameterSuite->paramDefine(parameters, kOfxParamTypeDouble, "width", NULL));

    MFX_CHECK(runtime->parameterSuite->paramDefine(parameters, kOfxParamTypeInteger, "steps", NULL));

    MFX_CHECK(runtime->parameterSuite->paramDefine(parameters, kOfxParamTypeString, "path", NULL));

    return kOfxStatOK;
}

static OfxStatus plugin0_cook(PluginRuntime *runtime, OfxMeshEffectHandle meshEffect) {
    OfxStatus status;
    OfxMeshInputHandle input, output;
    OfxPropertySetHandle propertySet;

    MFX_CHECK(runtime->meshEffectSuite->inputGetHandle(meshEffect, kOfxMeshMainInput, &input, &propertySet));
    if (status != kOfxStatOK) {
        return kOfxStatErrUnknown;
    }

    MFX_CHECK(runtime->meshEffectSuite->inputGetHandle(meshEffect, kOfxMeshMainOutput, &output, &propertySet));
    if (status != kOfxStatOK) {
        return kOfxStatErrUnknown;
    }

    OfxTime time = 0;
    OfxMeshHandle input_mesh;
    OfxPropertySetHandle input_mesh_prop;
    MFX_CHECK(runtime->meshEffectSuite->inputGetMesh(input, time, &input_mesh, &input_mesh_prop));

    int input_point_count;
    MFX_CHECK(runtime->propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropPointCount, 0, &input_point_count));

    OfxPropertySetHandle pos_attrib;
    MFX_CHECK(runtime->meshEffectSuite->meshGetAttribute(input_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib));

    float *input_points;
    MFX_CHECK(runtime->propertySuite->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void**)&input_points));

    printf("DEBUG: Found %d points in input mesh\n", input_point_count);

    MFX_CHECK(runtime->meshEffectSuite->inputReleaseMesh(input_mesh));

    // Get parameters
    OfxParamSetHandle parameters;
    OfxParamHandle param;
    MFX_CHECK(runtime->meshEffectSuite->getParamSet(meshEffect, &parameters));

    MFX_CHECK(runtime->parameterSuite->paramGetHandle(parameters, "width", &param, NULL));

    double width;
    MFX_CHECK(runtime->parameterSuite->paramGetValue(param, &width));

    printf("-- width parameter set to: %f\n", width);

    // TODO: core cook

    OfxMeshHandle output_mesh;
    OfxPropertySetHandle output_mesh_prop;
    MFX_CHECK(runtime->meshEffectSuite->inputGetMesh(output, time, &output_mesh, &output_mesh_prop));

    int output_point_count = 0, output_corner_count = 0, output_face_count = 0;

    // TODO: Consolidate geo counts
    output_point_count = 4;
    output_corner_count = 4;
    output_face_count = 1;

    printf("DEBUG: Allocating output mesh data: %d points, %d corners, %d faces\n", output_point_count, output_corner_count, output_face_count);

    MFX_CHECK(runtime->propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropPointCount, 0, output_point_count));

    MFX_CHECK(runtime->propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropCornerCount, 0, output_corner_count));

    MFX_CHECK(runtime->propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropFaceCount, 0, output_face_count));

    MFX_CHECK(runtime->meshEffectSuite->meshAlloc(output_mesh));

    MFX_CHECK(runtime->meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib));

    OfxPropertySetHandle cornerpoint_attrib;
    MFX_CHECK(runtime->meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribPoint, kOfxMeshAttribCornerPoint, &cornerpoint_attrib));

    OfxPropertySetHandle facesize_attrib;
    MFX_CHECK(runtime->meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribPoint, kOfxMeshAttribFaceSize, &facesize_attrib));

    float *output_points;
    MFX_CHECK(runtime->propertySuite->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void**)&output_points));

    int *output_corners;
    MFX_CHECK(runtime->propertySuite->propGetPointer(cornerpoint_attrib, kOfxMeshAttribPropData, 0, (void**)&output_corners));

    int *output_faces;
    MFX_CHECK(runtime->propertySuite->propGetPointer(facesize_attrib, kOfxMeshAttribPropData, 0, (void**)&output_faces));

    // Warning! This example is simplistic and abusively assumes that data is contiguous.
    // Even if it might be common, plugin developpers should use the stride of attributes.
    
    output_points[0 * 3 + 0] = -1.0f;
    output_points[0 * 3 + 1] = -width;
    output_points[0 * 3 + 2] = 0.0f;

    output_points[1 * 3 + 0] = 1.0f;
    output_points[1 * 3 + 1] = -width;
    output_points[1 * 3 + 2] = 0.0f;

    output_points[2 * 3 + 0] = 1.0f;
    output_points[2 * 3 + 1] = width;
    output_points[2 * 3 + 2] = 0.0f;

    output_points[3 * 3 + 0] = -1.0f;
    output_points[3 * 3 + 1] = width;
    output_points[3 * 3 + 2] = 0.0f;

    for (int i = 0 ; i < 4 ; ++i) output_corners[i] = i;

    output_faces[0] = 4;

    MFX_CHECK(runtime->meshEffectSuite->inputReleaseMesh(output_mesh));

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

static OfxStatus plugin1_mainEntry(const char *action,
                                   const void *handle,
                                   OfxPropertySetHandle inArgs,
                                   OfxPropertySetHandle outArgs) {
    (void)action;
    (void)handle;
    (void)inArgs;
    (void)outArgs;
    return kOfxStatReplyDefault;
}

static void plugin1_setHost(OfxHost *host) {
    plugin1_runtime.host = host;
}

OfxExport int OfxGetNumberOfPlugins(void) {
    return 2;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
    static OfxPlugin plugins[] = {
        {
        /* pluginApi */          kOfxMeshEffectPluginApi,
        /* apiVersion */         kOfxMeshEffectPluginApiVersion,
        /* pluginIdentifier */   "MfxSamplePlugin0",
        /* pluginVersionMajor */ 1,
        /* pluginVersionMinor */ 0,
        /* setHost */            plugin0_setHost,
        /* mainEntry */          plugin0_mainEntry
        },
        {
        /* pluginApi */          kOfxMeshEffectPluginApi,
        /* apiVersion */         kOfxMeshEffectPluginApiVersion,
        /* pluginIdentifier */   "MfxSamplePlugin1",
        /* pluginVersionMajor */ 1,
        /* pluginVersionMinor */ 0,
        /* setHost */            plugin1_setHost,
        /* mainEntry */          plugin1_mainEntry
        }
    };
    return plugins + nth;
}
