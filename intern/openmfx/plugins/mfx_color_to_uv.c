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

/**
 * This plugin is a test for corner attributes, transfering corner colors to UVs.
 */

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "ofxCore.h"
#include "ofxMeshEffect.h"
#include "util/plugin_support.h"

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
    const OfxMeshEffectSuiteV1 *meshEffectSuite = gRuntime.meshEffectSuite;
    const OfxPropertySuiteV1 *propertySuite = gRuntime.propertySuite;

    OfxPropertySetHandle inputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, NULL, &inputProperties);
    propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");

    OfxPropertySetHandle outputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, NULL, &outputProperties);
    propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");

    return kOfxStatOK;
}

static OfxStatus createInstance(OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

static OfxStatus destroyInstance(OfxMeshEffectHandle instance) {
    return kOfxStatOK;
}

static OfxStatus cook(OfxMeshEffectHandle instance) {
    const OfxMeshEffectSuiteV1 *meshEffectSuite = gRuntime.meshEffectSuite;
    const OfxPropertySuiteV1 *propertySuite = gRuntime.propertySuite;
    OfxTime time = 0;
    OfxStatus status;

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
    int input_point_count = 0, input_corner_count = 0, input_face_count = 0;
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropPointCount, 0, &input_point_count);
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropCornerCount, 0, &input_corner_count);
    propertySuite->propGetInt(input_mesh_prop, kOfxMeshPropFaceCount, 0, &input_face_count);

    // Get attribute pointers
   

    // Get corner color
    OfxPropertySetHandle vcolor_attrib, uv_attrib;
    status = meshEffectSuite->meshGetAttribute(input_mesh, kOfxMeshAttribCorner, "color0", &vcolor_attrib);

    printf("Look for color0...\n");
    char *vcolor_data = NULL;
    if (kOfxStatOK == status) {
      printf("found!\n");
      propertySuite->propGetPointer(vcolor_attrib, kOfxMeshAttribPropData, 0, (void**)&vcolor_data);
      meshEffectSuite->attributeDefine(output_mesh, kOfxMeshAttribCorner, "uv0", 2, kOfxMeshAttribTypeFloat, kOfxMeshAttribSemanticTextureCoordinate, &uv_attrib);
      propertySuite->propSetInt(uv_attrib, kOfxMeshAttribPropIsOwner, 0, 1);
    }

    // Allocate output mesh
    int output_point_count = input_point_count;
    int output_corner_count = input_corner_count;
    int output_face_count = input_face_count;

    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropPointCount, 0, output_point_count);
    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropCornerCount, 0, output_corner_count);
    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropFaceCount, 0, output_face_count);

    meshEffectSuite->meshAlloc(output_mesh);

    // Get output mesh data


    // Fill in output data
    Attribute input_pos, output_pos;
    getPointAttribute(input_mesh, kOfxMeshAttribPointPosition, &input_pos);
    getPointAttribute(output_mesh, kOfxMeshAttribPointPosition, &output_pos);
    copyAttribute(&output_pos, &input_pos, 0, input_point_count);

    Attribute input_cornerpoint, output_cornerpoint;
    getCornerAttribute(input_mesh, kOfxMeshAttribCornerPoint, &input_cornerpoint);
    getCornerAttribute(output_mesh, kOfxMeshAttribCornerPoint, &output_cornerpoint);
    copyAttribute(&output_cornerpoint, &input_cornerpoint, 0, input_corner_count);

    Attribute input_facesize, output_facesize;
    getFaceAttribute(input_mesh, kOfxMeshAttribFaceSize, &input_facesize);
    getFaceAttribute(output_mesh, kOfxMeshAttribFaceSize, &output_facesize);
    copyAttribute(&output_facesize, &input_facesize, 0, input_face_count);

    if (NULL != vcolor_data) {
      Attribute input_color, output_uv;
      getCornerAttribute(input_mesh, "color0", &input_color);
      getCornerAttribute(output_mesh, "uv0", &output_uv);
      copyAttribute(&output_uv, &input_color, 0, input_corner_count);
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
        /* pluginIdentifier */   "CornerColor2Uv",
        /* pluginVersionMajor */ 1,
        /* pluginVersionMinor */ 0,
        /* setHost */            setHost,
        /* mainEntry */          mainEntry
    };
    return &plugin;
}
