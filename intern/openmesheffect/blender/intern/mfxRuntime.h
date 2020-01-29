/**
 * Open Mesh Effect modifier for Blender
 * Copyright (C) 2019 - 2020 Elie Michel
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/** \file
 * \ingroup openmesheffect
 */

#include "mfxModifier.h"
#include "mfxHost.h"

#include "ofxCore.h"

/**
 * Structure holding runtime allocated data for OpenMeshEffect plug-in hosting.
 */
typedef struct OpenMeshEffectRuntime {
  /**
   * Path to the OFX plug-in bundle.
   */
  char current_plugin_path[1024];

  /**
   * Plug-in registry (as defined in mfxHost.h) holding the list of available filters within the
   * OFX bundle.
   */
  PluginRegistry registry;

  /**
   * Index of the current "asset" (a.k.a. "effect" or "plugin" - we should not use "asset" which is
   * a noun from Houdini API) wrt. the list contained in plugin registry.
   */
  int current_asset_index;

  /**
   * Number of parameters available in the current effect of the current OFX bundle.
   */
  int num_parameters;


  // OFX data handles
  
  /**
   * Pointer to the OfxHost singleton.
   */
  OfxHost *ofx_host;

  /**
   * Descriptor of the effect, listing its parameters for instance
   */
  OfxMeshEffectHandle effect_desc;

  /**
   * Effect instance, used for cooking
   */
  OfxMeshEffectHandle effect_instance;
} OpenMeshEffectRuntime;

/**
 * Initialize runtim data
 */
void runtime_init(OpenMeshEffectRuntime *rd);

// (private)
void runtime_free_effect_instance(OpenMeshEffectRuntime *rd);

// (private)
bool runtime_ensure_effect_instance(OpenMeshEffectRuntime *rd);

/**
 * Free runtime data content, but the the rd pointer itself.
 */
void runtime_free(OpenMeshEffectRuntime *rd);

bool runtime_set_plugin_path(OpenMeshEffectRuntime *rd, const char *plugin_path);

void runtime_set_asset_index(OpenMeshEffectRuntime *rd, int asset_index);

void runtime_get_parameters_from_rna(OpenMeshEffectRuntime *rd, OpenMeshEffectModifierData *fxmd);
