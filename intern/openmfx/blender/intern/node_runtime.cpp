/**
 * OpenMfx geometry node for Blender
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

#include "MFX_node_runtime.h"
#include "MFX_util.h"

#include "BLI_math_vector.h"
#include "BLI_path_util.h"
#include "BLI_string.h"

#include "DNA_node_types.h" // bNode

#include "BlenderMfxHost.h"

#include <OpenMfx/Sdk/Cpp/Host/EffectRegistry>
#include <OpenMfx/Sdk/Cpp/Host/EffectLibrary>

#include <stdio.h>

#define EffectRegistry OpenMfx::EffectRegistry::GetInstance()

namespace blender::nodes::node_geo_open_mfx_cc {

RuntimeData::RuntimeData()
{
  m_loaded_plugin_path[0] = '\0';
  m_loaded_effect_index = -1;
  m_effect_descriptor = nullptr;
  m_effect_instance = nullptr;
  m_library = nullptr;
  m_must_update = true;
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
  m_library = other.m_library;
  m_must_update = other.m_must_update;

  EffectRegistry.incrementLibraryReference(m_library);

  if (other.m_effect_instance != nullptr) {
    ensureEffectInstance();
  }
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
    m_must_update = true;
    return true;
  }

  printf("Loading OFX plugin %s\n", m_loaded_plugin_path);

  char abs_path[FILE_MAX];
  MFX_normalize_plugin_path(abs_path, m_loaded_plugin_path);

  EffectRegistry.setHost(&BlenderMfxHost::GetInstance());
  m_library = EffectRegistry.getLibrary(abs_path);

  m_must_update = true;
  return true;
}

bool RuntimeData::setEffectIndex(int effect_index)
{
  if (m_loaded_effect_index == effect_index) {
    return false;
  }

  if (-1 != m_loaded_effect_index) {
    freeEffectInstance();
  }

  if (isPluginLoaded()) {
    m_loaded_effect_index = min_ii(max_ii(-1, effect_index), m_library->effectCount() - 1);
  }
  else {
    m_loaded_effect_index = -1;
  }

  if (-1 != m_loaded_effect_index) {
    ensureEffectInstance();
  }

  m_must_update = true;
  return true;
}

void RuntimeData::clearMustUpdate()
{
  m_must_update = false;
}

OfxMeshEffectHandle RuntimeData::effectDescriptor() const
{
  return m_effect_descriptor;
}

OfxMeshEffectHandle RuntimeData::effectInstance() const
{
  return m_effect_instance;
}

const OpenMfx::EffectLibrary &RuntimeData::library() const
{
  return *m_library;
}

bool RuntimeData::mustUpdate() const
{
  return m_must_update;
}

void RuntimeData::unloadPlugin()
{
  if (isPluginLoaded()) {
    printf("Unloading OFX plugin %s\n", m_loaded_plugin_path);
    freeEffectInstance();

    EffectRegistry.releaseLibrary(m_library);
    m_library = nullptr;
  }
  m_loaded_plugin_path[0] = '\0';
  m_loaded_effect_index = -1;
}

inline bool RuntimeData::isPluginLoaded() const
{
  return m_library != nullptr;
}

void RuntimeData::ensureEffectInstance()
{
  if (nullptr != m_effect_instance)
    return; // Instance already available

  if (!isPluginLoaded() || m_loaded_effect_index == -1)
    return; // Invalid effect

  m_effect_descriptor = EffectRegistry.getEffectDescriptor(m_library, m_loaded_effect_index);

  if (m_effect_descriptor == nullptr)
    return; // Invalid effect

  auto &host = BlenderMfxHost::GetInstance();
  if (!host.CreateInstance(m_effect_descriptor, m_effect_instance)) {
    m_effect_instance = nullptr;
  }
}

void RuntimeData::freeEffectInstance()
{
  if (nullptr != m_effect_instance) {
    auto &host = BlenderMfxHost::GetInstance();
    host.DestroyInstance(m_effect_instance);
    m_effect_instance = nullptr;
  }
  m_effect_descriptor = nullptr;
  m_loaded_effect_index = -1;
}

} // namespace blender::nodes::node_geo_open_mfx_cc
