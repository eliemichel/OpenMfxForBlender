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

TEST(OpenMeshEffectHost, ListPlugins)
{
	PluginRegistry registry;
	EXPECT_EQ(load_registry(&registry, FULL_LIBRARY_OUTPUT_PATH "openmesheffect_sample_plugin.ofx"), true);
	EXPECT_EQ(use_plugin(&registry, 0), true);
	free_registry(&registry);
}
