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
  char plugin_path[1024];

  /**
   * Plug-in registry (as defined in mfxHost.h) holding the list of available filters within the
   * OFX bundle. This is a pointer to a reference-counted registry handled by mfxPluginRegistryPool
   */
  PluginRegistry *registry;

  /**
   * Tells whether the plugin specified by plugin_path is valid. If true, then 'registry' can be used
   */
  bool is_plugin_valid;

  /**
   * Index of the current effect within the list of effects contained in the currently opened
   * plugin. A value of -1 means none.
   */
  int effect_index;

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

/**
 * PRIVATE
 * Free the descriptor and instance, if it was allocated (otherwise does nothing)
 */
void runtime_free_effect_instance(OpenMeshEffectRuntime *rd);

/**
 * PRIVATE
 * Ensures that the ofx_host member if a valid OfxHost
 */
void runtime_ensure_host(OpenMeshEffectRuntime *rd);

/**
 * PRIVATE
 * Ensures that the effect descriptor and instances are valid (may fail, and hence return false)
 */
bool runtime_ensure_effect_instance(OpenMeshEffectRuntime *rd);

/**
 * PRIVATE
 * Ensures that the plugin path is unloaded and reset
 */
void runtime_reset_plugin_path(OpenMeshEffectRuntime *rd);

/**
 * Free runtime data content, but the the rd pointer itself.
 */
void runtime_free(OpenMeshEffectRuntime *rd);

/**
 * Set the plugin path, as put return status in is_plugin_valid
 */
void runtime_set_plugin_path(OpenMeshEffectRuntime *rd, const char *plugin_path);

/**
 * Pick an effect in the plugin by its index. Value is clamped to valid values (including -1 to
 * mean "no effect selected").
 */
void runtime_set_effect_index(OpenMeshEffectRuntime *rd, int effect_index);

/**
 * Set parameter values from Blender's RNA to the Open Mesh Effect host's structure.
 */
void runtime_get_parameters_from_rna(OpenMeshEffectRuntime *rd, OpenMeshEffectModifierData *fxmd);
