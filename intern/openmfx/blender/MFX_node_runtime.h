#pragma once

struct PluginRegistry;

namespace blender::nodes::node_geo_open_mfx_cc {

/**
 * Runtime data stored in an OpenMfx Geometry Node.
 * Holds information about the currently loaded OpenMfx effect
 */
class RuntimeData {
 public:
  // These two setters return true iff they changed the value
  bool setPluginPath(const char *plugin_path);
  bool setEffectIndex(int effect_index);

  // Release the current plugin registry and reset
  void unloadPlugin();
  bool isPluginLoaded() const;

 public:
  char loaded_plugin_path[1024];
  int loaded_effect_index;

 private:
  PluginRegistry *m_registry;
};

}  // namespace blender::nodes::node_geo_open_mfx_cc
