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

#include <stdbool.h>
#include <string.h>

#include "ofxCore.h"
#include "ofxMeshEffect.h"

typedef struct PluginRuntime {
    OfxHost *host;
    OfxPropertySuiteV1 *propertySuite;
    OfxParameterSuiteV1 *parameterSuite;
    OfxMeshEffectSuiteV1 *meshEffectSuite;
} PluginRuntime;

PluginRuntime gRuntime;

static OfxStatus describe(PluginRuntime *runtime, OfxMeshEffectHandle descriptor) {
    bool missing_suite =
        NULL == runtime->propertySuite ||
        NULL == runtime->parameterSuite ||
        NULL == runtime->meshEffectSuite;
    if (missing_suite) {
        return kOfxStatErrMissingHostFeature;
    }
    OfxMeshEffectSuiteV1 *meshEffectSuite = runtime->meshEffectSuite;
    OfxPropertySuiteV1 *propertySuite = runtime->propertySuite;

    OfxPropertySetHandle inputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, &inputProperties);
    propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");

    OfxPropertySetHandle outputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, &outputProperties);
    propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");

    return kOfxStatOK;
}

static OfxStatus cook(PluginRuntime *runtime, OfxMeshEffectHandle instance) {
    OfxMeshEffectSuiteV1 *meshEffectSuite = runtime->meshEffectSuite;
    OfxPropertySuiteV1 *propertySuite = runtime->propertySuite;
    OfxTime time = 0;

    // Get input/output
    OfxMeshInputHandle input, output;
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainInput, &input, NULL);
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainOutput, &output, NULL);

    // Get meshes
    OfxPropertySetHandle input_mesh, output_mesh;
    meshEffectSuite->inputGetMesh(input, time, &input_mesh);
    meshEffectSuite->inputGetMesh(output, time, &output_mesh);

    // Get input mesh data
    int input_point_count = 0, input_vertex_count = 0, input_face_count = 0;
    float *input_points;
    int *input_vertices, *input_faces;
    propertySuite->propGetInt(input_mesh, kOfxMeshPropPointCount,
                              0, &input_point_count);
    propertySuite->propGetInt(input_mesh, kOfxMeshPropVertexCount,
                              0, &input_vertex_count);
    propertySuite->propGetInt(input_mesh, kOfxMeshPropFaceCount,
                              0, &input_face_count);
    propertySuite->propGetPointer(input_mesh, kOfxMeshPropPointData,
                                  0, (void**)&input_points);
    propertySuite->propGetPointer(input_mesh, kOfxMeshPropVertexData,
                                  0, (void**)&input_vertices);
    propertySuite->propGetPointer(input_mesh, kOfxMeshPropFaceData,
                                  0, (void**)&input_faces);

    // Allocate output mesh
    int output_point_count = input_point_count;
    int output_vertex_count = input_vertex_count;
    int output_face_count = input_face_count;
    meshEffectSuite->meshAlloc(output_mesh,
                               output_point_count,
                               output_vertex_count,
                               output_face_count);

    // Get output mesh data
    float *output_points;
    int *output_vertices, *output_faces;
    propertySuite->propGetPointer(output_mesh, kOfxMeshPropPointData,
                                  0, (void**)&output_points);
    propertySuite->propGetPointer(output_mesh, kOfxMeshPropVertexData,
                                  0, (void**)&output_vertices);
    propertySuite->propGetPointer(output_mesh, kOfxMeshPropFaceData,
                                  0, (void**)&output_faces);

    // Fill in output data
    memcpy(output_points, input_points, 3 * input_point_count * sizeof(float));
    memcpy(output_vertices, input_vertices, input_vertex_count * sizeof(int));
    memcpy(output_faces, input_faces, 3 * input_face_count * sizeof(int));

    // Release meshes
    meshEffectSuite->inputReleaseMesh(input_mesh);
    meshEffectSuite->inputReleaseMesh(output_mesh);
    return kOfxStatOK;
}

static OfxStatus mainEntry(const char *action,
                           const void *handle,
                           OfxPropertySetHandle inArgs,
                           OfxPropertySetHandle outArgs) {
    (void)inArgs;
    (void)outArgs;
    if (0 == strcmp(action, kOfxActionLoad)) {
        return kOfxStatOK;
    }
    if (0 == strcmp(action, kOfxActionUnload)) {
        return kOfxStatOK;
    }
    if (0 == strcmp(action, kOfxActionDescribe)) {
        return describe(&gRuntime, (OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxActionCreateInstance)) {
        return kOfxStatOK;
    }
    if (0 == strcmp(action, kOfxActionDestroyInstance)) {
        return kOfxStatOK;
    }
    if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
        return cook(&gRuntime, (OfxMeshEffectHandle)handle);
    }
    return kOfxStatReplyDefault;
}

static void setHost(OfxHost *host) {
    gRuntime.host = host;
    gRuntime.propertySuite = host->fetchSuite(host->host, kOfxPropertySuite, 1);
    gRuntime.parameterSuite = host->fetchSuite(host->host, kOfxParameterSuite, 1);
    gRuntime.meshEffectSuite = host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);
}

OfxExport int OfxGetNumberOfPlugins(void) {
    return 1;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
    (void)nth;
    static OfxPlugin plugin = {
        /* pluginApi */          kOfxMeshEffectPluginApi,
        /* apiVersion */         kOfxMeshEffectPluginApiVersion,
        /* pluginIdentifier */   "MirrorPlugin",
        /* pluginVersionMajor */ 1,
        /* pluginVersionMinor */ 0,
        /* setHost */            setHost,
        /* mainEntry */          mainEntry
    };
    return &plugin;
}
