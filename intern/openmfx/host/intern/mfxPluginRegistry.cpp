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

#include <string.h>
#include <stdio.h>

#include "ofxMeshEffect.h"

#include "util/memory_util.h"
#include "util/path_util.h"

#include "mfxPluginRegistry.h"

/**
 * Initialize a plugin registry before anything else.
 */
static void registry_init(PluginRegistry *registry) {
  registry->num_plugins = 0;
  registry->plugins = NULL;
  registry->status = NULL;
  registry->handle = NULL;
  registry->getNumberOfPlugins = NULL;
  registry->getPlugin = NULL;
  registry->setBundleDirectory = NULL;
}

/**
 * Initialize a plugin registry provided that the procedure have been loaded
 * correctly.
 */
static void registry_init_plugins(PluginRegistry *registry) {
  int i, n;
  n = registry->getNumberOfPlugins();
  printf("Found %d plugins.\n", n);

  if (n > 0) {
    registry->plugins = (OfxPlugin **)malloc_array(sizeof(OfxPlugin *), n, "mfx plugins");
    registry->status = (OfxPluginStatus*)malloc_array(sizeof(OfxPluginStatus), n, "mfx plugins status");
  }

  // 1. Count number of supported plugins
  registry->num_plugins = 0;
  for (i = 0 ; i < n ; ++i) {
    OfxPlugin *plugin;
    plugin = registry->getPlugin(i);
    printf("Plugin #%d: %s (API %s, version %d)\n", i, plugin->pluginIdentifier, plugin->pluginApi, plugin->apiVersion);

    // API/Version check
    if (0 != strcmp(plugin->pluginApi, kOfxMeshEffectPluginApi)) {
      printf("Unsupported plugin API: %s (expected %s)", plugin->pluginApi, kOfxMeshEffectPluginApi);
      continue;
    }
    if (plugin->apiVersion != kOfxMeshEffectPluginApiVersion) {
      printf("Plugin API version mismatch: %d found, but %d expected", plugin->apiVersion, kOfxMeshEffectPluginApiVersion);
      continue;
    }

    printf("Plugin #%d in binary is #%d in plugin registry\n", i, registry->num_plugins);
    registry->plugins[registry->num_plugins] = plugin;
    registry->status[registry->num_plugins] = OfxPluginStatNotLoaded;
    ++registry->num_plugins;
  }

  printf("Found %d supported plugins.\n", registry->num_plugins);

  // 2. Resize plug-in array to remove unused cells at the end
  if (registry->num_plugins > 0) {
    OfxPlugin **old_plugins = registry->plugins;
    OfxPluginStatus *old_status = registry->status;
    registry->plugins = (OfxPlugin**)malloc_array(sizeof(OfxPlugin*), registry->num_plugins, "mfx plugins");
    registry->status = (OfxPluginStatus*)malloc_array(sizeof(OfxPluginStatus), registry->num_plugins, "mfx plugins status");
    for (i = 0 ; i < registry->num_plugins ; ++i) {
      registry->plugins[i] = old_plugins[i];
      registry->status[i] = old_status[i];
    }
    free_array(old_plugins);
    free_array(old_status);
  }
}

/**
 * Initialize in a plugin registry the attributes related to binary loading.
 */
static bool registry_init_binary(PluginRegistry *registry, const char *ofx_filepath) {
  // Open ofx binary
  registry->handle = binary_open(ofx_filepath);
  if (NULL == registry->handle) {
    return false;
  }

  // Get procedures
  registry->getNumberOfPlugins =
    (OfxGetNumberOfPluginsFunc)binary_get_proc(registry->handle, "OfxGetNumberOfPlugins");
  registry->getPlugin =
    (OfxGetPluginFunc)binary_get_proc(registry->handle, "OfxGetPlugin");
  registry->setBundleDirectory =
    (OfxSetBundleDirectoryFunc)binary_get_proc(registry->handle, "OfxSetBundleDirectory");

  return NULL != registry->getNumberOfPlugins && NULL != registry->getPlugin;
}

bool load_registry(PluginRegistry *registry, const char *ofx_filepath) {
  printf("Loading OFX plug-ins from %s.\n", ofx_filepath);

  registry_init(registry);

  if (false == registry_init_binary(registry, ofx_filepath)) {
    printf("Could not init binary.\n");
    free_registry(registry);
    return false;
  }

  if (NULL != registry->setBundleDirectory) {
    char *bundle_directory = (char*)malloc_array(sizeof(char), strlen(ofx_filepath) + 1, "bundle directory");
    strcpy(bundle_directory, ofx_filepath);
    *strrchr(bundle_directory, PATH_DIR_SEP) = '\0';
    registry->setBundleDirectory(bundle_directory);
    free_array(bundle_directory);
  }

  registry_init_plugins(registry);

  return true;
}

void free_registry(PluginRegistry *registry) {
  registry->num_plugins = 0;
  if (NULL != registry->plugins) {
    free_array(registry->plugins);
    registry->plugins = NULL;
  }
  if (NULL != registry->status) {
    free_array(registry->status);
    registry->status = NULL;
  }
  if (NULL != registry->handle) {
    binary_close(registry->handle);
    registry->handle = NULL;
  }
}
