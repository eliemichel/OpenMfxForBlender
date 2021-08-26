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
#include <stdexcept>

#include "ofxMeshEffect.h" // for kOfxMeshEffectPropContext
#include "ofxExtras.h" // for kOfxHostPropBeforeMeshReleaseCb

#include "Allocator.h"
#include "properties.h"

using namespace OpenMfx;

// OFX PROPERTIES SUITE

// // OfxPropertyStruct

void OfxPropertyStruct::deep_copy_from(const OfxPropertyStruct &other)
{
  m_name = other.m_name;  // weak pointer?
  this->value[0] = other.value[0];
  this->value[1] = other.value[1];
  this->value[2] = other.value[2];
  this->value[3] = other.value[3];
}

// // OfxPropertySetStruct

OfxPropertySetStruct::OfxPropertySetStruct(PropertySetContext context)
{
  this->context = context;
}

bool OfxPropertySetStruct::check_property_context(PropertySetContext context, PropertyType type, const char *property)
{
  switch (context) {
    case PropertySetContext::MeshEffect:
    return (
      (0 == strcmp(property, kOfxMeshEffectPropContext) && type == PROP_TYPE_STRING) ||
      false
    );
    case PropertySetContext::Input:
    return (
      (0 == strcmp(property, kOfxPropLabel) && type == PROP_TYPE_STRING) ||
      false
    );
    case PropertySetContext::Host:
    return (
      (0 == strcmp(property, kOfxHostPropBeforeMeshReleaseCb) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxHostPropBeforeMeshGetCb) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxMeshPropHostHandle) && type == PROP_TYPE_POINTER) ||
      false
    );
    case PropertySetContext::Mesh:
    return (
      (0 == strcmp(property, kOfxMeshPropInternalData) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxMeshPropHostHandle)   && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxMeshPropPointCount)   && type == PROP_TYPE_INT)     ||
      (0 == strcmp(property, kOfxMeshPropCornerCount)  && type == PROP_TYPE_INT)     ||
      (0 == strcmp(property, kOfxMeshPropFaceCount)    && type == PROP_TYPE_INT)     ||
      (0 == strcmp(property, kOfxMeshPropNoLooseEdge)  && type == PROP_TYPE_INT)     ||
      (0 == strcmp(property, kOfxMeshPropConstantFaceSize) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxMeshPropAttributeCount) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxMeshPropTransformMatrix) && type == PROP_TYPE_POINTER) ||
      false
    );
    case PropertySetContext::Param:
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
    case PropertySetContext::Attrib:
    return (
      (0 == strcmp(property, kOfxMeshAttribPropData) && type == PROP_TYPE_POINTER) ||
      (0 == strcmp(property, kOfxMeshAttribPropStride) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxMeshAttribPropComponentCount) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kOfxMeshAttribPropType) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxMeshAttribPropSemantic) && type == PROP_TYPE_STRING) ||
      (0 == strcmp(property, kOfxMeshAttribPropIsOwner) && type == PROP_TYPE_INT) ||
      (0 == strcmp(property, kMeshAttribRequestPropMandatory) && type == PROP_TYPE_INT) ||
      false);
    case PropertySetContext::ActionIdentityIn:
    return (
        (0 == strcmp(property, kOfxPropTime) && type == PROP_TYPE_INT) ||
        false);
    case PropertySetContext::ActionIdentityOut:
    return (
        (0 == strcmp(property, kOfxPropName) && type == PROP_TYPE_STRING) ||
        (0 == strcmp(property, kOfxPropTime) && type == PROP_TYPE_INT) ||
        false);
    case PropertySetContext::Other:
  default:
    printf("Warning: PROP_CTX_OTHER is depreciated.\n");
    return true;
  }
}
