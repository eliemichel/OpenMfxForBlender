/*
 * Copyright 2019-2021 Elie Michel
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

/** \file
 * \ingroup openmesheffect
 *
 */

#ifndef __MFX_MESH_EFFECT_SUITE_H__
#define __MFX_MESH_EFFECT_SUITE_H__

// Mesh Effect Suite Entry Points

#include "ofxMeshEffect.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const OfxMeshEffectSuiteV1 gMeshEffectSuiteV1;

// See ofxMeshEffect.h for docstrings

OfxStatus getPropertySet(OfxMeshEffectHandle meshEffect, OfxPropertySetHandle *propHandle);
OfxStatus getParamSet(OfxMeshEffectHandle meshEffect, OfxParamSetHandle *paramSet);
OfxStatus inputDefine(OfxMeshEffectHandle meshEffect,
                      const char *name,
                      OfxMeshInputHandle *input,
                      OfxPropertySetHandle *propertySet);
OfxStatus inputGetHandle(OfxMeshEffectHandle meshEffect,
                         const char *name,
                         OfxMeshInputHandle *input,
                         OfxPropertySetHandle *propertySet);
OfxStatus inputGetPropertySet(OfxMeshInputHandle input, OfxPropertySetHandle *propHandle);
OfxStatus inputRequestAttribute(OfxMeshInputHandle input,
                                const char *attachment,
                                const char *name,
                                int componentCount,
                                const char *type,
                                const char *semantic,
                                int mandatory);
OfxStatus inputGetMesh(OfxMeshInputHandle input,
                       OfxTime time,
                       OfxMeshHandle *meshHandle,
                       OfxPropertySetHandle *propertySet);
OfxStatus inputReleaseMesh(OfxMeshHandle meshHandle);

// Future behavior: attributes will be NOT owned by default
OfxStatus attributeDefine(OfxMeshHandle meshHandle,
                          const char *attachment,
                          const char *name,
                          int componentCount,
                          const char *type,
                          const char *semantic,
                          OfxPropertySetHandle *attributeHandle);
OfxStatus meshGetAttribute(OfxMeshHandle meshHandle,
                           const char *attachment,
                           const char *name,
                           OfxPropertySetHandle *attributeHandle);
OfxStatus meshGetPropertySet(OfxMeshHandle mesh, OfxPropertySetHandle *propHandle);
OfxStatus meshAlloc(OfxMeshHandle meshHandle);
int ofxAbort(OfxMeshEffectHandle meshEffect);

#ifdef __cplusplus
}
#endif

#endif  // __MFX_MESH_EFFECT_SUITE_H__
