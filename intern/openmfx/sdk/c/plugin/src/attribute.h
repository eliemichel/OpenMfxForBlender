#ifndef __MFX_SDK_PLUGIN_ATTRIBUTE__
#define __MFX_SDK_PLUGIN_ATTRIBUTE__

#include "runtime.h"

#include <ofxCore.h>
#include <ofxMeshEffect.h>
#include <ofxProperty.h>

typedef enum MfxAttributeType {
    MFX_UNKNOWN_ATTR = -1,
    MFX_UBYTE_ATTR,
    MFX_INT_ATTR,
    MFX_FLOAT_ATTR,
} MfxAttributeType;

typedef struct MfxAttributeProperties {
  int component_count;
  MfxAttributeType type;
  char *semantic;
  char *data;
  size_t byte_stride;
  int is_owner;
} MfxAttributeProperties;

/**
 * Convert a type string from MeshEffect API to its local enum counterpart
 */
MfxAttributeType mfxAttributeTypeAsEnum(const char* attr_type);

/**
 * Convert a type from enum to the MeshEffect API string
 */
const char* mfxAttributeTypeAsString(MfxAttributeType type);

/**
 * Get attribute properties from an attribute handle
 */
OfxStatus mfxGetAttributeProperties(
    const MfxPluginRuntime *runtime,
    const OfxPropertySetHandle attrib,
    MfxAttributeProperties *props);

/**
 * Set attribute properties using an attribute handle
 */
OfxStatus mfxSetAttributeProperties(
    const MfxPluginRuntime* runtime,
    OfxPropertySetHandle attrib,
    const MfxAttributeProperties *props);

/**
 * Get attribute properties from a name and an attachment
 */
OfxStatus mfxGetAttribute(
    const MfxPluginRuntime* runtime,
    OfxMeshHandle mesh,
    const char* attachment,
    const char* name,
    MfxAttributeProperties* props
);

/**
 * Get point attribute properties from its name (short for mfxGetAttribute)
 */
OfxStatus mfxGetPointAttribute(
    const MfxPluginRuntime* runtime,
    OfxMeshHandle mesh,
    const char* name,
    MfxAttributeProperties* props
);

/**
 * Get corner attribute properties from its name (short for mfxGetAttribute)
 */
OfxStatus mfxGetCornerAttribute(
    const MfxPluginRuntime* runtime,
    OfxMeshHandle mesh,
    const char* name,
    MfxAttributeProperties* props
);

/**
 * Get face attribute properties from its name (short for mfxGetAttribute)
 */
OfxStatus mfxGetFaceAttribute(
    const MfxPluginRuntime* runtime,
    OfxMeshHandle mesh,
    const char* name,
    MfxAttributeProperties* props
);

/**
 * Copy attribute and try to cast. If number of component is different, copy the common components
 * only.
 */
OfxStatus mfxCopyAttribute(
    MfxAttributeProperties* destination,
    const MfxAttributeProperties* source,
    int start,
    int count
);

#endif // __MFX_SDK_PLUGIN_ATTRIBUTE__
