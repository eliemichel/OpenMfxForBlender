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

#include "AttributeEnums.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <cstring>
#include <cstdio>
#include <cassert>

namespace OpenMfx {

AttributeType attributeTypeAsEnum(const char* mfxType)
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
    WARN_LOG << "Unknown attribute type: " << mfxType;
    return AttributeType::Unknown;
}

const char* attributeTypeAsString(AttributeType type) {
    switch (type) {
    case AttributeType::UByte:
        return kOfxMeshAttribTypeUByte;
    case AttributeType::Int:
        return kOfxMeshAttribTypeInt;
    case AttributeType::Float:
        return kOfxMeshAttribTypeFloat;
    case AttributeType::Unknown:
    default:
        WARN_LOG << "Unknown attribute type: " << (int)type;
        return "";
    }
}

int byteSizeOf(AttributeType type) {
    switch (type)
    {
    case AttributeType::UByte:
        return sizeof(unsigned char);
    case AttributeType::Int:
        return sizeof(int);
    case AttributeType::Float:
        return sizeof(float);
    case AttributeType::Unknown:
    default:
        ERR_LOG << "Unknown type";
        return 0;
    }
}

AttributeAttachment attributeAttachmentAsEnum(const char* mfxAttachment) {
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
    WARN_LOG << "Unknown attribute attachment: " << mfxAttachment;
    return AttributeAttachment::Mesh;
}

const char* attributeAttachmentAsString(AttributeAttachment attachment) {
    switch (attachment) {
    case AttributeAttachment::Point:
        return kOfxMeshAttribPoint;
    case AttributeAttachment::Corner:
        return kOfxMeshAttribCorner;
    case AttributeAttachment::Face:
        return kOfxMeshAttribFace;
    case AttributeAttachment::Mesh:
        return kOfxMeshAttribMesh;
    case AttributeAttachment::Invalid:
    default:
        WARN_LOG << "Invalid attribute attachment: " << (int)attachment;
        return kOfxMeshAttribMesh;
    }
}

AttributeSemantic attributeSemanticAsEnum(const char* mfxSemantic) {
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
    WARN_LOG << "Unknown attribute semantic: " << mfxSemantic;
    return AttributeSemantic::None;
}

const char* attributeSemanticAsString(AttributeSemantic semantic) {
    switch (semantic) {
    case AttributeSemantic::TextureCoordinate:
        return kOfxMeshAttribSemanticTextureCoordinate;
    case AttributeSemantic::Normal:
        return kOfxMeshAttribSemanticNormal;
    case AttributeSemantic::Color:
        return kOfxMeshAttribSemanticColor;
    case AttributeSemantic::Weight:
        return kOfxMeshAttribSemanticWeight;
    case AttributeSemantic::None:
        return nullptr;
    default:
        WARN_LOG << "Unknown attribute semantic: " << (int)semantic;
        return nullptr;
    }
}

} // namespace OpenMfx
