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

#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "ofxCore.h"
#include "ofxMeshEffect.h"

#include "util/ofx_util.h"
#include "util/plugin_support.h"

#define MFX_CHECK(call) \
status = call; \
if (kOfxStatOK != status) { \
  printf("[MFX] Suite method call '" #call "' returned status %d (%s)\n", status, getOfxStateName(status)); \
}

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
    const OfxParameterSuiteV1 *parameterSuite = gRuntime.parameterSuite;
    OfxStatus status;

    OfxPropertySetHandle inputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainInput, NULL, &inputProperties);
    propertySuite->propSetString(inputProperties, kOfxPropLabel, 0, "Main Input");

    OfxPropertySetHandle outputProperties;
    meshEffectSuite->inputDefine(descriptor, kOfxMeshMainOutput, NULL, &outputProperties);
    propertySuite->propSetString(outputProperties, kOfxPropLabel, 0, "Main Output");

    // Declare parameters
    OfxParamSetHandle parameters;
    OfxParamHandle param;
    status = meshEffectSuite->getParamSet(descriptor, &parameters);

    status = parameterSuite->paramDefine(parameters, kOfxParamTypeInteger2D, "Translation", NULL);
    status = parameterSuite->paramDefine(parameters, kOfxParamTypeInteger2D, "Rotation", NULL);
    status = parameterSuite->paramDefine(parameters, kOfxParamTypeInteger2D, "Scale", NULL);

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
    const OfxParameterSuiteV1 *parameterSuite = gRuntime.parameterSuite;
    OfxTime time = 0;
    OfxStatus status;

    // Get input/output
    OfxMeshInputHandle input, output;
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainInput, &input, NULL);
    meshEffectSuite->inputGetHandle(instance, kOfxMeshMainOutput, &output, NULL);

    // Get parameters
    OfxParamSetHandle parameters;
    OfxParamHandle param;
    double tx, ty, rx, ry, sx, sy;
    MFX_CHECK(meshEffectSuite->getParamSet(instance, &parameters));
    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Translation", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &tx, &ty));
    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Rotation", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &rx, &ry));
    MFX_CHECK(parameterSuite->paramGetHandle(parameters, "Scale", &param, NULL));
    MFX_CHECK(parameterSuite->paramGetValue(param, &sx, &sy));

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

    // Get Corner color
    OfxPropertySetHandle vcolor_attrib, uv_attrib;
    status = meshEffectSuite->meshGetAttribute(input_mesh, kOfxMeshAttribCorner, "color0", &vcolor_attrib);

    printf("Look for color0...\n");
    char *vcolor_data = NULL;
    if (kOfxStatOK == status) {
      printf("found!\n");
      propertySuite->propGetPointer(vcolor_attrib, kOfxMeshAttribPropData, 0, (void**)&vcolor_data);
      meshEffectSuite->attributeDefine(output_mesh,
                                       kOfxMeshAttribCorner,
                                       "uv0",
                                       2,
                                       kOfxMeshAttribTypeFloat,
                                       kOfxMeshAttribSemanticTextureCoordinate,
                                       &uv_attrib);
    }
    else {
      // DEBUG
      meshEffectSuite->attributeDefine(output_mesh,
                                       kOfxMeshAttribCorner,
                                       "uv0",
                                       2,
                                       kOfxMeshAttribTypeFloat,
                                       kOfxMeshAttribSemanticTextureCoordinate,
                                       &uv_attrib);
    }

    // Allocate output mesh
    int output_point_count = input_point_count;
    int output_corner_count = input_corner_count;
    int output_face_count = input_face_count;

    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropPointCount, 0, output_point_count);
    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropCornerCount, 0, output_corner_count);
    propertySuite->propSetInt(output_mesh_prop, kOfxMeshPropFaceCount, 0, output_face_count);

    meshEffectSuite->meshAlloc(output_mesh);

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
      char *uv_data;
      int uv_stride, vcolor_stride;
      propertySuite->propGetInt(uv_attrib, kOfxMeshAttribPropComponentCount, 0, &uv_stride);
      propertySuite->propGetInt(vcolor_attrib, kOfxMeshAttribPropComponentCount, 0, &vcolor_stride);
      propertySuite->propGetPointer(uv_attrib, kOfxMeshAttribPropData, 0, (void**)&uv_data);
      for (int i = 0; i < input_corner_count; ++i) {
        float *vcolor = (float *)(vcolor_data + vcolor_stride * i);
        float *uv = (float *)(uv_data + uv_stride * i);
        uv[0] = vcolor[0];
        uv[1] = vcolor[1];
      }
    }
    else {
      // DEBUG
      char *uv_data;
      int uv_stride;
      propertySuite->propGetInt(uv_attrib, kOfxMeshAttribPropComponentCount, 0, &uv_stride);
      propertySuite->propGetPointer(uv_attrib, kOfxMeshAttribPropData, 0, (void**)&uv_data);
      for (int i = 0; i < input_corner_count; ++i) {
        int corner = *(int *)input_cornerpoint.data[i * input_cornerpoint.stride];
        float *P = (float *)input_pos.data[corner * input_pos.stride];
        float *uv = (float *)(uv_data + uv_stride * i);
        uv[0] = P[0];
        uv[1] = P[1];
      }
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
