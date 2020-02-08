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

static OfxStatus load() {
    return kOfxStatOK;
}

static OfxStatus unload() {
    return kOfxStatOK;
}

static OfxStatus describe(OfxMeshEffectHandle descriptor) {
    bool missing_suite =
        NULL == gRuntime.propertySuite ||
        NULL == gRuntime.parameterSuite ||
        NULL == gRuntime.meshEffectSuite;
    if (missing_suite) {
        return kOfxStatErrMissingHostFeature;
    }
    OfxMeshEffectSuiteV1 *meshEffectSuite = gRuntime.meshEffectSuite;
    OfxPropertySuiteV1 *propertySuite = gRuntime.propertySuite;
    OfxParameterSuiteV1 *parameterSuite = gRuntime.parameterSuite;

    OfxPropertySetHandle inputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, &inputProperties);
    propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");

    OfxPropertySetHandle outputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, &outputProperties);
    propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");

    OfxParamSetHandle parameters;
    meshEffectSuite->getParamSet(descriptor, &parameters);
    // "axis" parameter is a bitmask, with threee booleans respectively for axes x, y and z.
    parameterSuite->paramDefine(parameters, kOfxParamTypeInteger, "axis", NULL);

    return kOfxStatOK;
}

static OfxStatus createInstance(OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

static OfxStatus destroyInstance(OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

static OfxStatus cook(OfxMeshEffectHandle instance) {
    OfxMeshEffectSuiteV1 *meshEffectSuite = gRuntime.meshEffectSuite;
    OfxPropertySuiteV1 *propertySuite = gRuntime.propertySuite;
    OfxParameterSuiteV1 *parameterSuite = gRuntime.parameterSuite;
    OfxTime time = 0;

    // Get input/output
    OfxMeshInputHandle input, output;
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainInput, &input, NULL);
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainOutput, &output, NULL);

    // Get meshes
    OfxMeshHandle input_mesh, output_mesh;
    OfxPropertySetHandle input_mesh_prop, output_mesh_prop;
    meshEffectSuite->inputGetMesh(input, time, &input_mesh, &input_mesh_prop);
    meshEffectSuite->inputGetMesh(output, time, &output_mesh, &output_mesh_prop);

    // Get parameters
    OfxParamSetHandle parameters;
    OfxParamHandle axis_param;
    int axis_value;
    meshEffectSuite->getParamSet(instance, &parameters);
    parameterSuite->paramGetHandle(parameters, "axis", &axis_param, NULL);
    parameterSuite->paramGetValue(axis_param, &axis_value);

    // Get input mesh data
    int input_point_count = 0, input_vertex_count = 0, input_face_count = 0;
    float *input_points;
    int *input_vertices, *input_faces;
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropPointCount, 0, &input_point_count);
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropVertexCount, 0, &input_vertex_count);
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropFaceCount, 0, &input_face_count);

    // Get attribute pointers
    OfxPropertySetHandle pos_attrib, vertpoint_attrib, facecounts_attrib;
    meshEffectSuite->meshGetAttribute(input_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib);
    propertySuite->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void**)&input_points);
    meshEffectSuite->meshGetAttribute(input_mesh, kOfxMeshAttribVertex, kOfxMeshAttribVertexPoint, &vertpoint_attrib);
    propertySuite->propGetPointer(vertpoint_attrib, kOfxMeshAttribPropData, 0, (void**)&input_vertices);
    meshEffectSuite->meshGetAttribute(input_mesh, kOfxMeshAttribFace, kOfxMeshAttribFaceCounts, &facecounts_attrib);
    propertySuite->propGetPointer(facecounts_attrib, kOfxMeshAttribPropData, 0, (void**)&input_faces);

    // Allocate output mesh
    int output_point_count = 2 * input_point_count;
    int output_vertex_count = 2 * input_vertex_count;
    int output_face_count = 2 * input_face_count;

    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropPointCount, 0, output_point_count);
    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropVertexCount, 0, output_vertex_count);
    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropFaceCount, 0, output_face_count);

    meshEffectSuite->meshAlloc(output_mesh);

    // Get output mesh data
    float *output_points;
    int *output_vertices, *output_faces;
    meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib);
    propertySuite->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void**)&output_points);
    meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribVertex, kOfxMeshAttribVertexPoint, &vertpoint_attrib);
    propertySuite->propGetPointer(vertpoint_attrib, kOfxMeshAttribPropData, 0, (void**)&output_vertices);
    meshEffectSuite->meshGetAttribute(output_mesh, kOfxMeshAttribFace, kOfxMeshAttribFaceCounts, &facecounts_attrib);
    propertySuite->propGetPointer(facecounts_attrib, kOfxMeshAttribPropData, 0, (void**)&output_faces);

    // Fill in output data
    for (int i = 0 ; i < 3 * input_point_count ; ++i) {
        float value = input_points[i];
        output_points[i] = value;
        if ((axis_value & (1 << (i % 3))) != 0) value = -value;
        output_points[i + 3 * input_point_count] = value;
    }
    for (int i = 0 ; i < input_vertex_count ; ++i) {
        output_vertices[i] = input_vertices[i];
        output_vertices[i + input_vertex_count]
            = input_point_count + input_vertices[i];
    }
    for (int i = 0 ; i < input_face_count ; ++i) {
        output_faces[i] = input_faces[i];
        output_faces[i + input_face_count] = input_faces[i];
    }

    // Release meshes
    meshEffectSuite->inputReleaseMesh(input_mesh);
    meshEffectSuite->inputReleaseMesh(output_mesh);
    return kOfxStatOK;
}

static OfxStatus mainEntry(const char *action,
                           const void *handle,
                           OfxPropertySetHandle inArgs,
                           OfxPropertySetHandle outArgs) {
    if (0 == strcmp(action, kOfxActionLoad)) {
        return load();
    }
    if (0 == strcmp(action, kOfxActionUnload)) {
        return unload();
    }
    if (0 == strcmp(action, kOfxActionDescribe)) {
        return describe((OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxActionCreateInstance)) {
        return createInstance((OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxActionDestroyInstance)) {
        return destroyInstance((OfxMeshEffectHandle)handle);
    }
    if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
        return cook((OfxMeshEffectHandle)handle);
    }
    return kOfxStatReplyDefault;
}

static void setHost(OfxHost *host) {
    gRuntime.host = host;
    if (NULL != host) {
      gRuntime.propertySuite = host->fetchSuite(host->host, kOfxPropertySuite, 1);
      gRuntime.parameterSuite = host->fetchSuite(host->host, kOfxParameterSuite, 1);
      gRuntime.meshEffectSuite = host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);
    }
}

OfxExport int OfxGetNumberOfPlugins(void) {
    return 1;
}

OfxExport OfxPlugin *OfxGetPlugin(int nth) {
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
