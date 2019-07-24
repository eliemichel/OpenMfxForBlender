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

#include "properties.h"
#include "parameters.h"
#include "inputs.h"

// OpenFX Internal Extensions

/**
 * Implementation specific extensions to OpenFX Mesh Effect API.
 * These MUST NOT be used by plugins, but are here for communication between
 * the core host and Blender-specific code (in mfxModifier). This is among
 * others a way to keep this part of the code under the Apache 2 license while
 * mfxModifier must be released under GPL.
 */

/**
 * Blind pointer to some internal data representation that is not cleared on
 * mesh release, e.g. pointer to a Mesh object.
 */
#define kOfxMeshPropInternalData "OfxMeshPropInternalData"
/**
 * Pointer to current ofx host
 */
#define kOfxMeshPropHostHandle "OfxMeshPropHostHandle"

/**
 * Custom callback called before releasing mesh data, converting the host mesh
 * data into some internal representation, typically stored into mesh property
 * kOfxMeshPropInternalData.
 *
 * Callback signature must be:
 *   OfxStatus callback(OfxHost *host, OfxPropertySetHandle meshHandle);
 * (type BeforeMeshReleaseCbFunc)
 */
#define kOfxHostPropBeforeMeshReleaseCb "OfxHostPropBeforeMeshReleaseCb"

typedef OfxStatus (*BeforeMeshReleaseCbFunc)(OfxHost*, OfxPropertySetHandle);


// Mesh Effect

typedef struct OfxMeshEffectStruct {
    OfxMeshInputSetStruct inputs;
    OfxPropertySetStruct properties;
    OfxParamSetStruct parameters;
    OfxHost *host; // weak pointer, do not deep copy
} OfxMeshEffectStruct;

// OfxMeshEffectStruct

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
                       OfxPropertySetHandle *meshHandle);
OfxStatus inputReleaseMesh(OfxPropertySetHandle meshHandle);
OfxStatus meshAlloc(OfxPropertySetHandle meshHandle,
                    int pointCount,
                    int vertexCount,
                    int faceCount);
int ofxAbort(OfxMeshEffectHandle meshEffect);

#endif // __MFX_MESHEFFECT_H__
