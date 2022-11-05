#include "meshEffectSuite.h"

#include <OpenMfx/Sdk/C/Common>

#include <stdio.h>
#include <string.h>

OfxStatus defaultAttributesDefine(OfxMeshHandle mesh);

OfxStatus getParamSet(OfxMeshEffectHandle meshEffect,
                      OfxParamSetHandle *paramSet)
{
  printf("[host] getParamSet(meshEffect %p)\n", meshEffect);
  *paramSet = &meshEffect->parameters;
  return kOfxStatOK;
}

OfxStatus inputDefine(OfxMeshEffectHandle meshEffect,
                      const char *name,
                      OfxMeshInputHandle *inputHandle,
                      OfxPropertySetHandle *propertySet)
{
  printf("[host] inputDefine(meshEffect %p, %s)\n", meshEffect, name);
  if (meshEffect->is_valid != 1) {
    return kOfxStatErrBadHandle;
  }

  int input_index = 0;
  while (meshEffect->inputs[input_index].is_valid) ++input_index;
  if (input_index == 16) return kOfxStatErrMemory;
  OfxMeshInputHandle new_input = &meshEffect->inputs[input_index];

  meshInputInit(new_input);
  strncpy(new_input->name, name, 256);

  *inputHandle = new_input;
  *propertySet = (OfxPropertySetHandle)&new_input->properties;
  return kOfxStatOK;
}

OfxStatus inputGetHandle(OfxMeshEffectHandle meshEffect,
                         const char *name,
                         OfxMeshInputHandle *inputHandle,
                         OfxPropertySetHandle *propertySet)
{
  printf("[host] inputGetHandle(meshEffect %p, %s)\n", meshEffect, name);
  if (meshEffect->is_valid != 1) {
    return kOfxStatErrBadHandle;
  }

  for (int i = 0 ; i < 16 && meshEffect->inputs[i].is_valid ; ++i) {
    OfxMeshInputHandle input = &meshEffect->inputs[i];
    if (0 == strncmp(name, input->name, 256)) {
      *inputHandle = input;
      if (NULL != propertySet) {
        *propertySet = (OfxPropertySetHandle)&input->properties;
      }
      return kOfxStatOK;
    }
  }

  return kOfxStatErrBadIndex;
}

OfxStatus inputGetPropertySet(OfxMeshInputHandle input,
                              OfxPropertySetHandle *propertySet)
{
  printf("[host] inputGetPropertySet(input %p)\n", input);
  if (input->is_valid != 1) {
    return kOfxStatErrBadHandle;
  }

  *propertySet = (OfxPropertySetHandle)&input->properties;
  return kOfxStatOK;
}

OfxStatus inputGetMesh(OfxMeshInputHandle input,
                       OfxTime time,
                       OfxMeshHandle *meshHandle,
                       OfxPropertySetHandle *propertySet)
{
  printf("[host] inputGetMesh(input %p, %f)\n", input, time);
  if (input->is_valid != 1) {
    return kOfxStatErrBadHandle;
  }
  if (time != 0.0) {
    return kOfxStatErrMissingHostFeature;
  }

  if (0 == strncmp(input->name, kOfxMeshMainOutput, 256)) {
    MFX_ENSURE(defaultAttributesDefine(&input->mesh));
  }

  *meshHandle = &input->mesh;
  if (NULL != propertySet) {
    *propertySet = (OfxPropertySetHandle)&input->mesh.properties;
  }

  return kOfxStatOK;
}

OfxStatus inputReleaseMesh(OfxMeshHandle meshHandle)
{
  printf("[host] inputReleaseMesh(mesh %p)\n", meshHandle);
  return kOfxStatOK;
}

OfxStatus meshGetAttributeByIndex(OfxMeshHandle meshHandle,
                                  int index,
                                  OfxPropertySetHandle *attributeHandle)
{
  printf("[host] meshGetAttributeByIndex(mesh %p, %d)\n", meshHandle, index);
  if (index < 0 || index >= 32) return kOfxStatErrBadIndex;
  OfxMeshAttributePropertySet* attribute = &meshHandle->attributes[index];
  if (!attribute->is_valid) return kOfxStatErrBadIndex;

  *attributeHandle = (OfxPropertySetHandle)attribute;
  return kOfxStatOK;
}

OfxStatus meshGetAttribute(OfxMeshHandle meshHandle,
                           const char *attachment,
                           const char *name,
                           OfxPropertySetHandle *attributeHandle)
{
  printf("[host] meshGetAttribute(mesh %p, %s, %s)\n", meshHandle, attachment, name);
  for (int i = 0 ; i < 32 && meshHandle->attributes[i].is_valid ; ++i) {
    OfxMeshAttributePropertySet* attribute = &meshHandle->attributes[i];
    if (0 == strncmp(attachment, attribute->attachment, 64) &&
        0 == strncmp(name, attribute->name, 64))
    {
      *attributeHandle = (OfxPropertySetHandle)attribute;
      return kOfxStatOK;
    }
  }

  return kOfxStatErrBadIndex;
}

OfxStatus attributeDefine(OfxMeshHandle meshHandle,
                          const char *attachment,
                          const char *name,
                          int componentCount,
                          const char *type,
                          const char *semantic,
                          OfxPropertySetHandle *attributeHandle)
{
  printf("[host] attributeDefine(mesh %p, %s, %s, %d, %s, %s)\n", meshHandle, attachment, name, componentCount, type, semantic);

  // Check for duplicates
  OfxPropertySetHandle existingAttribute = NULL;
  if (kOfxStatErrBadIndex != meshGetAttribute(meshHandle, attachment, name, &existingAttribute)) {
    return kOfxStatErrExists;
  }

  int attribute_index = 0;
  while (meshHandle->attributes[attribute_index].is_valid) ++attribute_index;
  if (attribute_index == 32) return kOfxStatErrMemory;
  OfxMeshAttributePropertySet* attribute = &meshHandle->attributes[attribute_index];

  attribute->is_valid = 1;
  strncpy(attribute->attachment, attachment, 64);
  strncpy(attribute->name, name, 64);
  attribute->component_count = componentCount;
  strncpy(attribute->type, type, 64);
  strncpy(attribute->semantic, semantic, 64);

  *attributeHandle = (OfxPropertySetHandle)attribute;
  return kOfxStatOK;
}

OfxStatus meshGetPropertySet(OfxMeshHandle mesh,
                             OfxPropertySetHandle *propertySet)
{
  printf("[host] meshGetPropertySet(mesh %p)\n", mesh);
  if (NULL == mesh) {
    return kOfxStatErrBadHandle;
  }

  *propertySet = (OfxPropertySetHandle)&mesh->properties;
  return kOfxStatOK;
}

OfxStatus meshAlloc(OfxMeshHandle meshHandle)
{
  printf("[host] meshAlloc(mesh %p)\n", meshHandle);
  OfxMeshPropertySet *props = &meshHandle->properties;
  for (int i = 0 ; i < 32 && meshHandle->attributes[i].is_valid ; ++i) {
    MFX_ENSURE(attributeAlloc(&meshHandle->attributes[i], props));
  }
  return kOfxStatOK;
}

OfxStatus defaultAttributesDefine(OfxMeshHandle mesh)
{
  OfxPropertySetHandle attrib = NULL;
  MFX_ENSURE(attributeDefine(mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, 3, kOfxMeshAttribTypeFloat, NULL, &attrib));
  MFX_ENSURE(attributeDefine(mesh, kOfxMeshAttribCorner, kOfxMeshAttribCornerPoint, 1, kOfxMeshAttribTypeInt, NULL, &attrib));
  MFX_ENSURE(attributeDefine(mesh, kOfxMeshAttribFace, kOfxMeshAttribFaceSize, 1, kOfxMeshAttribTypeInt, NULL, &attrib));
  return kOfxStatOK;
}

const OfxMeshEffectSuiteV1 meshEffectSuiteV1 = {
  NULL, // OfxStatus (*getPropertySet)(OfxMeshEffectHandle meshEffect,
  getParamSet, // OfxStatus (*getParamSet)(OfxMeshEffectHandle meshEffect,
  inputDefine, // OfxStatus (*inputDefine)(OfxMeshEffectHandle meshEffect,
  inputGetHandle, // OfxStatus (*inputGetHandle)(OfxMeshEffectHandle meshEffect,
  inputGetPropertySet, // OfxStatus (*inputGetPropertySet)(OfxMeshInputHandle input,
  NULL, // OfxStatus (*inputRequestAttribute)(OfxMeshInputHandle input,
  inputGetMesh, // OfxStatus (*inputGetMesh)(OfxMeshInputHandle input,
  inputReleaseMesh, // OfxStatus (*inputReleaseMesh)(OfxMeshHandle meshHandle);
  attributeDefine, // OfxStatus(*attributeDefine)(OfxMeshHandle meshHandle,
  meshGetAttributeByIndex,
  meshGetAttribute, // OfxStatus(*meshGetAttribute)(OfxMeshHandle meshHandle,
  meshGetPropertySet, // OfxStatus (*meshGetPropertySet)(OfxMeshHandle mesh,
  meshAlloc, // OfxStatus (*meshAlloc)(OfxMeshHandle meshHandle);
  NULL, // int (*abort)(OfxMeshEffectHandle meshEffect);
};
