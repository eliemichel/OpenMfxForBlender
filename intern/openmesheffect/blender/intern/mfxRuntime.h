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
class OpenMeshEffectRuntime {
 public:
  OpenMeshEffectRuntime();
  ~OpenMeshEffectRuntime();

  /**
   * Set the plugin path, as put return status in is_plugin_valid
   */
  void set_plugin_path(const char *plugin_path);

  /**
   * Pick an effect in the plugin by its index. Value is clamped to valid values (including -1 to
   * mean "no effect selected").
   */
  void set_effect_index(int effect_index);

  /**
   * Set parameter values from Blender's RNA to the Open Mesh Effect host's structure.
   */
  void get_parameters_from_rna(OpenMeshEffectModifierData *fxmd);

  /**
   * Copy messages returned by the plugin in the RNA
   */
  void set_message_in_rna(OpenMeshEffectModifierData *fxmd);

  /**
   * Ensures that the effect descriptor and instances are valid (may fail, and hence return false)
   */
  bool ensure_effect_instance();

 public:
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

private:
  /**
   * Get absolute path (ui file browser returns relative path for saved files)
   */
  static void normalize_plugin_path(char *path, char *out_path);

private:
  /**
   * Free the descriptor and instance, if it had been allocated (otherwise does nothing)
   */
  void free_effect_instance();

  /**
   * Ensures that the ofx_host member if a valid OfxHost
   */
  void ensure_host();

  /**
   * Ensures that the plugin path is unloaded and reset
   */
  void reset_plugin_path();
};


///////////////////////////////////////////////////////////////////////////////
// Private functions

