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
 */


#include "util/ofx_util.h"
#include "util/memory_util.h"

#include "intern/messages.h"
#include "intern/properties.h"
#include "intern/parameters.h"
#include "intern/inputs.h"
#include "intern/mesheffect.h"
#include "intern/parameterSuite.h"
#include "intern/propertySuite.h"
#include "intern/meshEffectSuite.h"
#include "intern/messageSuite.h"
#include "mfxPluginRegistry.h"

#include "mfxHost.h"

#include "ofxProperty.h"
#include "ofxParam.h"
#include "ofxMeshEffect.h"
#include "ofxMessage.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

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
  if (0 == strcmp(suiteName, kOfxMessageSuite) && suiteVersion <= 2) {
    switch (suiteVersion) {
      case 1: // V2 is backward compatible
      case 2:
        return &gMessageSuiteV2;
      default:
        printf("Suite '%s' is only supported in version 1.\n", suiteName);
        return NULL;
    }
  }

  printf("Suite '%s' is not supported by this host.\n", suiteName);
  return NULL;
}

// OFX MESH EFFECT HOST

// TODO: Use a more C++ idiomatic singleton pattern
OfxHost *gHost = NULL;
int gHostUse = 0;

OfxHost * getGlobalHost(void) {
  printf("Getting Global Host; reference counter will be set to %d.\n", gHostUse + 1);
  if (0 == gHostUse) {
    printf("(Allocating new host data)\n");
    gHost = new OfxHost;
    OfxPropertySetHandle hostProperties = new OfxPropertySetStruct(PropertySetContext::Host);
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
    delete gHost->host;
    delete gHost;
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
  effectHandle = new OfxMeshEffectStruct(host);

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
  delete effectDescriptor;
}

bool ofxhost_create_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectDescriptor, OfxMeshEffectHandle *effectInstance) {
  OfxStatus status;
  OfxMeshEffectHandle instance;

  *effectInstance = NULL;

  instance = new OfxMeshEffectStruct(effectDescriptor->host);
  instance->deep_copy_from(*effectDescriptor);

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

  delete effectInstance;
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

bool ofxhost_is_identity(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance, bool *shouldCook) {
  OfxStatus status;

  OfxPropertySetStruct inArgs(PropertySetContext::ActionIdentityIn);
  OfxPropertySetStruct outArgs(PropertySetContext::ActionIdentityOut);

  propSetInt(&inArgs, kOfxPropTime, 0, 0);
  propSetString(&outArgs, kOfxPropName, 0, "");
  propSetInt(&outArgs, kOfxPropTime, 0, 0);

  *shouldCook = true;

  status = plugin->mainEntry(kOfxMeshEffectActionIsIdentity, effectInstance, &inArgs, &outArgs);
  printf("%s action returned status %d (%s)\n", kOfxMeshEffectActionIsIdentity, status, getOfxStateName(status));

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
  if (kOfxStatOK == status) {
    *shouldCook = false;
    return true;
  }
  return true;
}
