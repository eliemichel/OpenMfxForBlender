#include "attribute.h"

#include <OpenMfx/Sdk/C/Common> // for ENSURE

#include <string.h>

#define ENSURE(XXX) MFX_ENSURE(runtime->XXX)

MfxAttributeType mfxAttributeTypeAsEnum(const char* attr_type) {
    if (0 == strcmp(attr_type, kOfxMeshAttribTypeUByte)) {
        return MFX_UBYTE_ATTR;
    }
    if (0 == strcmp(attr_type, kOfxMeshAttribTypeInt)) {
        return MFX_INT_ATTR;
    }
    if (0 == strcmp(attr_type, kOfxMeshAttribTypeFloat)) {
        return MFX_FLOAT_ATTR;
    }
    printf("Warning: inknown attribute type: %s\n", attr_type);
    return MFX_UNKNOWN_ATTR;
}

const char* mfxAttributeTypeAsString(MfxAttributeType type) {
    static const char* names[] = {
        "OfxMeshAttribTypeUnknown",
        kOfxMeshAttribTypeUByte,
        kOfxMeshAttribTypeInt,
        kOfxMeshAttribTypeFloat,
    };
    return names[type - MFX_UNKNOWN_ATTR];
}

OfxStatus mfxGetAttributeProperties(
    const MfxPluginRuntime* runtime,
    const OfxPropertySetHandle attrib,
    MfxAttributeProperties *props)
{
    ENSURE(propertySuite->propGetInt(attrib, kOfxMeshAttribPropComponentCount, 0, &props->component_count));

    char* type;
    ENSURE(propertySuite->propGetString(attrib, kOfxMeshAttribPropType, 0, &type));
    props->type = mfxAttributeTypeAsEnum(type);

    ENSURE(propertySuite->propGetString(attrib, kOfxMeshAttribPropSemantic, 0, &props->semantic));
    ENSURE(propertySuite->propGetPointer(attrib, kOfxMeshAttribPropData, 0, (void**)&props->data));

    int byte_stride;
    ENSURE(propertySuite->propGetInt(attrib, kOfxMeshAttribPropStride, 0, &byte_stride));
    props->byte_stride = (size_t)byte_stride;

    ENSURE(propertySuite->propGetInt(attrib, kOfxMeshAttribPropIsOwner, 0, &props->is_owner));
    return kOfxStatOK;
}

OfxStatus mfxSetAttributeProperties(
    const MfxPluginRuntime* runtime,
    OfxPropertySetHandle attrib,
    const MfxAttributeProperties *props)
{
    ENSURE(propertySuite->propSetInt(attrib, kOfxMeshAttribPropComponentCount, 0, props->component_count));
    const char *type = mfxAttributeTypeAsString(props->type);
    ENSURE(propertySuite->propSetString(attrib, kOfxMeshAttribPropType, 0, type));
    ENSURE(propertySuite->propSetString(attrib, kOfxMeshAttribPropSemantic, 0, props->semantic));
    ENSURE(propertySuite->propSetPointer(attrib, kOfxMeshAttribPropData, 0, (void*)props->data));
    ENSURE(propertySuite->propSetInt(attrib, kOfxMeshAttribPropStride, 0, (int)props->byte_stride));
    ENSURE(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, props->is_owner));
    return kOfxStatOK;
}

OfxStatus mfxGetAttribute(
    const MfxPluginRuntime* runtime,
    OfxMeshHandle mesh,
    const char* attachment,
    const char* name,
    MfxAttributeProperties* props
) {
    OfxPropertySetHandle attrib;
    ENSURE(meshEffectSuite->meshGetAttribute(mesh, attachment, name, &attrib));
    MFX_ENSURE(mfxGetAttributeProperties(runtime, attrib, props));
    return kOfxStatOK;
}

OfxStatus mfxGetPointAttribute(
    const MfxPluginRuntime* runtime,
    OfxMeshHandle mesh,
    const char* name,
    MfxAttributeProperties* props
) {
    return mfxGetAttribute(runtime, mesh, kOfxMeshAttribPoint, name, props);
}

OfxStatus mfxGetCornerAttribute(
    const MfxPluginRuntime* runtime,
    OfxMeshHandle mesh,
    const char* name,
    MfxAttributeProperties* props
) {
    return mfxGetAttribute(runtime, mesh, kOfxMeshAttribCorner, name, props);
}

OfxStatus mfxGetFaceAttribute(
    const MfxPluginRuntime* runtime,
    OfxMeshHandle mesh,
    const char* name,
    MfxAttributeProperties* props
) {
    return mfxGetAttribute(runtime, mesh, kOfxMeshAttribFace, name, props);
}

OfxStatus mfxCopyAttribute(
    MfxAttributeProperties* destination,
    const MfxAttributeProperties* source,
    int start,
    int count
) {
    int componentCount =
        source->component_count < destination->component_count
        ? source->component_count
        : destination->component_count;

    if (source->type == destination->type)
    {
        size_t componentByteSize = 0;
        switch (source->type)
        {
        case MFX_UBYTE_ATTR:
            componentByteSize = sizeof(unsigned char);
            break;
        case MFX_INT_ATTR:
            componentByteSize = sizeof(int);
            break;
        case MFX_FLOAT_ATTR:
            componentByteSize = sizeof(float);
            break;
        default:
            printf("Error: unsupported attribute type: %d\n", source->type);
            return kOfxStatErrFatal;
        }

        for (int i = 0; i < count; ++i) {
            const void* src = (void*)(source->data + (start + i) * source->byte_stride);
            void* dst = (void*)(destination->data + (start + i) * destination->byte_stride);
            memcpy(dst, src, componentCount * componentByteSize);
        }
        return kOfxStatOK;
    }

    switch (destination->type) {
    case MFX_FLOAT_ATTR:
        switch (source->type) {
        case MFX_UBYTE_ATTR:
            for (int i = 0; i < count; ++i) {
                const unsigned char* src = (unsigned char*)(source->data + (start + i) * source->byte_stride);
                float* dst = (float*)(destination->data + (start + i) * destination->byte_stride);
                for (int k = 0; k < componentCount; ++k)
                {
                    dst[k] = (float)src[k] / 255.0f;
                }
            }
            return kOfxStatOK;
            break;
        default:
            return kOfxStatErrUnsupported;
        }
        break;
    default:
        return kOfxStatErrUnsupported;
    }
    printf("Warning: unsupported input/output type combinason in copyAttribute: %d -> %d\n", source->type, destination->type);
    return kOfxStatErrUnsupported;
}
