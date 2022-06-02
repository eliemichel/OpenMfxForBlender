#pragma once

#include "ofxMeshEffect.h"

struct PluginRegistry;

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

  // These two setters return true iff they changed the value
  bool setPluginPath(const char *plugin_path);
  bool setEffectIndex(int effect_index);
  OfxMeshEffectHandle effectDescriptor() const;
  const PluginRegistry &registry() const;

 private:
  // Release the current plugin registry and reset
  void unloadPlugin();
  bool isPluginLoaded() const;

 private:
  char m_loaded_plugin_path[1024];
  int m_loaded_effect_index;
  OfxMeshEffectHandle m_effect_descriptor;
  PluginRegistry *m_registry;
};

}  // namespace blender::nodes::node_geo_open_mfx_cc
