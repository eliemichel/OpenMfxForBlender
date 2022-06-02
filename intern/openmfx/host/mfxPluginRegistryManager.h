/*
 * Copyright 2019-2022 Elie Michel
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
 * There is one PluginRegistry per .ofx file, and the PluginRegistryPool
 * ensures that the same file is not loaded twice.
 */

#ifndef __MFX_PLUGIN_REGISTRY_POOL_H__
#define __MFX_PLUGIN_REGISTRY_POOL_H__

#include <stdbool.h>

#include "mfxPluginRegistry.h"

/**
 * This file exposes the 2 key methods of the PluginRegistryPool singleton to C files.
 * XXX This is more or less obsolete as no C code uses it, but could still turn out to
 * be useful one day (why not removing it in the meantime?)
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Access the global plugin registry pool. There is one registry per ofx file,
 * and this pool ensures that the same registry is not loaded twice.
 * For each call to get_registry, a call to release_registry must be issued eventually
 */
PluginRegistry *get_registry(const char *ofx_filepath);

void release_registry(const PluginRegistry *registry);

#ifdef __cplusplus
}
#endif

#endif // __MFX_PLUGIN_REGISTRY_POOL_H__
