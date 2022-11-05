#include "mesh.h"
#include <OpenMfx/Sdk/C/Common> // for MFX_ENSURE

#define ENSURE(XXX) MFX_ENSURE(runtime->XXX)

OfxStatus mfxGetMeshProperties(
    const MfxPluginRuntime *runtime,
    const OfxMeshHandle mesh,
    MfxMeshProperties *props)
{
    OfxPropertySetHandle propHandle;
    ENSURE(meshEffectSuite->meshGetPropertySet(mesh, &propHandle));
    ENSURE(propertySuite->propGetInt(propHandle, kOfxMeshPropPointCount, 0, &props->point_count));
    ENSURE(propertySuite->propGetInt(propHandle, kOfxMeshPropCornerCount, 0, &props->corner_count));
    ENSURE(propertySuite->propGetInt(propHandle, kOfxMeshPropFaceCount, 0, &props->face_count));
    ENSURE(propertySuite->propGetInt(propHandle, kOfxMeshPropConstantFaceSize, 0, &props->constant_face_size));
    return kOfxStatOK;
}

OfxStatus mfxSetMeshProperties(
    const MfxPluginRuntime *runtime,
    OfxMeshHandle mesh,
    const MfxMeshProperties *props)
{
    OfxPropertySetHandle propHandle;
    ENSURE(meshEffectSuite->meshGetPropertySet(mesh, &propHandle));
    ENSURE(propertySuite->propSetInt(propHandle, kOfxMeshPropPointCount, 0, props->point_count));
    ENSURE(propertySuite->propSetInt(propHandle, kOfxMeshPropCornerCount, 0, props->corner_count));
    ENSURE(propertySuite->propSetInt(propHandle, kOfxMeshPropFaceCount, 0, props->face_count));
    ENSURE(propertySuite->propSetInt(propHandle, kOfxMeshPropConstantFaceSize, 0, props->constant_face_size));
    return kOfxStatOK;
}
