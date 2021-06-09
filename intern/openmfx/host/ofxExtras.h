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

typedef OfxStatus (*BeforeMeshReleaseCbFunc)(OfxHost*, OfxMeshHandle);

/**
 * Custom callback called when getting mesh data, converting the internal data
 * to the host mesh data. Internal data is typically got from mesh property
 * kOfxMeshPropInternalData.
 *
 * Callback signature must be:
 *   OfxStatus callback(OfxHost *host, OfxPropertySetHandle meshHandle);
 * (type BeforeMeshGetCbFunc)
 */
#define kOfxHostPropBeforeMeshGetCb "OfxHostPropBeforeMeshGetCb"

typedef OfxStatus (*BeforeMeshGetCbFunc)(OfxHost*, OfxMeshHandle);

/**
 * Internal property on attributes that are used to store attribute requests
 */
#define kMeshAttribRequestPropMandatory "MeshAttribRequestPropMandatory"
