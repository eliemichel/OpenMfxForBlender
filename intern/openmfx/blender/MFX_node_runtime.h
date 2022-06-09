/**
 * OpenMfx node for Blender
 * Copyright (C) 2019 - 2022 Elie Michel
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
 * \ingroup openmfx
 */

#pragma once

#include "ofxMeshEffect.h"

struct PluginRegistry;
struct bNode;

namespace blender::nodes::node_geo_open_mfx_cc {

/**
 * Runtime data stored in an OpenMfx Geometry Node.
 * Holds information about the currently loaded OpenMfx effect
 */
class RuntimeData {
 public:
  RuntimeData();
  ~RuntimeData();
  RuntimeData(const RuntimeData &) = delete;
  RuntimeData &operator=(const RuntimeData &other);

  // These two setters return true iff they changed the value, and also mustUpdate is turned true when they return true
  bool setPluginPath(const char *plugin_path);
  bool setEffectIndex(int effect_index);
  void clearMustUpdate();

  OfxMeshEffectHandle effectDescriptor() const;
  OfxMeshEffectHandle effectInstance() const;
  const PluginRegistry &registry() const;
  bool mustUpdate() const;

 private:
  // Release the current plugin registry and reset
  void unloadPlugin();
  bool isPluginLoaded() const;

  void ensureEffectInstance();
  void freeEffectInstance();

 private:
  bool m_must_update;
  char m_loaded_plugin_path[1024];
  int m_loaded_effect_index;
  OfxMeshEffectHandle m_effect_descriptor;
  OfxMeshEffectHandle m_effect_instance;
  PluginRegistry *m_registry;
};

}  // namespace blender::nodes::node_geo_open_mfx_cc
