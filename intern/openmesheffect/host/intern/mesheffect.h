/*
 * Copyright 2019 Elie Michel
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

#ifndef __MFX_MESHEFFECT_H__
#define __MFX_MESHEFFECT_H__

#include "ofxMeshEffect.h"
#include "ofxExtras.h"

#include "properties.h"
#include "parameters.h"
#include "inputs.h"
#include "messages.h"

// Mesh Effect

typedef struct OfxMeshEffectStruct {
    OfxMeshInputSetStruct inputs;
    OfxPropertySetStruct properties;
    OfxParamSetStruct parameters;
    OfxHost *host; // weak pointer, do not deep copy

    // Only the last persistent message is stored
    OfxMessageType messageType;
    char message[1024];
} OfxMeshEffectStruct;

// OfxMeshEffectStruct

/**
 * /pre meshEffectHandle->host has been set first
 */
void init_mesh_effect(OfxMeshEffectHandle meshEffectHandle);
void free_mesh_effect(OfxMeshEffectHandle meshEffectHandle);
void deep_copy_mesh_effect(OfxMeshEffectStruct *destination,
	                       const OfxMeshEffectStruct *source);

// Mesh Effect Suite Entry Points

extern const OfxMeshEffectSuiteV1 gMeshEffectSuiteV1;

// See ofxMeshEffect.h for docstrings

OfxStatus getPropertySet(OfxMeshEffectHandle meshEffect,
                         OfxPropertySetHandle *propHandle);
OfxStatus getParamSet(OfxMeshEffectHandle meshEffect,
                      OfxParamSetHandle *paramSet);
OfxStatus inputDefine(OfxMeshEffectHandle meshEffect,
                      const char *name,
                      OfxPropertySetHandle *propertySet);
OfxStatus inputGetHandle(OfxMeshEffectHandle meshEffect,
                         const char *name,
                         OfxMeshInputHandle *input,
                         OfxPropertySetHandle *propertySet);
OfxStatus inputGetPropertySet(OfxMeshInputHandle input,
                              OfxPropertySetHandle *propHandle);
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
                          OfxPropertySetHandle *attributeHandle);
OfxStatus meshGetAttribute(OfxMeshHandle meshHandle,
                           const char *attachment,
                           const char *name,
                           OfxPropertySetHandle *attributeHandle);
OfxStatus meshGetPropertySet(OfxMeshHandle mesh,
                             OfxPropertySetHandle *propHandle);
OfxStatus meshAlloc(OfxMeshHandle meshHandle);
int ofxAbort(OfxMeshEffectHandle meshEffect);

#endif // __MFX_MESHEFFECT_H__
