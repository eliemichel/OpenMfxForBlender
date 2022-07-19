#include "MfxAttributeProps.h"
#include "util/mfx_util.h"

#include <cstring>
#include <cstdio>
#include <cassert>

//-----------------------------------------------------------------------------

void MfxAttributeProps::fetchProperties(const OfxPropertySuiteV1* propertySuite, OfxPropertySetHandle properties)
{
    char* type;
    int isOwner;

    MFX_CHECK(propertySuite->propGetString(properties, kOfxMeshAttribPropType, 0, &type));
    MFX_CHECK(propertySuite->propGetInt(properties, kOfxMeshAttribPropStride, 0, &this->stride));
    MFX_CHECK(propertySuite->propGetInt(properties, kOfxMeshAttribPropComponentCount, 0, &this->componentCount));
    MFX_CHECK(propertySuite->propGetPointer(properties, kOfxMeshAttribPropData, 0, (void**)&this->data));
    MFX_CHECK(propertySuite->propGetInt(properties, kOfxMeshAttribPropIsOwner, 0, &isOwner));

    this->type = AttributeTypeAsEnum(type);
    this->isOwner = (bool)isOwner;
}

void MfxAttributeProps::fetchProperties(const OfxPropertySuiteV1 *propertySuite,
                                        const OfxMeshEffectSuiteV1 *meshEffectSuite,
                                        OfxMeshHandle mesh,
                                        const char *attachment,
                                        const char *name)
{
    OfxPropertySetHandle properties;
    MFX_CHECK(meshEffectSuite->meshGetAttribute(mesh, attachment, name, &properties));
    fetchProperties(propertySuite, properties);
}

void MfxAttributeProps::setProperties(const OfxPropertySuiteV1* propertySuite, OfxPropertySetHandle properties) const
{
    const char* type = AttributeTypeAsString(this->type);

    MFX_CHECK(propertySuite->propSetString(properties, kOfxMeshAttribPropType, 0, type));
    MFX_CHECK(propertySuite->propSetInt(properties, kOfxMeshAttribPropStride, 0, this->stride));
    MFX_CHECK(propertySuite->propSetInt(properties, kOfxMeshAttribPropComponentCount, 0, this->componentCount));
    MFX_CHECK(propertySuite->propSetPointer(properties, kOfxMeshAttribPropData, 0, (void*)this->data));
    MFX_CHECK(propertySuite->propSetInt(properties, kOfxMeshAttribPropIsOwner, 0, (int)this->isOwner));
}

//-----------------------------------------------------------------------------

MfxAttributeType MfxAttributeProps::AttributeTypeAsEnum(const char* mfxType)
{
    if (0 == strcmp(mfxType, kOfxMeshAttribTypeUByte)) {
        return MfxAttributeType::UByte;
    }
    if (0 == strcmp(mfxType, kOfxMeshAttribTypeInt)) {
        return MfxAttributeType::Int;
    }
    if (0 == strcmp(mfxType, kOfxMeshAttribTypeFloat)) {
        return MfxAttributeType::Float;
    }
    printf("Warning: unknown attribute type: %s\n", mfxType);
    return MfxAttributeType::Unknown;
}

const char* MfxAttributeProps::AttributeTypeAsString(MfxAttributeType type) {
    switch (type) {
    case MfxAttributeType::UByte:
        return kOfxMeshAttribTypeUByte;
    case MfxAttributeType::Int:
        return kOfxMeshAttribTypeInt;
    case MfxAttributeType::Float:
        return kOfxMeshAttribTypeFloat;
    case MfxAttributeType::Unknown:
    default:
        printf("Warning: unknown attribute type: %d\n", (int)type);
        return "";
    }
}

MfxAttributeAttachment MfxAttributeProps::AttributeAttachmentAsEnum(const char* mfxAttachment) {
    if (0 == strcmp(mfxAttachment, kOfxMeshAttribPoint)) {
        return MfxAttributeAttachment::Point;
    }
    if (0 == strcmp(mfxAttachment, kOfxMeshAttribCorner)) {
        return MfxAttributeAttachment::Corner;
    }
    if (0 == strcmp(mfxAttachment, kOfxMeshAttribFace)) {
        return MfxAttributeAttachment::Face;
    }
    if (0 == strcmp(mfxAttachment, kOfxMeshAttribMesh)) {
        return MfxAttributeAttachment::Mesh;
    }
    printf("Warning: unknown attribute attachment: %s\n", mfxAttachment);
    return MfxAttributeAttachment::Mesh;
}

const char* MfxAttributeProps::AttributeAttachmentAsString(MfxAttributeAttachment attachment) {
    switch (attachment) {
    case MfxAttributeAttachment::Point:
        return kOfxMeshAttribPoint;
    case MfxAttributeAttachment::Corner:
        return kOfxMeshAttribCorner;
    case MfxAttributeAttachment::Face:
        return kOfxMeshAttribFace;
    case MfxAttributeAttachment::Mesh:
    default:
        return kOfxMeshAttribMesh;
    }
}

MfxAttributeSemantic MfxAttributeProps::AttributeSemanticAsEnum(const char* mfxSemantic) {
    if (nullptr == mfxSemantic) {
        return MfxAttributeSemantic::None;
    }
    if (0 == strcmp(mfxSemantic, kOfxMeshAttribSemanticTextureCoordinate)) {
        return MfxAttributeSemantic::TextureCoordinate;
    }
    if (0 == strcmp(mfxSemantic, kOfxMeshAttribSemanticNormal)) {
        return MfxAttributeSemantic::Normal;
    }
    if (0 == strcmp(mfxSemantic, kOfxMeshAttribSemanticColor)) {
        return MfxAttributeSemantic::Color;
    }
    if (0 == strcmp(mfxSemantic, kOfxMeshAttribSemanticWeight)) {
        return MfxAttributeSemantic::Weight;
    }
    printf("Warning: unknown attribute semantic: %s\n", mfxSemantic);
    return MfxAttributeSemantic::None;
}

const char* MfxAttributeProps::AttributeSemanticAsString(MfxAttributeSemantic semantic) {
    switch (semantic) {
    case MfxAttributeSemantic::TextureCoordinate:
        return kOfxMeshAttribSemanticTextureCoordinate;
    case MfxAttributeSemantic::Normal:
        return kOfxMeshAttribSemanticNormal;
    case MfxAttributeSemantic::Color:
        return kOfxMeshAttribSemanticColor;
    case MfxAttributeSemantic::Weight:
        return kOfxMeshAttribSemanticWeight;
    case MfxAttributeSemantic::None:
        return nullptr;
    default:
        printf("Warning: unknown attribute semantic: %d\n", (int)semantic);
        return nullptr;
    }
}
