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

#include "testing/testing.h"

#include "intern/properties.h"
#include "intern/propertySuite.h"
#include "mfxHost/MfxHost"
#include "mfxPluginRegistry.h"
#include "ofxExtras.h"

using namespace OpenMfx;

TEST(OpenMfxHost, ListPlugins)
{
  MfxHost host;
  PluginRegistry registry;
  EXPECT_EQ(load_registry(&registry, FULL_LIBRARY_OUTPUT_PATH "openmfx_sample_plugin.ofx"), true);

  EXPECT_EQ(registry.num_plugins, 2);

  OfxPlugin *plugin = registry.plugins[0];
  EXPECT_EQ(host.LoadPlugin(plugin), true);

  OfxMeshEffectHandle effectDescriptor;
  EXPECT_EQ(host.GetDescriptor(plugin, effectDescriptor), true);

  OfxMeshEffectHandle effectInstance;
  EXPECT_EQ(host.CreateInstance(effectDescriptor, effectInstance), true);

  bool shouldCook = false;
  char *inputName = nullptr;
  EXPECT_EQ(host.IsIdentity(effectInstance, &shouldCook, &inputName), false);

  host.DestroyInstance(effectInstance);
  host.ReleaseDescriptor(effectDescriptor);
  host.UnloadPlugin(plugin);
  free_registry(&registry);
}

TEST(OpenMfxHost, Properties)
{
  OfxPropertySetStruct property_set(PropertySetContext::Other);

  property_set.context = PropertySetContext::Other;  // Will raise warnings
  EXPECT_EQ(propSetPointer(&property_set, "TestPropPointer", 0, &property_set), kOfxStatOK);
  EXPECT_EQ(propSetString(&property_set, "TestPropString", 0, "lorem ipsum"), kOfxStatOK);
  EXPECT_EQ(propSetDouble(&property_set, "TestPropDouble", 0, 3.1415), kOfxStatOK);
  EXPECT_EQ(propSetInt(&property_set, "TestPropInt", 0, 42), kOfxStatOK);

  property_set.context = PropertySetContext::MeshEffect;
  EXPECT_EQ(propSetInt(&property_set, "WrongField", 0, 0), kOfxStatErrBadHandle);
  EXPECT_EQ(propSetString(&property_set, kOfxMeshEffectPropContext, 0, "x"), kOfxStatOK);

  property_set.context = PropertySetContext::Input;
  EXPECT_EQ(propSetInt(&property_set, "WrongField", 0, 0), kOfxStatErrBadHandle);
  EXPECT_EQ(propSetInt(&property_set, kOfxPropLabel, 0, 0), kOfxStatErrBadHandle);  // wrong type
  EXPECT_EQ(propSetString(&property_set, kOfxPropLabel, 0, "label"), kOfxStatOK);

  property_set.context = PropertySetContext::Mesh;
  EXPECT_EQ(propSetInt(&property_set, "WrongField", 0, 0), kOfxStatErrBadHandle);
  EXPECT_EQ(propSetInt(&property_set, kOfxMeshPropInternalData, 0, 0), kOfxStatErrBadHandle);
  EXPECT_EQ(propSetPointer(&property_set, kOfxMeshPropInternalData, 0, NULL), kOfxStatOK);

  property_set.context = PropertySetContext::Host;
  EXPECT_EQ(propSetInt(&property_set, "WrongField", 0, 0), kOfxStatErrBadHandle);
  EXPECT_EQ(propSetInt(&property_set, kOfxHostPropBeforeMeshReleaseCb, 0, 0),
            kOfxStatErrBadHandle);
  EXPECT_EQ(propSetPointer(&property_set, kOfxHostPropBeforeMeshReleaseCb, 0, NULL), kOfxStatOK);
}

TEST(OpenMfxHost, IdentityPlugin)
{
  MfxHost host;
  PluginRegistry registry;
  EXPECT_EQ(load_registry(&registry, FULL_LIBRARY_OUTPUT_PATH "openmfx_identity_plugin.ofx"),
            true);

  OfxPlugin *plugin = registry.plugins[0];
  EXPECT_EQ(host.LoadPlugin(plugin), true);

  OfxMeshEffectHandle effectDescriptor;
  EXPECT_EQ(host.GetDescriptor(plugin, effectDescriptor), true);

  OfxMeshEffectHandle effectInstance;
  EXPECT_EQ(host.CreateInstance(effectDescriptor, effectInstance), true);

  bool shouldCook = false;
  char *inputName = nullptr;
  EXPECT_EQ(host.IsIdentity(effectInstance, &shouldCook, &inputName), true);

  host.DestroyInstance(effectInstance);
  host.ReleaseDescriptor(effectDescriptor);
  host.UnloadPlugin(plugin);
  free_registry(&registry);
}
