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

/**
 * TODO: At the moment this only copies the position attribute and topology. We should copy all
 * available attributes but there is no mechanism yet to query the list of existing attributes.
 */

#include <stdbool.h>
#include <string.h>

#include "ofxCore.h"
#include "ofxMeshEffect.h"
#include "util/plugin_support.h"

static OfxStatus describe(PluginRuntime *runtime, OfxMeshEffectHandle descriptor) {
    bool missing_suite =
        NULL == runtime->propertySuite ||
        NULL == runtime->parameterSuite ||
        NULL == runtime->meshEffectSuite;
    if (missing_suite) {
        return kOfxStatErrMissingHostFeature;
    }
    const OfxMeshEffectSuiteV1 *meshEffectSuite = runtime->meshEffectSuite;
    const OfxPropertySuiteV1 *propertySuite = runtime->propertySuite;

    OfxPropertySetHandle inputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, NULL, &inputProperties);
    propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");

    OfxPropertySetHandle outputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, NULL, &outputProperties);
    propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");

    return kOfxStatOK;
}

static OfxStatus cook(PluginRuntime *runtime, OfxMeshEffectHandle instance) {
    const OfxMeshEffectSuiteV1 *meshEffectSuite = runtime->meshEffectSuite;
    const OfxPropertySuiteV1 *propertySuite = runtime->propertySuite;
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

    // Get input mesh data
    int input_point_count = 0, input_vertex_count = 0, input_face_count = 0;
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropPointCount, 0, &input_point_count);
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropVertexCount, 0, &input_vertex_count);
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropFaceCount, 0, &input_face_count);


    // Allocate output mesh
    int output_point_count = input_point_count;
    int output_vertex_count = input_vertex_count;
    int output_face_count = input_face_count;

    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropPointCount, 0, output_point_count);
    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropVertexCount, 0, output_vertex_count);
    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropFaceCount, 0, output_face_count);

    meshEffectSuite->meshAlloc(output_mesh);



    // Fill in output data
    Attribute input_pos, output_pos;
    getPointAttribute(input_mesh, kOfxMeshAttribPointPosition, &input_pos);
    getPointAttribute(output_mesh, kOfxMeshAttribPointPosition, &output_pos);
    copyAttribute(&output_pos, &input_pos, 0, input_point_count);

    Attribute input_vertpoints, output_vertpoint;
    getPointAttribute(input_mesh, kOfxMeshAttribVertexPoint, &input_vertpoints);
    getPointAttribute(output_mesh, kOfxMeshAttribVertexPoint, &output_vertpoint);
    copyAttribute(&output_vertpoint, &input_vertpoints, 0, input_vertex_count);

    Attribute input_facecounts, output_facecounts;
    getPointAttribute(input_mesh, kOfxMeshAttribFaceCounts, &input_facecounts);
    getPointAttribute(output_mesh, kOfxMeshAttribFaceCounts, &output_facecounts);
    copyAttribute(&output_facecounts, &input_facecounts, 0, input_face_count);

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
