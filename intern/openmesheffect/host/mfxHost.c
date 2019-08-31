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

// OFX SUITES MAIN

static const void * fetchSuite(OfxPropertySetHandle host,
                               const char *suiteName,
                               int suiteVersion) {
  (void)host; // TODO: check host?

  if (0 == strcmp(suiteName, kOfxMeshEffectSuite) && suiteVersion == 1) {
    switch (suiteVersion) {
      case 1:
        return &gMeshEffectSuiteV1;
      default:
        printf("Suite '%s' is only supported in version 1.\n", suiteName);
        return NULL;
    }
  }
  if (0 == strcmp(suiteName, kOfxParameterSuite) && suiteVersion == 1) {
    switch (suiteVersion) {
      case 1:
        return &gParameterSuiteV1;
      default:
        printf("Suite '%s' is only supported in version 1.\n", suiteName);
        return NULL;
    }
  }
  if (0 == strcmp(suiteName, kOfxPropertySuite) && suiteVersion == 1) {
    switch (suiteVersion) {
      case 1:
        return &gPropertySuiteV1;
      default:
        printf("Suite '%s' is only supported in version 1.\n", suiteName);
        return NULL;
    }
  }

  printf("Suite '%s' is not supported by this host.\n", suiteName);
  return NULL;
}

// OFX MESH EFFECT HOST

OfxHost *gHost = NULL;
int gHostUse = 0;

OfxHost * getGlobalHost(void) {
  printf("Getting Global Host; reference counter will be set to %d.\n", gHostUse + 1);
  if (0 == gHostUse) {
    printf("(Allocating new host data)\n");
    gHost = malloc_array(sizeof(OfxHost), 1, "global host");
    OfxPropertySetHandle hostProperties = malloc_array(sizeof(OfxPropertySetStruct), 1, "global host properties");
    init_properties(hostProperties);
    hostProperties->context = PROP_CTX_HOST;
    propSetPointer(hostProperties, kOfxHostPropBeforeMeshReleaseCb, 0, (void*)NULL);
    propSetPointer(hostProperties, kOfxHostPropBeforeMeshGetCb, 0, (void*)NULL);
    gHost->host = hostProperties;
    gHost->fetchSuite = fetchSuite;
  }
  ++gHostUse;
  return gHost;
}

void releaseGlobalHost(void) {
  printf("Releasing Global Host; reference counter will be set to %d.\n", gHostUse - 1);
  if (--gHostUse == 0) {
    printf("(Freeing host data)\n");
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
