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

#include "mfxHost.h"
#include "intern/properties.h"

TEST(OpenMeshEffectHost, ListPlugins)
{
	PluginRegistry registry;
	EXPECT_EQ(load_registry(&registry, FULL_LIBRARY_OUTPUT_PATH "openmesheffect_sample_plugin.ofx"), true);
	EXPECT_EQ(use_plugin(&registry, 0), true);
	free_registry(&registry);
}

TEST(OpenMeshEffectHost, Properties)
{
	OfxPropertySetStruct property_set;
	init_properties(&property_set);

	property_set.context = PROP_CTX_OTHER; // Will raise warnings
	EXPECT_EQ(propSetPointer(&property_set, "TestPropPointer", 0, &property_set), kOfxStatOK);
	EXPECT_EQ(propSetString(&property_set, "TestPropString", 0, "lorem ipsum"), kOfxStatOK);
	EXPECT_EQ(propSetDouble(&property_set, "TestPropDouble", 0, 3.1415), kOfxStatOK);
	EXPECT_EQ(propSetInt(&property_set, "TestPropInt", 0, 42), kOfxStatOK);

	property_set.context = PROP_CTX_MESH_EFFECT;
	EXPECT_EQ(propSetInt(&property_set, "WrongField", 0, 0), kOfxStatErrBadHandle);
	EXPECT_EQ(propSetInt(&property_set, kOfxMeshEffectPropContext, 0, 0), kOfxStatOK);

	property_set.context = PROP_CTX_INPUT;
	EXPECT_EQ(propSetInt(&property_set, "WrongField", 0, 0), kOfxStatErrBadHandle);
	EXPECT_EQ(propSetInt(&property_set, kOfxPropLabel, 0, 0), kOfxStatErrBadHandle); // wrong type
	EXPECT_EQ(propSetString(&property_set, kOfxPropLabel, 0, "label"), kOfxStatOK);

	property_set.context = PROP_CTX_MESH;
	EXPECT_EQ(propSetInt(&property_set, "WrongField", 0, 0), kOfxStatErrBadHandle);
	EXPECT_EQ(propSetInt(&property_set, kOfxMeshPropInternalData, 0, 0), kOfxStatErrBadHandle);
	EXPECT_EQ(propSetPointer(&property_set, kOfxMeshPropInternalData, 0, NULL), kOfxStatOK);

	property_set.context = PROP_CTX_HOST;
	EXPECT_EQ(propSetInt(&property_set, "WrongField", 0, 0), kOfxStatErrBadHandle);
	EXPECT_EQ(propSetInt(&property_set, kOfxHostPropBeforeMeshReleaseCb, 0, 0), kOfxStatErrBadHandle);
	EXPECT_EQ(propSetPointer(&property_set, kOfxHostPropBeforeMeshReleaseCb, 0, NULL), kOfxStatOK);
}

TEST(OpenMeshEffectHost, IdentityPlugin)
{
	PluginRegistry registry;
	EXPECT_EQ(load_registry(&registry, FULL_LIBRARY_OUTPUT_PATH "openmesheffect_identity_plugin.ofx"), true);
	EXPECT_EQ(use_plugin(&registry, 0), true);
	free_registry(&registry);
}
