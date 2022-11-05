#ifndef __MFX_SDK_PLUGIN_MESH__
#define __MFX_SDK_PLUGIN_MESH__

#include "runtime.h"

#include <ofxCore.h>
#include <ofxMeshEffect.h>
#include <ofxProperty.h>

typedef struct MfxMeshProperties {
  int point_count;
  int corner_count;
  int face_count;
  int constant_face_size;
} MfxMeshProperties;

OfxStatus mfxGetMeshProperties(
    const MfxPluginRuntime *runtime,
    const OfxMeshHandle mesh,
    MfxMeshProperties *props);

OfxStatus mfxSetMeshProperties(
    const MfxPluginRuntime *runtime,
    OfxMeshHandle mesh,
    const MfxMeshProperties *props);

#endif // __MFX_SDK_PLUGIN_MESH__
