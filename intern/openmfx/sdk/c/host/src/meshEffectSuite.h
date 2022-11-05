#ifndef _meshEffectSuite_h_
#define _meshEffectSuite_h_

/*****************************************************************************/
/* Mesh Effect Suite */

#include "types.h"

#include <ofxCore.h>
#include <ofxMeshEffect.h>

OfxStatus defaultAttributesDefine(OfxMeshHandle mesh);

OfxStatus getParamSet(OfxMeshEffectHandle meshEffect,
                      OfxParamSetHandle *paramSet);

OfxStatus inputDefine(OfxMeshEffectHandle meshEffect,
                      const char *name,
                      OfxMeshInputHandle *inputHandle,
                      OfxPropertySetHandle *propertySet);

OfxStatus inputGetHandle(OfxMeshEffectHandle meshEffect,
                         const char *name,
                         OfxMeshInputHandle *inputHandle,
                         OfxPropertySetHandle *propertySet);

OfxStatus inputGetPropertySet(OfxMeshInputHandle input,
                              OfxPropertySetHandle *propertySet);

OfxStatus inputGetMesh(OfxMeshInputHandle input,
                       OfxTime time,
                       OfxMeshHandle *meshHandle,
                       OfxPropertySetHandle *propertySet);

OfxStatus inputReleaseMesh(OfxMeshHandle meshHandle);

OfxStatus meshGetAttributeByIndex(OfxMeshHandle meshHandle,
                                  int index,
                                  OfxPropertySetHandle *attributeHandle);

OfxStatus meshGetAttribute(OfxMeshHandle meshHandle,
                           const char *attachment,
                           const char *name,
                           OfxPropertySetHandle *attributeHandle);

OfxStatus attributeDefine(OfxMeshHandle meshHandle,
                          const char *attachment,
                          const char *name,
                          int componentCount,
                          const char *type,
                          const char *semantic,
                          OfxPropertySetHandle *attributeHandle);

OfxStatus meshGetPropertySet(OfxMeshHandle mesh,
                             OfxPropertySetHandle *propertySet);

OfxStatus meshAlloc(OfxMeshHandle meshHandle);

OfxStatus defaultAttributesDefine(OfxMeshHandle mesh);

extern const OfxMeshEffectSuiteV1 meshEffectSuiteV1;

#endif // _meshEffectSuite_h_
