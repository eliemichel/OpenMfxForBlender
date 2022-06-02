#include "MFX_node_runtime.h"
#include "MFX_util.h"

#include "BLI_math_vector.h"
#include "BLI_path_util.h"
#include "BLI_string.h"

#include "PluginRegistryManager.h"
#include "BlenderMfxHost.h"

#include <stdio.h>

using OpenMfx::PluginRegistryManager;
#define PluginManager OpenMfx::PluginRegistryManager::GetInstance()

namespace blender::nodes::node_geo_open_mfx_cc {

RuntimeData::RuntimeData()
{
  m_loaded_plugin_path[0] = '\0';
  m_loaded_effect_index = -1;
  m_effect_descriptor = nullptr;
  m_registry = nullptr;
}

RuntimeData::~RuntimeData()
{
  unloadPlugin();
}

RuntimeData &RuntimeData::operator=(const RuntimeData &other)
{
  BLI_strncpy(m_loaded_plugin_path, other.m_loaded_plugin_path, sizeof(m_loaded_plugin_path));
  m_loaded_effect_index = other.m_loaded_effect_index;
  m_effect_descriptor = other.m_effect_descriptor;
  m_registry = other.m_registry;
  PluginManager.incrementRegistryReference(m_registry);
  return *this;
}

bool RuntimeData::setPluginPath(const char *plugin_path)
{
  if (0 == strcmp(m_loaded_plugin_path, plugin_path)) {
    return false;
  }

  unloadPlugin();

  BLI_strncpy(m_loaded_plugin_path, plugin_path, sizeof(m_loaded_plugin_path));

  if (0 == strcmp(m_loaded_plugin_path, "")) {
    return true;
  }

  printf("Loading OFX plugin %s\n", m_loaded_plugin_path);

  char abs_path[FILE_MAX];
  MFX_normalize_plugin_path(abs_path, m_loaded_plugin_path);

  PluginManager.setHost(&BlenderMfxHost::GetInstance());
  m_registry = PluginManager.getRegistry(abs_path);
  return true;
}

bool RuntimeData::setEffectIndex(int effect_index)
{
  if (m_loaded_effect_index == effect_index) {
    return false;
  }

  if (-1 != m_loaded_effect_index) {
    m_effect_descriptor = nullptr;
    // free_effect_instance();
  }

  if (isPluginLoaded()) {
    m_loaded_effect_index = min_ii(max_ii(-1, effect_index), m_registry->num_plugins - 1);
  }
  else {
    m_loaded_effect_index = -1;
  }

  if (-1 != m_loaded_effect_index) {
    m_effect_descriptor = PluginManager.getEffectDescriptor(m_registry, m_loaded_effect_index);
    // ensure_effect_instance();
  }
  return true;
}

OfxMeshEffectHandle RuntimeData::effectDescriptor() const
{
  return m_effect_descriptor;
}

const PluginRegistry &RuntimeData::registry() const
{
  return *m_registry;
}

void RuntimeData::unloadPlugin()
{
  if (isPluginLoaded()) {
    printf("Unloading OFX plugin %s\n", m_loaded_plugin_path);
    m_effect_descriptor = nullptr;
    // free_effect_instance();

    PluginManager.releaseRegistry(m_registry);
    m_registry = nullptr;
  }
  m_loaded_plugin_path[0] = '\0';
  m_loaded_effect_index = -1;
}

inline bool RuntimeData::isPluginLoaded() const
{
  return m_registry != nullptr;
}

} // namespace blender::nodes::node_geo_open_mfx_cc
