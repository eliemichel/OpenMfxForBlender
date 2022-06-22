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

#include "attributes.h"
#include "propertySuite.h"
#include "ofxMeshEffect.h"

#include <cstring>
#include <algorithm>

using namespace OpenMfx;

// // OfxMeshAttributeStruct

OfxAttributeStruct::OfxAttributeStruct()
    : properties(PropertySetContext::Attrib)
{}

void OfxAttributeStruct::set_name(const char *name)
{
    m_name = std::string(name);
}

void OfxAttributeStruct::deep_copy_from(const OfxAttributeStruct &other)
{
  m_name = other.m_name;
  m_attachment = other.m_attachment;
  
  properties.deep_copy_from(other.properties);
}

AttributeType OfxAttributeStruct::type()
{
    char* stringType;
    propGetString(&properties, kOfxMeshAttribPropType, 0, &stringType);

    if (0 == strcmp(stringType, kOfxMeshAttribTypeUByte)) {
        return AttributeType::UByte;
    }
    else if (0 == strcmp(stringType, kOfxMeshAttribTypeInt)) {
        return AttributeType::Int;
    }
    else if (0 == strcmp(stringType, kOfxMeshAttribTypeFloat)) {
        return AttributeType::Float;
    }
    else {
        return AttributeType::Invalid;
    }
}

void OfxAttributeStruct::setIndex(const Index& index)
{
    m_attachment = index.first;
    m_name = index.second;
}

OfxAttributeStruct::Index OfxAttributeStruct::index() const
{
    return std::make_pair(m_attachment, m_name);
}

AttributeAttachment OfxAttributeStruct::attachmentAsEnum(const char* mfxAttachment)
{
    if (0 == strcmp(mfxAttachment, kOfxMeshAttribPoint)) {
        return AttributeAttachment::Point;
    }
    if (0 == strcmp(mfxAttachment, kOfxMeshAttribCorner)) {
        return AttributeAttachment::Corner;
    }
    if (0 == strcmp(mfxAttachment, kOfxMeshAttribFace)) {
        return AttributeAttachment::Face;
    }
    if (0 == strcmp(mfxAttachment, kOfxMeshAttribMesh)) {
        return AttributeAttachment::Mesh;
    }
    printf("Warning: unknown attachment type: %s\n", mfxAttachment);
    return AttributeAttachment::Invalid;
}

AttributeType OfxAttributeStruct::typeAsEnum(const char* mfxType)
{
    if (0 == strcmp(mfxType, kOfxMeshAttribTypeUByte)) {
        return AttributeType::UByte;
    }
    if (0 == strcmp(mfxType, kOfxMeshAttribTypeInt)) {
        return AttributeType::Int;
    }
    if (0 == strcmp(mfxType, kOfxMeshAttribTypeFloat)) {
        return AttributeType::Float;
    }
    printf("Warning: unknown attribute type: %s\n", mfxType);
    return AttributeType::Invalid;
}

AttributeSemantic OfxAttributeStruct::semanticAsEnum(const char* mfxSemantic)
{
    if (nullptr == mfxSemantic) {
        return AttributeSemantic::None;
    }
    if (0 == strcmp(mfxSemantic, kOfxMeshAttribSemanticTextureCoordinate)) {
        return AttributeSemantic::TextureCoordinate;
    }
    if (0 == strcmp(mfxSemantic, kOfxMeshAttribSemanticNormal)) {
        return AttributeSemantic::Normal;
    }
    if (0 == strcmp(mfxSemantic, kOfxMeshAttribSemanticColor)) {
        return AttributeSemantic::Color;
    }
    if (0 == strcmp(mfxSemantic, kOfxMeshAttribSemanticWeight)) {
        return AttributeSemantic::Weight;
    }
    printf("Warning: unknown attribute semantic: %s\n", mfxSemantic);
    return AttributeSemantic::None;
}

int OfxAttributeStruct::byteSizeOf(AttributeType type)
{
    switch (type)
    {
    case AttributeType::UByte:
        return sizeof(unsigned char);
    case AttributeType::Int:
        return sizeof(int);
    case AttributeType::Float:
        return sizeof(float);
    default:
        printf("Error: unsupported attribute type: %d\n", type);
        return 0;
    }
}

const char* OfxAttributeStruct::attachmentAsString(AttributeAttachment attachment)
{
    switch (attachment) {
    case AttributeAttachment::Point:
        return kOfxMeshAttribPoint;
    case AttributeAttachment::Corner:
        return kOfxMeshAttribCorner;
    case AttributeAttachment::Face:
        return kOfxMeshAttribFace;
    case AttributeAttachment::Mesh:
        return kOfxMeshAttribMesh;
    default:
        return nullptr;
    }
}

const char* OfxAttributeStruct::typeAsString(AttributeType type)
{
    switch (type) {
    case AttributeType::Float:
        return kOfxMeshAttribTypeFloat;
    case AttributeType::Int:
        return kOfxMeshAttribTypeInt;
    case AttributeType::UByte:
        return kOfxMeshAttribTypeUByte;
    default:
        return nullptr;
    }
}

const char* OfxAttributeStruct::semanticAsString(AttributeSemantic semantic)
{
    switch (semantic) {
    case AttributeSemantic::Color:
        return kOfxMeshAttribSemanticColor;
    case AttributeSemantic::Normal:
        return kOfxMeshAttribSemanticNormal;
    case AttributeSemantic::TextureCoordinate:
        return kOfxMeshAttribSemanticTextureCoordinate;
    case AttributeSemantic::Weight:
        return kOfxMeshAttribSemanticWeight;
    default:
        return nullptr;
    }
}

bool OfxAttributeStruct::copy_data_from(const OfxAttributeStruct& source, int start, int count)
{
    AttributeType sourceType = typeAsEnum(source.properties[kOfxMeshAttribPropType].value[0].as_char);
    AttributeType destinationType = typeAsEnum(properties[kOfxMeshAttribPropType].value[0].as_char);

    int sourceComponentCount = source.properties[kOfxMeshAttribPropComponentCount].value[0].as_int;
    int destinationComponentCount = properties[kOfxMeshAttribPropComponentCount].value[0].as_int;

    char* sourceData = (char*)source.properties[kOfxMeshAttribPropData].value[0].as_pointer;
    char* destinationData = (char*)properties[kOfxMeshAttribPropData].value[0].as_pointer;

    int sourceStride = source.properties[kOfxMeshAttribPropStride].value[0].as_int;
    int destinationStride = properties[kOfxMeshAttribPropStride].value[0].as_int;

    // TODO: use at least OpenMP
    int componentCount = std::min(sourceComponentCount, destinationComponentCount);

    if (sourceType == destinationType)
    {
        size_t componentByteSize = byteSizeOf(sourceType);
        if (componentByteSize == 0) return false;

        for (int i = 0; i < count; ++i) {
            const void* src = (void*)&sourceData[(start + i) * sourceStride];
            void* dst = (void*)&destinationData[(start + i) * destinationStride];
            memcpy(dst, src, componentCount * componentByteSize);
        }
        return true;
    }

    switch (destinationType) {
    case AttributeType::Float:
        switch (sourceType) {
        case AttributeType::UByte:
            for (int i = 0; i < count; ++i) {
                const unsigned char* src = (unsigned char*)&sourceData[(start + i) * sourceStride];
                float* dst = (float*)&destinationData[(start + i) * destinationStride];
                for (int k = 0; k < componentCount; ++k)
                {
                    dst[k] = (float)src[k] / 255.0f;
                }
            }
            return true;
            break;
        }
        break;
    }
    printf("Warning: unsupported input/output type combinason in copyAttribute: %d -> %d\n", sourceType, destinationType);
    return false;
}
