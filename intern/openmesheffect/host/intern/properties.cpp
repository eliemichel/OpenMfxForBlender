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

#include <string.h>
#include <stdio.h>

#include "ofxMeshEffect.h" // for kOfxMeshEffectPropContext
#include "ofxExtras.h" // for kOfxHostPropBeforeMeshReleaseCb

#include "util/memory_util.h"

#include "properties.h"

// OFX PROPERTIES SUITE

// // OfxPropertyStruct

OfxPropertyStruct::OfxPropertyStruct()
{
}

OfxPropertyStruct::~OfxPropertyStruct()
{
}

void OfxPropertyStruct::deep_copy_from(const OfxPropertyStruct &other)
{
  this->name = other.name;  // weak pointer?
  this->value[0] = other.value[0];
  this->value[1] = other.value[1];
  this->value[2] = other.value[2];
  this->value[3] = other.value[3];
}

// // OfxPropertySetStruct

OfxPropertySetStruct::OfxPropertySetStruct()
{
  num_properties = 0;
  properties = NULL;
  context = PROP_CTX_OTHER;
}

OfxPropertySetStruct::~OfxPropertySetStruct()
{
  for (int i = 0; i < this->num_properties; ++i) {
    free_array(this->properties[i]);
  }
  this->num_properties = 0;
  if (NULL != this->properties) {
    free_array(this->properties);
    this->properties = NULL;
  }
}

int OfxPropertySetStruct::find_property(const char *property) const
{
  for (int i = 0 ; i < this->num_properties ; ++i) {
    if (0 == strcmp(this->properties[i]->name, property)) {
      return i;
    }
  }
  return -1;
}

void OfxPropertySetStruct::append_properties(int count)
{
  int old_num_properties = this->num_properties;
  OfxPropertyStruct **old_properties = this->properties;
  this->num_properties += count;
  this->properties = (OfxPropertyStruct **)malloc_array(
      sizeof(OfxPropertyStruct *), this->num_properties, "properties");
  for (int i = 0; i < this->num_properties; ++i) {
    this->properties[i] = i < old_num_properties ?
                                    old_properties[i] :
                                    (OfxPropertyStruct *)malloc_array(
                                        sizeof(OfxPropertyStruct), 1, "property");
  }
  if (NULL != old_properties) {
    free_array(old_properties);
  }
}

void OfxPropertySetStruct::remove_property(int index)
{
  OfxPropertyStruct **old_properties = this->properties;
  this->num_properties -= 1;
  this->properties = (OfxPropertyStruct **)malloc_array(
      sizeof(OfxPropertyStruct *), this->num_properties, "properties");
  for (int i = 0; i < this->num_properties; ++i) {
    this->properties[i] = i < index ? old_properties[i] : old_properties[i + 1];
  }
  if (NULL != old_properties) {
    free_array(old_properties[index]);
    free_array(old_properties);
  }
}

int OfxPropertySetStruct::ensure_property(const char *property)
{
  int i = find_property(property);
  if (i == -1) {
    append_properties(1);
    i = this->num_properties - 1;
    this->properties[i]->name = property;
  }
  return i;
}

void OfxPropertySetStruct::deep_copy_from(const OfxPropertySetStruct &other)
{
  append_properties(other.num_properties);
  for (int i = 0 ; i < this->num_properties ; ++i) {
    this->properties[i]->deep_copy_from(*other.properties[i]);
  }
  this->context = other.context;
}

bool OfxPropertySetStruct::check_property_context(PropertySetContext context, PropertyType type, const char *property)
{
  switch (context) {
  case PROP_CTX_MESH_EFFECT:
    return (
      (0 == strcmp(property, kOfxMeshEffectPropContext) && type == PROP_TYPE_STRING) ||
      false
    );
  case PROP_CTX_INPUT:
    return (
      (0 == strcmp(property, kOfxPropLabel) && type == PROP_TYPE_STRING) ||
      false
    );
  case PROP_CTX_HOST:
    return (
      (0 == strcmp(property, kOfxHostPropBeforeMeshReleaseCb) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxHostPropBeforeMeshGetCb) && type == PROP_TYPE_POINTER) ||
      false
    );
  case PROP_CTX_MESH:
    return (
      (0 == strcmp(property, kOfxMeshPropInternalData) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxMeshPropHostHandle)   && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxMeshPropPointCount)   && type == PROP_TYPE_INT)     ||
      (0 == strcmp(property, kOfxMeshPropVertexCount)  && type == PROP_TYPE_INT)     ||
      (0 == strcmp(property, kOfxMeshPropFaceCount)    && type == PROP_TYPE_INT)     ||
      false
    );
  case PROP_CTX_PARAM:
    return (
      (0 == strcmp(property, kOfxParamPropType) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxParamPropScriptName) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxParamPropDefault) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxParamPropDefault) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxParamPropDefault) && type == PROP_TYPE_DOUBLE) ||
      (0 == strcmp(property, kOfxParamPropDefault) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxPropLabel) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxParamPropMin) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxParamPropMin) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxParamPropMin) && type == PROP_TYPE_DOUBLE) ||
      (0 == strcmp(property, kOfxParamPropMin) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxParamPropMax) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxParamPropMax) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxParamPropMax) && type == PROP_TYPE_DOUBLE) ||
      (0 == strcmp(property, kOfxParamPropMax) && type == PROP_TYPE_POINTER) ||
      false
      );
  case PROP_CTX_ATTRIB:
    return (
      (0 == strcmp(property, kOfxMeshAttribPropData) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxMeshAttribPropStride) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxMeshAttribPropComponentCount) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxMeshAttribPropType) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxMeshAttribPropIsOwner) && type == PROP_TYPE_INT) ||
      false
      );
  case PROP_CTX_OTHER:
  default:
    printf("Warning: PROP_CTX_OTHER is depreciated.\n");
    return true;
  }
}

// // Property Suite Entry Points

const OfxPropertySuiteV1 gPropertySuiteV1 = {
    /* propSetPointer */   propSetPointer,
    /* propSetString */    propSetString,
    /* propSetDouble */    propSetDouble,
    /* propSetInt */       propSetInt,
    /* propSetPointerN */  propSetPointerN,
    /* propSetStringN */   propSetStringN,
    /* propSetDoubleN */   propSetDoubleN,
    /* propSetIntN */      propSetIntN,
    /* propGetPointer */   propGetPointer,
    /* propGetString */    propGetString,
    /* propGetDouble */    propGetDouble,
    /* propGetInt */       propGetInt,
    /* propGetPointerN */  propGetPointerN,
    /* propGetStringN */   propGetStringN,
    /* propGetDoubleN */   propGetDoubleN,
    /* propGetIntN */      propGetIntN,
    /* propReset */        propReset,
    /* propGetDimension */ propGetDimension
};

OfxStatus propSetPointer(OfxPropertySetHandle properties, const char *property, int index, void *value) {
  if (false == OfxPropertySetStruct::check_property_context(
                   properties->context, PROP_TYPE_POINTER, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = properties->ensure_property(property);
  properties->properties[i]->value[index].as_pointer = value;
  return kOfxStatOK;
}

OfxStatus propSetString(OfxPropertySetHandle properties, const char *property, int index, const char *value) {
  if (false == OfxPropertySetStruct::check_property_context(properties->context, PROP_TYPE_STRING, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = properties->ensure_property(property);
  properties->properties[i]->value[index].as_const_char = value;
  return kOfxStatOK;
}

OfxStatus propSetDouble(OfxPropertySetHandle properties, const char *property, int index, double value) {
  if (false == OfxPropertySetStruct::check_property_context(
                   properties->context, PROP_TYPE_DOUBLE, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = properties->ensure_property(property);
  properties->properties[i]->value[index].as_double = value;
  return kOfxStatOK;
}

OfxStatus propSetInt(OfxPropertySetHandle properties, const char *property, int index, int value) {
  if (false == OfxPropertySetStruct::check_property_context(
                   properties->context, PROP_TYPE_INT, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = properties->ensure_property(property);
  properties->properties[i]->value[index].as_int = value;
  return kOfxStatOK;
}

OfxStatus propSetPointerN(OfxPropertySetHandle properties, const char *property, int count, void *const*value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propSetPointer(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propSetStringN(OfxPropertySetHandle properties, const char *property, int count, const char *const*value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propSetString(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propSetDoubleN(OfxPropertySetHandle properties, const char *property, int count, const double *value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propSetDouble(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propSetIntN(OfxPropertySetHandle properties, const char *property, int count, const int *value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propSetInt(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propGetPointer(OfxPropertySetHandle properties, const char *property, int index, void **value) {
  if (false == OfxPropertySetStruct::check_property_context(properties->context, PROP_TYPE_POINTER, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = properties->ensure_property(property);
  *value = properties->properties[i]->value[index].as_pointer;
  return kOfxStatOK;
}

OfxStatus propGetString(OfxPropertySetHandle properties, const char *property, int index, char **value) {
  if (false == OfxPropertySetStruct::check_property_context(
                   properties->context, PROP_TYPE_STRING, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = properties->ensure_property(property);
  *value = properties->properties[i]->value[index].as_char;
  return kOfxStatOK;
}

OfxStatus propGetDouble(OfxPropertySetHandle properties, const char *property, int index, double *value) {
  if (false == OfxPropertySetStruct::check_property_context(
                   properties->context, PROP_TYPE_DOUBLE, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = properties->ensure_property(property);
  *value = properties->properties[i]->value[index].as_double;
  return kOfxStatOK;
}

OfxStatus propGetInt(OfxPropertySetHandle properties, const char *property, int index, int *value) {
  if (false == OfxPropertySetStruct::check_property_context(
                   properties->context, PROP_TYPE_INT, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = properties->ensure_property(property);
  *value = properties->properties[i]->value[index].as_int;
  return kOfxStatOK;
}

OfxStatus propGetPointerN(OfxPropertySetHandle properties, const char *property, int count, void **value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propGetPointer(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propGetStringN(OfxPropertySetHandle properties, const char *property, int count, char **value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propGetString(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propGetDoubleN(OfxPropertySetHandle properties, const char *property, int count, double *value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propGetDouble(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propGetIntN(OfxPropertySetHandle properties, const char *property, int count, int *value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propGetInt(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propReset(OfxPropertySetHandle properties, const char *property) {
  (void)properties;
  (void)property;
  return kOfxStatReplyDefault;
}

OfxStatus propGetDimension(OfxPropertySetHandle properties, const char *property, int *count) {
  (void)properties;
  (void)property;
  *count = 4;
  return kOfxStatOK;
}

