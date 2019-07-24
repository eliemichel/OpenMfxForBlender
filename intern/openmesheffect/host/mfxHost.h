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

#include <stdbool.h>
#include "ofxCore.h"
#include "ofxMeshEffect.h"

#include "intern/properties.h"
#include "intern/parameters.h"
#include "intern/inputs.h"
#include "intern/mesheffect.h"

OfxHost * getGlobalHost(void);
void releaseGlobalHost(void);

// Steps of use_plugin
bool ofxhost_load_plugin(OfxHost *host, OfxPlugin *plugin);
void ofxhost_unload_plugin(OfxPlugin *plugin);
bool ofxhost_get_descriptor(OfxHost *host, OfxPlugin *plugin, OfxMeshEffectHandle *effectDescriptor);
void ofxhost_release_descriptor(OfxMeshEffectHandle effectDescriptor);
bool ofxhost_create_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectDescriptor, OfxMeshEffectHandle *effectInstance);
void ofxhost_destroy_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance);
bool ofxhost_cook(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance);

#include "mfxPluginRegistry.h"
// TODO: use_plugin might be dropped from API
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
