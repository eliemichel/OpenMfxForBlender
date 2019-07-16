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
 * This is an implementation of an OpenFX host specialized toward the Mesh
 * Effect API (rather than the Image Effect API like most OpenFX host
 * implementations are.)
 */

#ifndef __MFX_HOST_H__
#define __MFX_HOST_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#include <windows.h>
#endif

#include <stdbool.h>
#include "ofxCore.h"
#include "ofxMeshEffect.h"

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

// Plugin registry

typedef enum OfxPluginStatus {
    OfxPluginStatOK,
    OfxPluginStatNotLoaded,
    OfxPluginStatError
} OfxPluginStatus;

typedef struct PluginRegistry {
#ifdef _WIN32
    HINSTANCE hinstance;
#else
    void *handle; // handle of the binary library, to be closed
#endif
    int num_plugins;
    OfxPlugin **plugins;
    OfxPluginStatus *status;
} PluginRegistry;

// Properties

typedef union OfxPropertyValueStruct {
    void *as_pointer;
    const char *as_const_char;
    char *as_char;
    double as_double;
    int as_int;
} OfxPropertyValueStruct;

typedef struct OfxPropertyStruct {
    const char *name;
    OfxPropertyValueStruct value[4];
} OfxPropertyStruct;

typedef enum PropertySetContext {
    PROP_CTX_HOST,
    PROP_CTX_MESH_EFFECT,
    PROP_CTX_INPUT,
    PROP_CTX_MESH,
    PROP_CTX_OTHER
} PropertySetContext;

typedef struct OfxPropertySetStruct {
    PropertySetContext context; // TODO: use this rather than generic property set objects
    int num_properties;
    OfxPropertyStruct **properties;
} OfxPropertySetStruct;

// Parameters

typedef union OfxParamValueStruct {
    void *as_pointer;
    const char *as_const_char;
    char *as_char;
    double as_double;
    int as_int;
} OfxParamValueStruct;

typedef enum ParamType {
    PARAM_TYPE_UNKNOWN = -1,
    PARAM_TYPE_DOUBLE,
    PARAM_TYPE_INT,
    PARAM_TYPE_STRING,
} ParamType;

typedef struct OfxParamStruct {
    const char *name;
    OfxParamValueStruct value[4];
    ParamType type;
    OfxPropertySetStruct properties;
} OfxParamStruct;

typedef struct OfxParamSetStruct {
    int num_parameters;
    OfxParamStruct **parameters;
    OfxPropertySetStruct *effect_properties; // weak pointer
} OfxParamSetStruct;

void parameter_set_type(OfxParamHandle param, ParamType type);
void parameter_realloc_string(OfxParamHandle param, int size);

// Inputs

typedef struct OfxMeshInputStruct {
    const char *name;
    OfxPropertySetStruct properties;
    OfxPropertySetStruct mesh;
    OfxHost *host; // weak pointer, do not deep copy
} OfxMeshInputStruct;

typedef struct OfxMeshInputSetStruct {
    int num_inputs;
    OfxMeshInputStruct **inputs;
    OfxHost *host; // weak pointer, do not deep copy
} OfxMeshInputSetStruct;

// Mesh Effect

typedef struct OfxMeshEffectStruct {
    OfxMeshInputSetStruct inputs;
    OfxPropertySetStruct properties;
    OfxParamSetStruct parameters;
    OfxHost *host; // weak pointer, do not deep copy
} OfxMeshEffectStruct;

OfxHost * getGlobalHost(void);
void releaseGlobalHost(void);

/**
 * /pre registry has never been allocated
 * /post if true is returned, registry is allocated and filled with valid
 *       OfxPlugin pointers. Registry must be later released using
 *       free_registry()
 */
bool load_registry(PluginRegistry *registry, const char *ofx_filepath);

/**
 * /pre registry has been allocated
 * /post registry will never be used again
 */
void free_registry(PluginRegistry *registry);

// Steps of use_plugin
bool ofxhost_load_plugin(OfxHost *host, OfxPlugin *plugin);
void ofxhost_unload_plugin(OfxPlugin *plugin);
bool ofxhost_get_descriptor(OfxHost *host, OfxPlugin *plugin, OfxMeshEffectHandle *effectDescriptor);
void ofxhost_release_descriptor(OfxMeshEffectHandle effectDescriptor);
bool ofxhost_create_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectDescriptor, OfxMeshEffectHandle *effectInstance);
void ofxhost_destroy_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance);
bool ofxhost_cook(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance);

/**
 * Flag the plugin as beeing used eventually and perform initial loading
 * /pre plugins have been loaded into the registry using load_plugins_linux()
 *      plugin_index is between 0 included and registry->num_plugins excluded
 * /post use_plugin() will not be called again for this plugin
 */
bool use_plugin(const PluginRegistry *registry, int plugin_index);

#ifdef __cplusplus
}
#endif

#endif // __MFX_HOST_H__
