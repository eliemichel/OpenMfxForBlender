#pragma once

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
        , data(NULL)
        , isOwner(false)
    {}

    MfxAttributeType type;
    int stride;
    int componentCount;
    char* data;
    bool isOwner;

    /**
     * Compute a pointer to the index-th element of the data buffer.
     * The template argument should be set with respect to `type`.
     */
    template<typename T>
    T* at(int index) { return reinterpret_cast<T*>(data + index * stride); }

    template<typename T>
    const T* at(int index) const { return reinterpret_cast<T*>(data + index * stride); }
};
