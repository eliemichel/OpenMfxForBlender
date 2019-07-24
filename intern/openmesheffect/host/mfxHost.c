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
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "ofxProperty.h"
#include "ofxParam.h"
#include "ofxMeshEffect.h"

#include "util/ofx_util.h"
#include "util/memory_util.h"

#include "mfxHost.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif


// OFX SUITES MAIN

static const void * fetchSuite(OfxPropertySetHandle host,
                               const char *suiteName,
                               int suiteVersion) {

  static const OfxMeshEffectSuiteV1 meshEffectSuiteV1 = {
    /* getPropertySet */      getPropertySet,
    /* getParamSet */         getParamSet,
    /* inputDefine */         inputDefine,
    /* inputGetHandle */      inputGetHandle,
    /* inputGetPropertySet */ inputGetPropertySet,
    /* inputGetMesh */        inputGetMesh,
    /* inputReleaseMesh */    inputReleaseMesh,
    /* meshAlloc */           meshAlloc,
    /* abort */               ofxAbort
  };

  static const OfxParameterSuiteV1 parameterSuiteV1 = {
    /* paramDefine */            paramDefine,
    /* paramGetHandle */         paramGetHandle,
    /* paramSetGetPropertySet */ paramSetGetPropertySet,
    /* paramGetPropertySet */    paramGetPropertySet,
    /* paramGetValue */          paramGetValue,
    /* paramGetValueAtTime */    paramGetValueAtTime,
    /* paramGetDerivative */     paramGetDerivative,
    /* paramGetIntegral */       paramGetIntegral,
    /* paramSetValue */          paramSetValue,
    /* paramSetValueAtTime */    paramSetValueAtTime,
    /* paramGetNumKeys */        paramGetNumKeys,
    /* paramGetKeyTime */        paramGetKeyTime,
    /* paramGetKeyIndex */       paramGetKeyIndex,
    /* paramDeleteKey */         paramDeleteKey,
    /* paramDeleteAllKeys */     paramDeleteAllKeys,
    /* paramCopy */              paramCopy,
    /* paramEditBegin */         paramEditBegin,
    /* paramEditEnd */           paramEditEnd
  };

  static const OfxPropertySuiteV1 propertySuiteV1 = {
    /* propSetPointer */   propSetPointer,
    /* propSetString */    propSetString,
    /* propSetDouble */    propSetDouble,
    /* propSetInt */       propSetInt,
    /* propSetPointerN */  propSetPointerN,
    /* propSetStringN */   propSetStringN,
    /* propSetDoubleN */   propSetDoubleN,
    /* propSetIntN */      propSetIntN,
    /* propGetPointer */   propGetPointer,
    /* propGetString */    propGetString,
    /* propGetDouble */    propGetDouble,
    /* propGetInt */       propGetInt,
    /* propGetPointerN */  propGetPointerN,
    /* propGetStringN */   propGetStringN,
    /* propGetDoubleN */   propGetDoubleN,
    /* propGetIntN */      propGetIntN,
    /* propReset */        propReset,
    /* propGetDimension */ propGetDimension
  };

  (void)host; // TODO: check host?

  if (0 == strcmp(suiteName, kOfxMeshEffectSuite) && suiteVersion == 1) {
    return &meshEffectSuiteV1;
  }
  if (0 == strcmp(suiteName, kOfxParameterSuite) && suiteVersion == 1) {
    return &parameterSuiteV1;
  }
  if (0 == strcmp(suiteName, kOfxPropertySuite) && suiteVersion == 1) {
    return &propertySuiteV1;
  }
  return NULL;
}

// OFX MESH EFFECT HOST

typedef void (*OfxSetBundleDirectoryFunc)(const char *path);
typedef int (*OfxGetNumberOfPluginsFunc)(void);
typedef OfxPlugin *(*OfxGetPluginFunc)(int nth);

#ifdef _WIN32
LPVOID getLastErrorMessage() {
  LPVOID msg;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&msg,
    0, NULL);
  return msg;
}
#endif // _WIN32

bool load_registry(PluginRegistry *registry, const char *ofx_filepath) {
#ifdef _WIN32
  FARPROC proc;
#else
  void *proc;
#endif
  OfxGetNumberOfPluginsFunc fOfxGetNumberOfPlugins;
  OfxGetPluginFunc fOfxGetPlugin;
  OfxSetBundleDirectoryFunc fOfxSetBundleDirectory;

  // Init registry
  registry->num_plugins = 0;
  registry->plugins = NULL;
  registry->status = NULL;
#ifdef _WIN32
  registry->hinstance = NULL; 
#else
  registry->handle = NULL;
#endif
  
  // Open ofx binary
#ifdef _WIN32
  registry->hinstance = LoadLibrary(TEXT(ofx_filepath));
  if (NULL == registry->hinstance) {
    LPVOID msg = getLastErrorMessage();
    printf("mfxHost: Unable to load plugin binary at path %s. LoadLibrary returned: %s\n", ofx_filepath, msg);
    LocalFree(msg);
    return false;
  }
#else
  registry->handle = dlopen(ofx_filepath, RTLD_LAZY | RTLD_LOCAL);
  if (NULL == registry->handle) {
    printf("mfxHost: Unable to load plugin binary at path %s. dlopen returned: %s\n", ofx_filepath, dlerror());
    return false;
  }
#endif

#ifdef _WIN32
  proc = GetProcAddress(registry->hinstance, "OfxGetNumberOfPlugins");
#else
  proc = dlsym(registry->handle, "OfxGetNumberOfPlugins");
#endif
  fOfxGetNumberOfPlugins = (OfxGetNumberOfPluginsFunc)proc;

  if (NULL == proc) {
    printf("mfxHost: Unable to load symbol 'OfxGetNumberOfPlugins' from %s. ", ofx_filepath);
#ifdef _WIN32
    LPVOID msg = getLastErrorMessage();
    printf("GetProcAddress returned: %s\n", msg);
    LocalFree(msg);
#else
    printf("dlsym returned: %s\n", dlerror());
#endif
    free_registry(registry);
    return false;
  }

#ifdef _WIN32
  proc = GetProcAddress(registry->hinstance, "OfxGetPlugin");
#else
  proc = dlsym(registry->handle, "OfxGetPlugin");
#endif
  fOfxGetPlugin = (OfxGetPluginFunc)proc;

  if (NULL == proc) {
    printf("mfxHost: Unable to load symbol 'OfxGetPlugin' from %s. ", ofx_filepath);
#ifdef _WIN32
    LPVOID msg = getLastErrorMessage();
    printf("GetProcAddress returned: %s\n", msg);
    LocalFree(msg);
#else
    printf("dlsym returned: %s\n", dlerror());
#endif
    free_registry(registry);
    return false;
  }
  
#ifdef _WIN32
  proc = GetProcAddress(registry->hinstance, "OfxSetBundleDirectory");
#else
  proc = dlsym(registry->handle, "OfxSetBundleDirectory");
#endif
  fOfxSetBundleDirectory = (OfxSetBundleDirectoryFunc)proc;
  if (NULL == proc) {
    printf("mfxHost: Unable to load symbol 'OfxSetBundleDirectory' from %s. ", ofx_filepath);
#ifdef _WIN32
    LPVOID msg = getLastErrorMessage();
    printf("GetProcAddress returned: %s\n", msg);
    LocalFree(msg);
#else
    printf("dlsym returned: %s\n", dlerror());
#endif
  }

  {
    if (NULL != fOfxSetBundleDirectory) {
      char *bundle_directory = malloc_array(sizeof(char), strlen(ofx_filepath) + 1, "bundle directory");
      strcpy(bundle_directory, ofx_filepath);
      *strrchr(bundle_directory, '\\') = '\0'; // TODO: Forward slash
      fOfxSetBundleDirectory(bundle_directory);
    }
  }

  {
    int i, j, n;
    n = fOfxGetNumberOfPlugins();
    printf("Found %d plugins in %s.\n", n, ofx_filepath);

    // 1. Count number of supported plugins
    registry->num_plugins = 0;
    for (i = 0 ; i < n ; ++i) {
      OfxPlugin *plugin;
      plugin = fOfxGetPlugin(i);
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

      ++registry->num_plugins;
    }

    printf("Found %d supported plugins in %s.\n", registry->num_plugins, ofx_filepath);

    if (registry->num_plugins > 0) {
      registry->plugins = malloc_array(sizeof(OfxPlugin*), registry->num_plugins, "mfx plugins");
      registry->status = malloc_array(sizeof(OfxPluginStatus), registry->num_plugins, "mfx plugins status");
    }

    for (i = 0, j = 0 ; i < n ; ++i) {
      OfxPlugin *plugin;
      plugin = fOfxGetPlugin(i);
      
      if (0 != strcmp(plugin->pluginApi, kOfxMeshEffectPluginApi)
        || plugin->apiVersion != kOfxMeshEffectPluginApiVersion) {
        continue;
      }
      
      printf("Plugin #%d in binary is #%d in plugin registry\n", i, j);
      registry->plugins[j] = plugin;
      registry->status[j] = OfxPluginStatNotLoaded; // will be loaded later on when needed
      ++j;
    }
  }

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
#ifdef _WIN32
  if (NULL != registry->hinstance) {
    FreeLibrary(registry->hinstance);
    registry->hinstance = NULL;
  }
#else
  if (NULL != registry->handle) {
    dlclose(registry->handle);
    registry->handle = NULL;
  }
#endif
}

OfxHost *gHost = NULL;
int gHostUse = 0;

OfxHost * getGlobalHost(void) {
  if (0 == gHostUse) {
    gHost = malloc_array(sizeof(OfxHost), 1, "global host");
    OfxPropertySetHandle hostProperties = malloc_array(sizeof(OfxPropertySetStruct), 1, "global host properties");
    init_properties(hostProperties);
    hostProperties->context = PROP_CTX_HOST;
    propSetPointer(hostProperties, kOfxHostPropBeforeMeshReleaseCb, 0, (void*)NULL);
    gHost->host = hostProperties;
    gHost->fetchSuite = fetchSuite;
  }
  ++gHostUse;
  return gHost;
}

void releaseGlobalHost(void) {
  if (--gHostUse == 0) {
    free_array(gHost->host);
    free_array(gHost);
    gHost = NULL;
  }
}

bool ofxhost_load_plugin(OfxHost *host, OfxPlugin *plugin) {
  OfxStatus status;

  plugin->setHost(host);

  status = plugin->mainEntry(kOfxActionLoad, NULL, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionLoad, status, getOfxStateName(status));

  if (kOfxStatReplyDefault == status) {
    printf("WARNING: The plugin '%s' ignored load action.\n", plugin->pluginIdentifier);
  }
  if (kOfxStatFailed == status) {
    printf("ERROR: The load action failed, no further actions will be passed to the plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while loading the plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  return true;
}

void ofxhost_unload_plugin(OfxPlugin *plugin) {
  OfxStatus status;
  
  status = plugin->mainEntry(kOfxActionUnload, NULL, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionUnload, status, getOfxStateName(status));

  if (kOfxStatReplyDefault == status) {
    printf("WARNING: The plugin '%s' ignored unload action.\n", plugin->pluginIdentifier);
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while unloading the plug-in '%s'.\n", plugin->pluginIdentifier);
  }

  plugin->setHost(NULL);
}

bool ofxhost_get_descriptor(OfxHost *host, OfxPlugin *plugin, OfxMeshEffectHandle *effectDescriptor) {
  OfxStatus status;
  OfxMeshEffectHandle effectHandle;

  *effectDescriptor = NULL;
  effectHandle = malloc_array(sizeof(OfxMeshEffectStruct), 1, "mesh effect descriptor");

  effectHandle->host = host;
  init_mesh_effect(effectHandle);

  status = plugin->mainEntry(kOfxActionDescribe, effectHandle, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionDescribe, status, getOfxStateName(status));

  if (kOfxStatErrMissingHostFeature == status) {
    printf("ERROR: The plugin '%s' lacks some host feature.\n", plugin->pluginIdentifier); // see message
    return false;
  }
  if (kOfxStatErrMemory == status) {
    printf("ERROR: Not enough memory for plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  if (kOfxStatFailed == status) {
    printf("ERROR: Error while describing plug-in '%s'.\n", plugin->pluginIdentifier); // see message
    return false;
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while describing plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }

  *effectDescriptor = effectHandle;

  return true;
}

void ofxhost_release_descriptor(OfxMeshEffectHandle effectDescriptor) {
  free_mesh_effect(effectDescriptor);
  free_array(effectDescriptor);
}

bool ofxhost_create_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectDescriptor, OfxMeshEffectHandle *effectInstance) {
  OfxStatus status;
  OfxMeshEffectHandle instance;

  *effectInstance = NULL;

  instance = malloc_array(sizeof(OfxMeshEffectStruct), 1, "mesh effect descriptor");
  deep_copy_mesh_effect(instance, effectDescriptor);

  status = plugin->mainEntry(kOfxActionCreateInstance, instance, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionCreateInstance, status, getOfxStateName(status));

  if (kOfxStatErrMemory == status) {
    printf("ERROR: Not enough memory for plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  if (kOfxStatFailed == status) {
    printf("ERROR: Error while creating an instance of plug-in '%s'.\n", plugin->pluginIdentifier); // see message
    return false;
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while creating an instance of plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }

  *effectInstance = instance;

  return true;
}

void ofxhost_destroy_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance) {
  OfxStatus status;

  status = plugin->mainEntry(kOfxActionDestroyInstance, effectInstance, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionDestroyInstance, status, getOfxStateName(status));

  if (kOfxStatFailed == status) {
    printf("ERROR: Error while destroying an instance of plug-in '%s'.\n", plugin->pluginIdentifier); // see message
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while destroying an instance of plug-in '%s'.\n", plugin->pluginIdentifier);
  }

  free_mesh_effect(effectInstance);
  free_array(effectInstance);
}

bool ofxhost_cook(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance) {
  OfxStatus status;

  status = plugin->mainEntry(kOfxMeshEffectActionCook, effectInstance, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxMeshEffectActionCook, status, getOfxStateName(status));

  if (kOfxStatErrMemory == status) {
    printf("ERROR: Not enough memory for plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  if (kOfxStatFailed == status) {
    printf("ERROR: Error while cooking an instance of plug-in '%s'.\n", plugin->pluginIdentifier); // see message
    return false;
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while cooking an instance of plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  return true;
}

bool use_plugin(const PluginRegistry *registry, int plugin_index) {
  OfxPlugin *plugin = registry->plugins[plugin_index];
  printf("Using plugin #%d: %s\n", plugin_index, plugin->pluginIdentifier);

  // Set host (TODO: do this in load_plugins?)
  OfxHost *host = getGlobalHost();

  // Load action if not loaded yet
  if (OfxPluginStatNotLoaded == registry->status[plugin_index]) {
    if (ofxhost_load_plugin(host, plugin)) {
      registry->status[plugin_index] = OfxPluginStatOK;
    } else {
      registry->status[plugin_index] = OfxPluginStatError;
      return false;
    }
  }

  if (OfxPluginStatError == registry->status[plugin_index]) {
    return false;
  }

  // Describe action
  OfxMeshEffectHandle effectDescriptor;
  if (ofxhost_get_descriptor(host, plugin, &effectDescriptor)) {
    OfxMeshEffectHandle effectInstance;

    // DEBUG
    printf("After describing effect:\n");
    printf("  Found %d inputs:\n", effectDescriptor->inputs.num_inputs);
    for (int i = 0 ; i < effectDescriptor->inputs.num_inputs ; ++i) {
      printf("    #%d: %s\n", i, effectDescriptor->inputs.inputs[i]->name);
    }

    // Create Instance action
    if (ofxhost_create_instance(plugin, effectDescriptor, &effectInstance)) {
      ofxhost_cook(plugin, effectInstance);
      ofxhost_destroy_instance(plugin, effectInstance);
    }
    ofxhost_release_descriptor(effectDescriptor);
  }

  // Unload action (TODO: move into e.g. free_registry)
  //ofxhost_unload_plugin(plugin);
  //releaseGlobalHost();

  return true;
}
