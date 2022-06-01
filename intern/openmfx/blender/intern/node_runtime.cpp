#include "MFX_node_runtime.h"
#include "MFX_util.h"

#include "BLI_math_vector.h"
#include "BLI_path_util.h"
#include "BLI_string.h"

#include "mfxPluginRegistryPool.h"

#include <stdio.h>

namespace blender::nodes::node_geo_open_mfx_cc {

bool RuntimeData::setPluginPath(const char *plugin_path)
{
  if (0 == strcmp(loaded_plugin_path, plugin_path)) {
    return false;
  }

  unloadPlugin();

  BLI_strncpy(loaded_plugin_path, plugin_path, sizeof(loaded_plugin_path));

  if (0 == strcmp(loaded_plugin_path, "")) {
    return true;
  }

  printf("Loading OFX plugin %s\n", loaded_plugin_path);

  char abs_path[FILE_MAX];
  MFX_normalize_plugin_path(abs_path, loaded_plugin_path);

  m_registry = get_registry(abs_path);
  return true;
}

bool RuntimeData::setEffectIndex(int effect_index)
{
  if (loaded_effect_index == effect_index) {
    return false;
  }

  if (-1 != loaded_effect_index) {
    // free_effect_instance();
  }

  if (isPluginLoaded()) {
    loaded_effect_index = min_ii(max_ii(-1, effect_index), m_registry->num_plugins - 1);
  }
  else {
    loaded_effect_index = -1;
  }

  if (-1 != loaded_effect_index) {
    // ensure_effect_instance();
  }
  return true;
}

void RuntimeData::unloadPlugin()
{
  if (isPluginLoaded()) {
    printf("Unloading OFX plugin %s\n", loaded_plugin_path);
    // free_effect_instance();

    release_registry(m_registry);
  }
  loaded_plugin_path[0] = '\0';
  loaded_effect_index = -1;
}

inline bool RuntimeData::isPluginLoaded() const
{
  return m_registry != nullptr;
}

}
