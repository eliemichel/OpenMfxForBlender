#pragma once

#include "ofxProperty.h"
#include "ofxMeshEffect.h"

enum class MfxAttributeType {
    Unknown = -1,
    UByte,
    Int,
    Float,
};

enum class MfxAttributeAttachment {
    Point,
    Corner,
    Face,
    Mesh,
};

enum class MfxAttributeSemantic {
    None = -1,
    TextureCoordinate,
    Normal,
    Color,
    Weight,
};

/**
 * Mfx*Props classes are a little different: for caching and convenience, they
 * store some data and care must be taken not to copy it around too much
 * (though it does not directly store the attribute data, only metadata).
 */
struct MfxAttributeProps
{
    MfxAttributeProps()
        : type(MfxAttributeType::Unknown)
        , stride(0)
        , componentCount(0)
        , data(nullptr)
        , isOwner(false)
    {}

    MfxAttributeType type;
    int stride;
    int componentCount;
    char* data;
    bool isOwner;

    void fetchProperties(const OfxPropertySuiteV1* propertySuite, OfxPropertySetHandle properties);
    void fetchProperties(const OfxPropertySuiteV1* propertySuite, const OfxMeshEffectSuiteV1 *meshEffectSuite, OfxMeshHandle mesh, const char *attachment, const char *name);
    void setProperties(const OfxPropertySuiteV1* propertySuite, OfxPropertySetHandle properties) const;

    template<typename T> inline T *at(int index) {
      return reinterpret_cast<T *>(data + static_cast<size_t>(index) * static_cast<size_t>(stride));
    }

    template<typename T> inline const T *at(int index) const {
      return reinterpret_cast<const T *>(data + static_cast<size_t>(index) * static_cast<size_t>(stride));
    }

    static MfxAttributeType AttributeTypeAsEnum(const char *mfxType);
    static const char* AttributeTypeAsString(MfxAttributeType type);
    static MfxAttributeAttachment AttributeAttachmentAsEnum(const char* mfxAttachment);
    static const char* AttributeAttachmentAsString(MfxAttributeAttachment attachment);
    static MfxAttributeSemantic AttributeSemanticAsEnum(const char* mfxSemantic);
    static const char* AttributeSemanticAsString(MfxAttributeSemantic semantic);
};
