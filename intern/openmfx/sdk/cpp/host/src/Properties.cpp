/*
 * Copyright 2019-2022 Elie Michel
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

#include "Allocator.h"
#include "Properties.h"

#include "ofxExtras.h"
#include <ofxMeshEffect.h>

#include <cstring>
#include <cstdio>
#include <stdexcept>

using namespace OpenMfx;

// OFX PROPERTIES SUITE

// // OfxPropertyStruct

void OfxPropertyStruct::deep_copy_from(const OfxPropertyStruct& other)
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

bool OfxPropertySetStruct::check_property_context(PropertySetContext context, PropertyType type, const char* property)
{
    switch (context) {
    case PropertySetContext::MeshEffect:
        return (
            (0 == strcmp(property, kOfxMeshEffectPropContext) && type == PropertyType::String) ||
            false
            );
    case PropertySetContext::Input:
        return (
            (0 == strcmp(property, kOfxPropLabel) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxInputPropRequestIOMap) && type == PropertyType::Int) ||
            false
            );
    case PropertySetContext::Host:
        return (
            (0 == strcmp(property, kOfxHostPropBeforeMeshReleaseCb) && type == PropertyType::Pointer) ||
            (0 == strcmp(property, kOfxHostPropBeforeMeshGetCb) && type == PropertyType::Pointer) ||
            (0 == strcmp(property, kOfxMeshPropHostHandle) && type == PropertyType::Pointer) ||
            false
            );
    case PropertySetContext::Mesh:
        return (
            (0 == strcmp(property, kOfxMeshPropInternalData) && type == PropertyType::Pointer) ||
            (0 == strcmp(property, kOfxMeshPropHostHandle) && type == PropertyType::Pointer) ||
            (0 == strcmp(property, kOfxMeshPropPointCount) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxMeshPropCornerCount) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxMeshPropFaceCount) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxMeshPropNoLooseEdge) && type == PropertyType::Int) ||
            //(0 == strcmp(property, "OfxMeshPropIsAttributeMap")  && type == PropertyType::Int)     || // TODO use defines
            //(0 == strcmp(property, "OfxMeshPropOutputPointsCount")  && type == PropertyType::Int)     ||
            //(0 == strcmp(property, "OfxMeshPropOriginPointsTotalPoolSize")  && type == PropertyType::Int)     ||
            (0 == strcmp(property, kOfxMeshPropConstantFaceSize) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxMeshPropAttributeCount) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxMeshPropTransformMatrix) && type == PropertyType::Pointer) ||
            (0 == strcmp(property, kOfxMeshPropIOMap) && type == PropertyType::Pointer) ||
            false
            );
    case PropertySetContext::Param:
        return (
            (0 == strcmp(property, kOfxParamPropType) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxParamPropScriptName) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxParamPropDefault) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxParamPropDefault) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxParamPropDefault) && type == PropertyType::Double) ||
            (0 == strcmp(property, kOfxParamPropDefault) && type == PropertyType::Pointer) ||
            (0 == strcmp(property, kOfxPropLabel) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxParamPropMin) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxParamPropMin) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxParamPropMin) && type == PropertyType::Double) ||
            (0 == strcmp(property, kOfxParamPropMin) && type == PropertyType::Pointer) ||
            (0 == strcmp(property, kOfxParamPropMax) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxParamPropMax) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxParamPropMax) && type == PropertyType::Double) ||
            (0 == strcmp(property, kOfxParamPropMax) && type == PropertyType::Pointer) ||
            false
            );
    case PropertySetContext::Attrib:
        return (
            (0 == strcmp(property, kOfxMeshAttribPropData) && type == PropertyType::Pointer) ||
            (0 == strcmp(property, kOfxMeshAttribPropStride) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxMeshAttribPropComponentCount) && type == PropertyType::Int) ||
            (0 == strcmp(property, kOfxMeshAttribPropType) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxMeshAttribPropSemantic) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxMeshAttribPropIsOwner) && type == PropertyType::Int) ||
            (0 == strcmp(property, kMeshAttribRequestPropMandatory) && type == PropertyType::Int) ||
            false);
    case PropertySetContext::ActionIdentityIn:
        return (
            (0 == strcmp(property, kOfxPropTime) && type == PropertyType::Int) ||
            false);
    case PropertySetContext::ActionIdentityOut:
        return (
            (0 == strcmp(property, kOfxPropName) && type == PropertyType::String) ||
            (0 == strcmp(property, kOfxPropTime) && type == PropertyType::Int) ||
            false);
    case PropertySetContext::Other:
    default:
        printf("Warning: PROP_CTX_OTHER is depreciated.\n");
        return true;
    }
}
