#include "MfxAttribute.h"
#include "macros.h"

#include <cstring>

MfxAttribute::MfxAttribute(const MfxHost& host, OfxPropertySetHandle properties)
	: MfxBase(host)
	, m_properties(properties)
{}

OfxStatus MfxAttribute::copyAttributeData(MfxAttributeProps& destination, const MfxAttributeProps& source, int start, int count)
{
    // TODO: use at least OpenMP
    int componentCount =
        source.componentCount < destination.componentCount
        ? source.componentCount
        : destination.componentCount;

    if (source.type == destination.type)
    {
        size_t componentByteSize = 0;
        switch (source.type)
        {
        case MfxAttributeType::UByte:
            componentByteSize = sizeof(unsigned char);
            break;
        case MfxAttributeType::Int:
            componentByteSize = sizeof(int);
            break;
        case MfxAttributeType::Float:
            componentByteSize = sizeof(float);
            break;
        default:
            printf("Error: unsupported attribute type: %d\n", (int)source.type);
            return kOfxStatErrFatal;
        }

        for (int i = 0; i < count; ++i) {
            const void* src = (void*)&source.data[(start + i) * source.stride];
            void* dst = (void*)&destination.data[(start + i) * destination.stride];
            memcpy(dst, src, componentCount * componentByteSize);
        }
        return kOfxStatOK;
    }

    switch (destination.type) {
    case MfxAttributeType::Float:
        switch (source.type) {
        case MfxAttributeType::UByte:
            for (int i = 0; i < count; ++i) {
                const unsigned char* src = (unsigned char*)&source.data[(start + i) * source.stride];
                float* dst = (float*)&destination.data[(start + i) * destination.stride];
                for (int k = 0; k < componentCount; ++k)
                {
                    dst[k] = (float)src[k] / 255.0f;
                }
            }
            return kOfxStatOK;
            break;
        }
        break;
    }
    printf("Warning: unsupported input/output type combinason in copyAttribute: %d -> %d\n", (int)source.type, (int)destination.type);
    return kOfxStatErrUnsupported;
}

//-----------------------------------------------------------------------------

void MfxAttribute::FetchProperties(MfxAttributeProps& props) const
{
    char* type;
    int isOwner;
    MFX_ENSURE(propertySuite->propGetString(m_properties, kOfxMeshAttribPropType, 0, &type));
    MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshAttribPropStride, 0, &props.stride));
    MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshAttribPropComponentCount, 0, &props.componentCount));
    MFX_ENSURE(propertySuite->propGetPointer(m_properties, kOfxMeshAttribPropData, 0, (void**)&props.data));
    MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshAttribPropIsOwner, 0, &isOwner));
    props.type = attributeTypeAsEnum(type);
    props.isOwner = (bool)isOwner;
}

void MfxAttribute::SetProperties(const MfxAttributeProps &props) {
    const char* type = attributeTypeAsString(props.type);

    MFX_ENSURE(propertySuite->propSetString(m_properties, kOfxMeshAttribPropType, 0, type));
    MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxMeshAttribPropStride, 0, props.stride));
    MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxMeshAttribPropComponentCount, 0, props.componentCount));
    MFX_ENSURE(propertySuite->propSetPointer(m_properties, kOfxMeshAttribPropData, 0, (void*)props.data));
    MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxMeshAttribPropIsOwner, 0, (int)props.isOwner));
}

void MfxAttribute::CopyFrom(const MfxAttribute& other, int start, int count)
{
    MfxAttributeProps sourceProps, destinationProps;
    other.FetchProperties(sourceProps);
    FetchProperties(destinationProps);
    copyAttributeData(destinationProps, sourceProps, start, count);
}

void MfxAttribute::ForwardFrom(const MfxAttribute &other) {
    MfxAttributeProps sourceProps, destinationProps;
    other.FetchProperties(sourceProps);

    destinationProps = sourceProps;
    destinationProps.isOwner = false;

    SetProperties(destinationProps);
}

//-----------------------------------------------------------------------------

MfxAttributeType MfxAttribute::attributeTypeAsEnum(const char* mfxType)
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

const char* MfxAttribute::attributeTypeAsString(MfxAttributeType type) {
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

MfxAttributeAttachment MfxAttribute::attributeAttachmentAsEnum(const char* mfxAttachment) {
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

const char* MfxAttribute::attributeAttachmentAsString(MfxAttributeAttachment attachment) {
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

MfxAttributeSemantic MfxAttribute::attributeSemanticAsEnum(const char* mfxSemantic) {
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

const char* MfxAttribute::attributeSemanticAsString(MfxAttributeSemantic semantic) {
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
