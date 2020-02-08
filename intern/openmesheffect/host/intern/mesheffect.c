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
#include <string.h>

#include "util/memory_util.h"

#include "mesheffect.h"

 // CONVERSION UTILS

static AttributeAttachment mfxToInternalAttribAttachment(const char *attachment)
{
  if (0 == strcmp(attachment, kOfxMeshAttribPoint)) {
    return ATTR_ATTACH_POINT;
  } else if (0 == strcmp(attachment, kOfxMeshAttribVertex)) {
    return ATTR_ATTACH_VERTEX;
  } else if (0 == strcmp(attachment, kOfxMeshAttribFace)) {
    return ATTR_ATTACH_FACE;
  } else if (0 == strcmp(attachment, kOfxMeshAttribMesh)) {
    return ATTR_ATTACH_MESH;
  } else {
    return ATTR_ATTACH_INVALID;
  }
}

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

const OfxMeshEffectSuiteV1 gMeshEffectSuiteV1 = {
    /* getPropertySet */      getPropertySet,
    /* getParamSet */         getParamSet,
    /* inputDefine */         inputDefine,
    /* inputGetHandle */      inputGetHandle,
    /* inputGetPropertySet */ inputGetPropertySet,
    /* inputGetMesh */        inputGetMesh,
    /* inputReleaseMesh */    inputReleaseMesh,
    /* attributeDefine */     attributeDefine,
    /* meshGetAttribute */    meshGetAttribute,
    /* meshGetPropertySet */  meshGetPropertySet,
    /* meshAlloc */           meshAlloc,
    /* abort */               ofxAbort
};

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
  meshEffect->inputs.inputs[i]->host = meshEffect->host;
  propSetPointer(&meshEffect->inputs.inputs[i]->mesh.properties, kOfxMeshPropInternalData, 0, NULL);
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
  if (NULL != propertySet) {
    *propertySet = &(meshEffect->inputs.inputs[i]->properties);
  }
  return kOfxStatOK;
}

OfxStatus inputGetPropertySet(OfxMeshInputHandle input,
                              OfxPropertySetHandle *propHandle) {
  *propHandle = &input->properties;
  return kOfxStatOK;
}

OfxStatus inputGetMesh(OfxMeshInputHandle input,
                       OfxTime time,
                       OfxMeshHandle *meshHandle,
                       OfxPropertySetHandle *propertySet) {
  (void)time;
  OfxMeshHandle inputMeshHandle = &input->mesh;
  OfxPropertySetHandle inputMeshProperties = &input->mesh.properties;
  propSetPointer(inputMeshProperties, kOfxMeshPropHostHandle, 0, (void*)input->host);
  propSetInt(inputMeshProperties, kOfxMeshPropPointCount, 0, 0);
  propSetInt(inputMeshProperties, kOfxMeshPropVertexCount, 0, 0);
  propSetInt(inputMeshProperties, kOfxMeshPropFaceCount, 0, 0);

  // Default attributes
  attributeDefine(inputMeshHandle, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, 3, kOfxMeshAttribTypeFloat, NULL);
  attributeDefine(inputMeshHandle, kOfxMeshAttribVertex, kOfxMeshAttribVertexPoint, 1, kOfxMeshAttribTypeInt, NULL);
  attributeDefine(inputMeshHandle, kOfxMeshAttribFace, kOfxMeshAttribFaceCounts, 1, kOfxMeshAttribTypeInt, NULL);
  
  // Call internal callback before actually getting data
  OfxHost *host = input->host;
  BeforeMeshGetCbFunc beforeMeshGetCb;
  if (NULL != host) {
    propGetPointer(host->host, kOfxHostPropBeforeMeshGetCb, 0, (void**)&beforeMeshGetCb);
    if (NULL != beforeMeshGetCb) {
      beforeMeshGetCb(host, inputMeshHandle);
    }
  }

  *meshHandle = inputMeshHandle;
  if (NULL != propertySet) {
    *propertySet = inputMeshProperties;
  }

  return kOfxStatOK;
}

OfxStatus inputReleaseMesh(OfxMeshHandle meshHandle) {
  // Call internal callback before actually releasing data
  OfxHost *host;
  BeforeMeshReleaseCbFunc beforeMeshReleaseCb;
  propGetPointer(&meshHandle->properties, kOfxMeshPropHostHandle, 0, (void**)&host);
  if (NULL != host) {
    propGetPointer(host->host, kOfxHostPropBeforeMeshReleaseCb, 0, (void**)&beforeMeshReleaseCb);
    if (NULL != beforeMeshReleaseCb) {
      beforeMeshReleaseCb(host, meshHandle);
    }
  }

  // Free attributes
  void *data;
  for (int i = 0; i < meshHandle->attributes.num_attributes; ++i) {
    OfxAttributeStruct *attribute = meshHandle->attributes.attributes[i];
    propGetPointer(&attribute->properties, kOfxMeshAttribPropData, 0, &data);
    if (NULL != data) {
      free_array(data);
    }
    propSetPointer(&attribute->properties, kOfxMeshAttribPropData, 0, NULL);
  }

  propSetInt(&meshHandle->properties, kOfxMeshPropPointCount, 0, 0);
  propSetInt(&meshHandle->properties, kOfxMeshPropVertexCount, 0, 0);
  propSetInt(&meshHandle->properties, kOfxMeshPropFaceCount, 0, 0);

  return kOfxStatOK;
}

OfxStatus attributeDefine(OfxMeshHandle meshHandle,
                          const char *attachment,
                          const char *name,
                          int componentCount,
                          const char *type,
                          OfxPropertySetHandle *attributeHandle)
{
  if (componentCount < 1 || componentCount > 4) {
    return kOfxStatErrValue;
  }
  if (type != kOfxMeshAttribTypeInt && type != kOfxMeshAttribTypeFloat) {
    return kOfxStatErrValue;
  }

  AttributeAttachment intAttachment = mfxToInternalAttribAttachment(attachment);
  if (intAttachment == ATTR_ATTACH_INVALID) {
    return kOfxStatErrBadIndex;
  }

  int i = ensure_attribute(&meshHandle->attributes, intAttachment, name);

  OfxPropertySetStruct *attributeProperties = &meshHandle->attributes.attributes[i]->properties;
  propSetPointer(attributeProperties, kOfxMeshAttribPropData, 0, NULL);
  propSetInt(attributeProperties, kOfxMeshAttribPropComponentCount, 0, componentCount);
  propSetString(attributeProperties, kOfxMeshAttribPropType, 0, type);

  if (attributeHandle){
    *attributeHandle = attributeProperties;
  }
  return kOfxStatOK;
}

OfxStatus meshGetAttribute(OfxMeshHandle meshHandle,
                           const char *attachment,
                           const char *name,
                           OfxPropertySetHandle *attributeHandle)
{
  AttributeAttachment intAttachment = mfxToInternalAttribAttachment(attachment);
  if (intAttachment == ATTR_ATTACH_INVALID) {
    return kOfxStatErrBadIndex;
  }

  int i = find_attribute(&meshHandle->attributes, intAttachment, name);

  if (i == -1) {
    return kOfxStatErrBadIndex;
  } else {
    *attributeHandle = &meshHandle->attributes.attributes[i]->properties;
    return kOfxStatOK;
  }
}

OfxStatus meshGetPropertySet(OfxMeshHandle mesh,
                             OfxPropertySetHandle *propHandle)
{
  *propHandle = &mesh->properties;
  return kOfxStatOK;
}

OfxStatus meshAlloc(OfxMeshHandle meshHandle) {
  OfxStatus status;

  // Get counts

  int elementCount[4]; // point, vertex, face, mesh

  status = propGetInt(&meshHandle->properties, kOfxMeshPropPointCount, 0, &elementCount[0]);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propGetInt(&meshHandle->properties, kOfxMeshPropVertexCount, 0, &elementCount[1]);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propGetInt(&meshHandle->properties, kOfxMeshPropFaceCount, 0, &elementCount[2]);
  if (kOfxStatOK != status) {
    return status;
  }
  elementCount[3] = 1;


  // Allocate memory attributes

  for (int i = 0; i < meshHandle->attributes.num_attributes; ++i) {
    OfxAttributeStruct *attribute = meshHandle->attributes.attributes[i];

    int count;
    status = propGetInt(&attribute->properties, kOfxMeshAttribPropComponentCount, 0, &count);
    if (kOfxStatOK != status) {
      return status;
    }

    char *type;
    status = propGetString(&attribute->properties, kOfxMeshAttribPropType, 0, &type);
    if (kOfxStatOK != status) {
      return status;
    }

    size_t byteSize = 0;
    if (type == kOfxMeshAttribTypeInt) {
      byteSize = sizeof(int);
    } else if (type == kOfxMeshAttribTypeFloat) {
      byteSize = sizeof(float);
    } else {
      return kOfxStatErrBadHandle;
    }

    void *data = (void*)malloc_array(byteSize * count, elementCount[attribute->attachment], "attribute data");
    if (NULL == data) {
      return kOfxStatErrMemory;
    }

    status = propSetPointer(&attribute->properties, kOfxMeshAttribPropData, 0, data);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  
  return kOfxStatOK;
}

int ofxAbort(OfxMeshEffectHandle meshEffect) {
  (void)meshEffect;
  return 0;
}

