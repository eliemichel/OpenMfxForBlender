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
 */

#ifndef __MFX_PLUGIN_REGISTRY_H__
#define __MFX_PLUGIN_REGISTRY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>

#include "ofxCore.h"

#include "util/binary_util.h"

typedef enum OfxPluginStatus {
    OfxPluginStatOK,
    OfxPluginStatNotLoaded,
    OfxPluginStatError
} OfxPluginStatus;

typedef void (*OfxSetBundleDirectoryFunc)(const char *path);
typedef int (*OfxGetNumberOfPluginsFunc)(void);
typedef OfxPlugin *(*OfxGetPluginFunc)(int nth);

/**
 * The plug-in registry holds all the data about the plug-ins made available by
 * a given ofx plug-in binary. A binary might contain many plug-ins.
 */
typedef struct PluginRegistry {
    BinaryHandle handle; // handle of the binary library, to be closed
    OfxGetNumberOfPluginsFunc getNumberOfPlugins;
    OfxGetPluginFunc getPlugin;
    OfxSetBundleDirectoryFunc setBundleDirectory;
    int num_plugins;
    OfxPlugin **plugins;
    OfxPluginStatus *status;
} PluginRegistry;

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

#ifdef __cplusplus
}
#endif

#endif // __MFX_PLUGIN_REGISTRY_H__
