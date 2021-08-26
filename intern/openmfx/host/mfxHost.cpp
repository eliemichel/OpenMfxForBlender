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


#include "intern/util/ofx_util.h"

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

using namespace OpenMfx;

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
