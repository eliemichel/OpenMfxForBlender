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

#include <stdio.h>

#include "util/memory_util.h"

#include "mesheffect.h"

// OFX MESH EFFECT SUITE

// // Mesh Effect

void init_mesh_effect(OfxMeshEffectHandle meshEffectHandle) {
  meshEffectHandle->inputs.host = meshEffectHandle->host;
  init_input_set(&meshEffectHandle->inputs);
  init_properties(&meshEffectHandle->properties);
  init_parameter_set(&meshEffectHandle->parameters);
  meshEffectHandle->parameters.effect_properties = &meshEffectHandle->properties;
  meshEffectHandle->properties.context = PROP_CTX_MESH_EFFECT;
}

void free_mesh_effect(OfxMeshEffectHandle meshEffectHandle) {
  free_input_set(&meshEffectHandle->inputs);
  free_properties(&meshEffectHandle->properties);
  free_parameter_set(&meshEffectHandle->parameters);
}

void deep_copy_mesh_effect(OfxMeshEffectStruct *destination, const OfxMeshEffectStruct *source) {
  deep_copy_input_set(&destination->inputs, &source->inputs);
  deep_copy_property_set(&destination->properties, &source->properties);
  deep_copy_parameter_set(&destination->parameters, &source->parameters);
  destination->parameters.effect_properties = &destination->properties;
  destination->host = source->host; // not deep copied, as this is a weak pointer
}

// // Mesh Effect Suite Entry Points

OfxStatus getPropertySet(OfxMeshEffectHandle meshEffect,
                         OfxPropertySetHandle *propHandle) {
  *propHandle = &meshEffect->properties;
  return kOfxStatOK;
}

OfxStatus getParamSet(OfxMeshEffectHandle meshEffect,
                      OfxParamSetHandle *paramSet) {
  *paramSet = &meshEffect->parameters;
  return kOfxStatOK;
}

OfxStatus inputDefine(OfxMeshEffectHandle meshEffect,
                      const char *name,
                      OfxPropertySetHandle *propertySet) {
  printf("Defining input '%s' on OfxMeshEffectHandle %p\n", name, meshEffect);
  int i = ensure_input(&meshEffect->inputs, name);
  *propertySet = &(meshEffect->inputs.inputs[i]->properties);
  return kOfxStatOK;
}

OfxStatus inputGetHandle(OfxMeshEffectHandle meshEffect,
                         const char *name,
                         OfxMeshInputHandle *input,
                         OfxPropertySetHandle *propertySet) {
  int i = find_input(&meshEffect->inputs, name);
  if (-1 == i) {
    return kOfxStatErrUnknown; // bad name
  }
  *input = meshEffect->inputs.inputs[i];
  *propertySet = &(meshEffect->inputs.inputs[i]->properties);
  return kOfxStatOK;
}

OfxStatus inputGetPropertySet(OfxMeshInputHandle input,
                              OfxPropertySetHandle *propHandle) {
  *propHandle = &input->properties;
  return kOfxStatOK;
}

OfxStatus inputGetMesh(OfxMeshInputHandle input,
                       OfxTime time,
                       OfxPropertySetHandle *meshHandle) {
  (void)time;
  OfxPropertySetHandle inputMeshHandle = &input->mesh;
  propSetPointer(inputMeshHandle, kOfxMeshPropInternalData, 0, NULL); // TODO: get this from input
  propSetPointer(inputMeshHandle, kOfxMeshPropHostHandle, 0, (void*)input->host);
  if (NULL == NULL) { // callback to get data from internal pointer
    propSetInt(inputMeshHandle, kOfxMeshPropPointCount, 0, 0);
    propSetInt(inputMeshHandle, kOfxMeshPropVertexCount, 0, 0);
    propSetInt(inputMeshHandle, kOfxMeshPropFaceCount, 0, 0);
    propSetPointer(inputMeshHandle, kOfxMeshPropPointData, 0, NULL);
    propSetPointer(inputMeshHandle, kOfxMeshPropVertexData, 0, NULL);
    propSetPointer(inputMeshHandle, kOfxMeshPropFaceData, 0, NULL);
  } else {
    // TODO
  }

  *meshHandle = inputMeshHandle;

  return kOfxStatOK;
}

OfxStatus inputReleaseMesh(OfxPropertySetHandle meshHandle) {
  float *pointData;
  int *vertexData, *faceData;

  // Call internal callback before actually releasing data
  OfxHost *host;
  BeforeMeshReleaseCbFunc beforeMeshReleaseCb;
  propGetPointer(meshHandle, kOfxMeshPropHostHandle, 0, (void**)&host);
  if (NULL != host) {
    propGetPointer(host->host, kOfxHostPropBeforeMeshReleaseCb, 0, (void**)&beforeMeshReleaseCb);
    if (NULL != beforeMeshReleaseCb) {
      beforeMeshReleaseCb(host, meshHandle);
    }
  }

  propGetPointer(meshHandle, kOfxMeshPropPointData, 0, (void**)&pointData);
  propGetPointer(meshHandle, kOfxMeshPropVertexData, 0, (void**)&vertexData);
  propGetPointer(meshHandle, kOfxMeshPropFaceData, 0, (void**)&faceData);
  
  if (NULL != pointData) {
    free_array(pointData);
  }
  if (NULL != vertexData) {
    free_array(vertexData);
  }
  if (NULL != faceData) {
    free_array(faceData);
  }

  propSetInt(meshHandle, kOfxMeshPropPointCount, 0, 0);
  propSetInt(meshHandle, kOfxMeshPropVertexCount, 0, 0);
  propSetInt(meshHandle, kOfxMeshPropFaceCount, 0, 0);
  propSetPointer(meshHandle, kOfxMeshPropPointData, 0, NULL);
  propSetPointer(meshHandle, kOfxMeshPropVertexData, 0, NULL);
  propSetPointer(meshHandle, kOfxMeshPropFaceData, 0, NULL);

  return kOfxStatOK;
}

OfxStatus meshAlloc(OfxPropertySetHandle meshHandle,
                    int pointCount,
                    int vertexCount,
                    int faceCount) {
  OfxStatus status;

  float *pointData = malloc_array(sizeof(float) * 3, pointCount, "point data");
  if (NULL == pointData) {
    return kOfxStatErrMemory;
  }

  int *vertexData = malloc_array(sizeof(int), vertexCount, "vertex data");
  if (NULL == vertexData) {
    free_array(pointData);
    return kOfxStatErrMemory;
  }

  int *faceData = malloc_array(sizeof(int) * 3, faceCount, "face data");
  if (NULL == faceData) {
    free_array(pointData);
    free_array(vertexData);
    return kOfxStatErrMemory;
  }

  status = propSetInt(meshHandle, kOfxMeshPropPointCount, 0, pointCount);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetInt(meshHandle, kOfxMeshPropVertexCount, 0, vertexCount);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetInt(meshHandle, kOfxMeshPropFaceCount, 0, faceCount);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetPointer(meshHandle, kOfxMeshPropPointData, 0, pointData);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetPointer(meshHandle, kOfxMeshPropVertexData, 0, vertexData);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetPointer(meshHandle, kOfxMeshPropFaceData, 0, faceData);
  if (kOfxStatOK != status) {
    return status;
  }
  return kOfxStatOK;
}

int ofxAbort(OfxMeshEffectHandle meshEffect) {
  (void)meshEffect;
  return 0;
}

