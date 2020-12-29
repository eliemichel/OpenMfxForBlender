/*
 * Copyright 2019-2020 Elie Michel
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

#include "meshEffectSuite.h"
#include "propertySuite.h"
#include "mesheffect.h"

#include <cstring>
#include <cstdio>

 // CONVERSION UTILS

static AttributeAttachment mfxToInternalAttribAttachment(const char *attachment)
{
  if (0 == strcmp(attachment, kOfxMeshAttribPoint)) {
    return ATTR_ATTACH_POINT;
  }
  else if (0 == strcmp(attachment, kOfxMeshAttribVertex)) {
    return ATTR_ATTACH_VERTEX;
  }
  else if (0 == strcmp(attachment, kOfxMeshAttribFace)) {
    return ATTR_ATTACH_FACE;
  }
  else if (0 == strcmp(attachment, kOfxMeshAttribMesh)) {
    return ATTR_ATTACH_MESH;
  }
  else {
    return ATTR_ATTACH_INVALID;
  }
}

// // Mesh Effect Suite Entry Points

const OfxMeshEffectSuiteV1 gMeshEffectSuiteV1 = {
    /* getPropertySet */ getPropertySet,
    /* getParamSet */ getParamSet,
    /* inputDefine */ inputDefine,
    /* inputGetHandle */ inputGetHandle,
    /* inputGetPropertySet */ inputGetPropertySet,
    /* inputRequestAttribute */ inputRequestAttribute,
    /* inputGetMesh */ inputGetMesh,
    /* inputReleaseMesh */ inputReleaseMesh,
    /* attributeDefine */ attributeDefine,
    /* meshGetAttribute */ meshGetAttribute,
    /* meshGetPropertySet */ meshGetPropertySet,
    /* meshAlloc */ meshAlloc,
    /* abort */ ofxAbort};

OfxStatus getPropertySet(OfxMeshEffectHandle meshEffect, OfxPropertySetHandle *propHandle)
{
  *propHandle = &meshEffect->properties;
  return kOfxStatOK;
}

OfxStatus getParamSet(OfxMeshEffectHandle meshEffect, OfxParamSetHandle *paramSet)
{
  *paramSet = &meshEffect->parameters;
  return kOfxStatOK;
}

OfxStatus inputDefine(OfxMeshEffectHandle meshEffect,
                      const char *name,
                      OfxMeshInputHandle *input,
                      OfxPropertySetHandle *propertySet)
{
  printf("Defining input '%s' on OfxMeshEffectHandle %p\n", name, meshEffect);
  int i = meshEffect->inputs.ensure(name);
  meshEffect->inputs.inputs[i]->host = meshEffect->host;
  propSetPointer(
      &meshEffect->inputs.inputs[i]->mesh.properties, kOfxMeshPropInternalData, 0, NULL);
  if (NULL != input) {
    *input = meshEffect->inputs.inputs[i];
  }
  if (NULL != propertySet) {
    *propertySet = &(meshEffect->inputs.inputs[i]->properties);
  }
  return kOfxStatOK;
}

OfxStatus inputGetHandle(OfxMeshEffectHandle meshEffect,
                         const char *name,
                         OfxMeshInputHandle *input,
                         OfxPropertySetHandle *propertySet)
{
  int i = meshEffect->inputs.find(name);
  if (-1 == i) {
    return kOfxStatErrUnknown;  // bad name
  }
  *input = meshEffect->inputs.inputs[i];
  if (NULL != propertySet) {
    *propertySet = &(meshEffect->inputs.inputs[i]->properties);
  }
  return kOfxStatOK;
}

OfxStatus inputGetPropertySet(OfxMeshInputHandle input, OfxPropertySetHandle *propHandle)
{
  *propHandle = &input->properties;
  return kOfxStatOK;
}

OfxStatus inputRequestAttribute(OfxMeshInputHandle input,
    const char* attachment,
    const char* name,
    int componentCount,
    const char* type,
    const char* semantic,
    int mandatory)
{
  if (componentCount < 1 || componentCount > 4) {
    return kOfxStatErrValue;
  }
  if (0 != strcmp(type, kOfxMeshAttribTypeInt) && 0 != strcmp(type, kOfxMeshAttribTypeFloat) &&
      0 != strcmp(type, kOfxMeshAttribTypeUByte)) {
    return kOfxStatErrValue;
  }

  if (0 != strcmp(semantic, kOfxMeshAttribSemanticTextureCoordinate) &&
      0 != strcmp(semantic, kOfxMeshAttribSemanticNormal) &&
      0 != strcmp(semantic, kOfxMeshAttribSemanticColor) &&
      0 != strcmp(semantic, kOfxMeshAttribSemanticWeight)) {
    return kOfxStatErrValue;
  }

  AttributeAttachment intAttachment = mfxToInternalAttribAttachment(attachment);
  if (intAttachment == ATTR_ATTACH_INVALID) {
    return kOfxStatErrBadIndex;
  }

  int i = input->requested_attributes.ensure(intAttachment, name);

  OfxPropertySetStruct *attributeProperties =
      &input->requested_attributes.attributes[i]->properties;
  propSetInt(attributeProperties, kOfxMeshAttribPropComponentCount, 0, componentCount);
  propSetString(attributeProperties, kOfxMeshAttribPropType, 0, type);
  propSetString(attributeProperties, kOfxMeshAttribPropSemantic, 0, semantic);
  propSetInt(attributeProperties, kMeshAttribRequestPropMandatory, 0, mandatory);

  return kOfxStatOK;
}

OfxStatus inputGetMesh(OfxMeshInputHandle input,
                       OfxTime time,
                       OfxMeshHandle *meshHandle,
                       OfxPropertySetHandle *propertySet)
{
  (void)time;
  OfxMeshHandle inputMeshHandle = &input->mesh;
  OfxPropertySetHandle inputMeshProperties = &input->mesh.properties;
  propSetPointer(inputMeshProperties, kOfxMeshPropHostHandle, 0, (void *)input->host);
  propSetInt(inputMeshProperties, kOfxMeshPropPointCount, 0, 0);
  propSetInt(inputMeshProperties, kOfxMeshPropVertexCount, 0, 0);
  propSetInt(inputMeshProperties, kOfxMeshPropFaceCount, 0, 0);
  propSetInt(inputMeshProperties, kOfxMeshPropAttributeCount, 0, 0);

  // Default attributes
  attributeDefine(inputMeshHandle,
                  kOfxMeshAttribPoint,
                  kOfxMeshAttribPointPosition,
                  3,
                  kOfxMeshAttribTypeFloat,
                  NULL,
                  NULL);
  attributeDefine(inputMeshHandle,
                  kOfxMeshAttribVertex,
                  kOfxMeshAttribVertexPoint,
                  1,
                  kOfxMeshAttribTypeInt,
                  NULL,
                  NULL);
  attributeDefine(inputMeshHandle,
                  kOfxMeshAttribFace,
                  kOfxMeshAttribFaceCounts,
                  1,
                  kOfxMeshAttribTypeInt,
                  NULL,
                  NULL);

  // Call internal callback before actually getting data
  OfxHost *host = input->host;
  BeforeMeshGetCbFunc beforeMeshGetCb;
  if (NULL != host) {
    propGetPointer(host->host, kOfxHostPropBeforeMeshGetCb, 0, (void **)&beforeMeshGetCb);
    if (NULL != beforeMeshGetCb) {
      OfxStatus status = beforeMeshGetCb(host, inputMeshHandle);
      if (kOfxStatOK != status) {
        *meshHandle = NULL;
        return status;
      }
    }
  }

  *meshHandle = inputMeshHandle;
  if (NULL != propertySet) {
    *propertySet = inputMeshProperties;
  }

  return kOfxStatOK;
}

OfxStatus inputReleaseMesh(OfxMeshHandle meshHandle)
{
  // Call internal callback before actually releasing data
  OfxHost *host;
  BeforeMeshReleaseCbFunc beforeMeshReleaseCb;
  propGetPointer(&meshHandle->properties, kOfxMeshPropHostHandle, 0, (void **)&host);
  if (NULL != host) {
    propGetPointer(host->host, kOfxHostPropBeforeMeshReleaseCb, 0, (void **)&beforeMeshReleaseCb);
    if (NULL != beforeMeshReleaseCb) {
      beforeMeshReleaseCb(host, meshHandle);
    }
  }

  // Free owned attributes
  void *data;
  int is_owner;
  for (int i = 0; i < meshHandle->attributes.num_attributes; ++i) {
    OfxAttributeStruct *attribute = meshHandle->attributes.attributes[i];
    propGetPointer(&attribute->properties, kOfxMeshAttribPropData, 0, &data);
    propGetInt(&attribute->properties, kOfxMeshAttribPropIsOwner, 0, &is_owner);
    if (is_owner && NULL != data) {
      delete[] static_cast<char*>(data);  // delete on void* is undefined behaviour
    }
    propSetPointer(&attribute->properties, kOfxMeshAttribPropData, 0, NULL);
    propSetInt(&attribute->properties, kOfxMeshAttribPropIsOwner, 0, 0);
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
                          const char *semantic,
                          OfxPropertySetHandle *attributeHandle)
{
  if (componentCount < 1 || componentCount > 4) {
    return kOfxStatErrValue;
  }
  if (0 != strcmp(type, kOfxMeshAttribTypeInt) &&
      0 != strcmp(type, kOfxMeshAttribTypeFloat) &&
      0 != strcmp(type, kOfxMeshAttribTypeUByte)) {
    return kOfxStatErrValue;
  }

  if (NULL != semantic) {
    if (0 != strcmp(semantic, kOfxMeshAttribSemanticTextureCoordinate) &&
        0 != strcmp(semantic, kOfxMeshAttribSemanticNormal) &&
        0 != strcmp(semantic, kOfxMeshAttribSemanticColor) &&
        0 != strcmp(semantic, kOfxMeshAttribSemanticWeight)) {
      return kOfxStatErrValue;
    }
  }

  AttributeAttachment intAttachment = mfxToInternalAttribAttachment(attachment);
  if (intAttachment == ATTR_ATTACH_INVALID) {
    return kOfxStatErrBadIndex;
  }

  int i = meshHandle->attributes.ensure(intAttachment, name);

  OfxPropertySetStruct *attributeProperties = &meshHandle->attributes.attributes[i]->properties;
  propSetPointer(attributeProperties, kOfxMeshAttribPropData, 0, NULL);
  propSetInt(attributeProperties, kOfxMeshAttribPropComponentCount, 0, componentCount);
  propSetString(attributeProperties, kOfxMeshAttribPropType, 0, type);
  propSetString(attributeProperties, kOfxMeshAttribPropSemantic, 0, semantic);
  propSetInt(attributeProperties, kOfxMeshAttribPropIsOwner, 0, 1);

  // Keep attribute count up-to-date
  propSetInt(
      &meshHandle->properties, kOfxMeshPropAttributeCount, 0, meshHandle->attributes.num_attributes);

  if (attributeHandle) {
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

  int i = meshHandle->attributes.find(intAttachment, name);

  if (i == -1) {
    return kOfxStatErrBadIndex;
  }
  else {
    *attributeHandle = &meshHandle->attributes.attributes[i]->properties;
    return kOfxStatOK;
  }
}

OfxStatus meshGetPropertySet(OfxMeshHandle mesh, OfxPropertySetHandle *propHandle)
{
  *propHandle = &mesh->properties;
  return kOfxStatOK;
}

OfxStatus meshAlloc(OfxMeshHandle meshHandle)
{
  OfxStatus status;

  // Get counts

  int elementCount[4];  // point, vertex, face, mesh

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

    int is_owner;
    status = propGetInt(&attribute->properties, kOfxMeshAttribPropIsOwner, 0, &is_owner);
    if (kOfxStatOK != status) {
      return status;
    }

    // Don't allocate non-own attributes (i.e. attributes which are just proxy to externally
    // allocated buffers.
    if (!is_owner) {
      continue;
    }

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
    if (0 == strcmp(type, kOfxMeshAttribTypeUByte)) {
      byteSize = sizeof(unsigned char);
    }
    else if (0 == strcmp(type, kOfxMeshAttribTypeInt)) {
      byteSize = sizeof(int);
    }
    else if (0 == strcmp(type, kOfxMeshAttribTypeFloat)) {
      byteSize = sizeof(float);
    }
    else {
      return kOfxStatErrBadHandle;
    }

    void *data = new char[byteSize * count * elementCount[attribute->attachment]];
    if (NULL == data) {
      return kOfxStatErrMemory;
    }

    status = propSetPointer(&attribute->properties, kOfxMeshAttribPropData, 0, data);
    if (kOfxStatOK != status) {
      return status;
    }

    status = propSetInt(
        &attribute->properties, kOfxMeshAttribPropStride, 0, (int)(byteSize * count));
    if (kOfxStatOK != status) {
      return status;
    }

    status = propSetInt(&attribute->properties, kOfxMeshAttribPropIsOwner, 0, 1);
    if (kOfxStatOK != status) {
      return status;
    }
  }

  return kOfxStatOK;
}

int ofxAbort(OfxMeshEffectHandle meshEffect)
{
  (void)meshEffect;
  return 0;
}
