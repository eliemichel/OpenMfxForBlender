/*
 * Copyright 2019-2020 Elie Michel
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

/**
 * This file defines the public C API for the Open Mesh Effect Host
 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include "ofxCore.h"
#include "ofxMeshEffect.h"

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
bool ofxhost_is_identity(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance, bool *shouldCook);

#ifdef __cplusplus
}
#endif

#endif // __MFX_HOST_H__
