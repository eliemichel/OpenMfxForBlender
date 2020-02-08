/*
 * Copyright 2019 Elie Michel
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

    status = runtime->meshEffectSuite->getPropertySet(meshEffect, &propHandle);
    printf("Suite method 'getPropertySet' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->propertySuite->propSetString(propHandle, kOfxMeshEffectPropContext, 0, kOfxMeshEffectContextFilter);
    printf("Suite method 'propSetString' returned status %d (%s)\n", status, getOfxStateName(status));

    // Shall move into "describe in context" when it will exist
    OfxPropertySetHandle inputProperties;
    status = runtime->meshEffectSuite->inputDefine(meshEffect, kOfxMeshMainInput, &inputProperties);
    printf("Suite method 'inputDefine' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");
    printf("Suite method 'propSetString' returned status %d (%s)\n", status, getOfxStateName(status));

    OfxPropertySetHandle outputProperties;
    status = runtime->meshEffectSuite->inputDefine(meshEffect, kOfxMeshMainOutput, &outputProperties); // yes, output are also "inputs", I should change this name in the API
    printf("Suite method 'inputDefine' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");
    printf("Suite method 'propSetString' returned status %d (%s)\n", status, getOfxStateName(status));

    // Declare parameters
    OfxParamSetHandle parameters;
    OfxParamHandle param;
    status = runtime->meshEffectSuite->getParamSet(meshEffect, &parameters);
    printf("Suite method 'getParamSet' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->parameterSuite->paramDefine(parameters, kOfxParamTypeDouble, "width", NULL);
    printf("Suite method 'paramDefine' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->parameterSuite->paramDefine(parameters, kOfxParamTypeInteger, "steps", NULL);
    printf("Suite method 'paramDefine' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->parameterSuite->paramDefine(parameters, kOfxParamTypeString, "path", NULL);
    printf("Suite method 'paramDefine' returned status %d (%s)\n", status, getOfxStateName(status));

    return kOfxStatOK;
}

static OfxStatus plugin0_cook(PluginRuntime *runtime, OfxMeshEffectHandle meshEffect) {
    OfxStatus status;
    OfxMeshInputHandle input, output;
    OfxPropertySetHandle propertySet;

    status = runtime->meshEffectSuite->inputGetHandle(meshEffect, kOfxMeshMainInput, &input, &propertySet);
    printf("Suite method 'inputGetHandle' returned status %d (%s)\n", status, getOfxStateName(status));
    if (status != kOfxStatOK) {
        return kOfxStatErrUnknown;
    }

    status = runtime->meshEffectSuite->inputGetHandle(meshEffect, kOfxMeshMainOutput, &output, &propertySet);
    printf("Suite method 'inputGetHandle' returned status %d (%s)\n", status, getOfxStateName(status));
    if (status != kOfxStatOK) {
        return kOfxStatErrUnknown;
    }

    OfxTime time = 0;
    OfxMeshHandle input_mesh;
    OfxPropertySetHandle input_mesh_prop;
    status = runtime->meshEffectSuite->inputGetMesh(input, time, &input_mesh, &input_mesh_prop);
    printf("Suite method 'inputGetMesh' returned status %d (%s)\n", status, getOfxStateName(status));

    int input_point_count;
    status = runtime->propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropPointCount, 0, &input_point_count);
    printf("Suite method 'propGetInt' returned status %d (%s)\n", status, getOfxStateName(status));

    OfxPropertySetHandle pos_attrib;
    status = runtime->meshEffectSuite->meshGetAttribute(input_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib);
    printf("Suite method 'meshGetAttribute' returned status %d (%s)\n", status, getOfxStateName(status));

    float *input_points;
    status = runtime->propertySuite->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void**)&input_points);
    printf("Suite method 'propGetPointer' returned status %d (%s)\n", status, getOfxStateName(status));

    printf("DEBUG: Found %d in input mesh\n", input_point_count);

    // TODO: store input data

    status = runtime->meshEffectSuite->inputReleaseMesh(input_mesh);
    printf("Suite method 'inputReleaseMesh' returned status %d (%s)\n", status, getOfxStateName(status));

    // Get parameters
    OfxParamSetHandle parameters;
    OfxParamHandle param;
    status = runtime->meshEffectSuite->getParamSet(meshEffect, &parameters);
    printf("Suite method 'getParamSet' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->parameterSuite->paramGetHandle(parameters, "width", &param, NULL);
    printf("Suite method 'paramGetHandle' returned status %d (%s)\n", status, getOfxStateName(status));

    double width;
    status = runtime->parameterSuite->paramGetValue(param, &width);
    printf("Suite method 'paramGetValue' returned status %d (%s)\n", status, getOfxStateName(status));

    printf("-- width parameter set to: %f\n", width);

    // TODO: core cook

    OfxMeshHandle output_mesh;
    OfxPropertySetHandle output_mesh_prop;
    status = runtime->meshEffectSuite->inputGetMesh(output, time, &output_mesh, &output_mesh_prop);
    printf("Suite method 'inputGetMesh' returned status %d (%s)\n", status, getOfxStateName(status));

    int output_point_count = 0, output_vertex_count = 0, output_face_count = 0;

    // TODO: Consolidate geo counts
    output_point_count = 4;
    output_vertex_count = 4;
    output_face_count = 1;

    printf("DEBUG: Allocating output mesh data: %d points, %d vertices, %d faces\n", output_point_count, output_vertex_count, output_face_count);

    status = runtime->propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropPointCount, 0, output_point_count);
    printf("Suite method 'propSetInt' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropVertexCount, 0, output_vertex_count);
    printf("Suite method 'propSetInt' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropFaceCount, 0, output_face_count);
    printf("Suite method 'propSetInt' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->meshEffectSuite->meshAlloc(output_mesh);
    printf("Suite method 'meshAlloc' returned status %d (%s)\n", status, getOfxStateName(status));

    status = runtime->meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib);
    printf("Suite method 'meshGetAttribute' returned status %d (%s)\n", status, getOfxStateName(status));

    OfxPropertySetHandle vertpoint_attrib;
    status = runtime->meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribPoint, kOfxMeshAttribVertexPoint, &vertpoint_attrib);
    printf("Suite method 'meshGetAttribute' returned status %d (%s)\n", status, getOfxStateName(status));

    OfxPropertySetHandle facecounts_attrib;
    status = runtime->meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribPoint, kOfxMeshAttribFaceCounts, &facecounts_attrib);
    printf("Suite method 'meshGetAttribute' returned status %d (%s)\n", status, getOfxStateName(status));

    float *output_points;
    status = runtime->propertySuite->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void**)&output_points);
    printf("Suite method 'propGetPointer' returned status %d (%s)\n", status, getOfxStateName(status));

    int *output_vertices;
    status = runtime->propertySuite->propGetPointer(vertpoint_attrib, kOfxMeshAttribPropData, 0, (void**)&output_vertices);
    printf("Suite method 'propGetPointer' returned status %d (%s)\n", status, getOfxStateName(status));

    int *output_faces;
    status = runtime->propertySuite->propGetPointer(facecounts_attrib, kOfxMeshAttribPropData, 0, (void**)&output_faces);
    printf("Suite method 'propGetPointer' returned status %d (%s)\n", status, getOfxStateName(status));

    // TODO: Fill data
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

    for (int i = 0 ; i < 4 ; ++i) output_vertices[i] = i;

    output_faces[0] = 4;

    status = runtime->meshEffectSuite->inputReleaseMesh(output_mesh);
    printf("Suite method 'inputReleaseMesh' returned status %d (%s)\n", status, getOfxStateName(status));

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
