/*
 * Copyright 2019-2022 Elie Michel
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

#include "EffectRegistryEntry.h"
#include "Host.h"

#include <cassert>
#include <cstring>

namespace OpenMfx {

EffectRegistryEntry::EffectRegistryEntry(const char *filename, Host *host)
    : m_host(host), m_count(0), m_next(nullptr)
{
  m_is_valid = m_library.load(filename);
  size_t len = strlen(filename);
  m_filename = new char[len + 1];
  strncpy(m_filename, filename, len + 1);

  m_descriptors.resize(m_library.effectCount());
  fill(m_descriptors.begin(), m_descriptors.end(), nullptr);
}

EffectRegistryEntry::~EffectRegistryEntry()
{
  assert(m_count == 0);
  if (NULL != m_filename) {
    delete[] m_filename;
    m_filename = NULL;
  }
  m_library.unload();
}

EffectLibrary& EffectRegistryEntry::library()
{
  return m_library;
}

EffectRegistryEntry* EffectRegistryEntry::next() const
{
  return m_next;
}

void EffectRegistryEntry::setNext(EffectRegistryEntry *other)
{
  m_next = other;
}

const char *EffectRegistryEntry::filename() const
{
  return m_filename;
}

bool EffectRegistryEntry::isValid() const
{
  return m_is_valid;
}

void EffectRegistryEntry::incrementReferences()
{
  ++m_count;
}

void EffectRegistryEntry::decrementReferences()
{
  --m_count;
}

bool EffectRegistryEntry::isReferenced() const
{
  return m_count > 0;
}

void EffectRegistryEntry::releaseDescriptors()
{
  assert(m_host != nullptr);
  for (auto &desc : m_descriptors) {
    if (nullptr != desc) {
      m_host->ReleaseDescriptor(desc);
      desc = nullptr;
    }
  }

  for (int i = 0; i < m_library.effectCount(); ++i) {
    OfxPlugin *plugin = m_library.plugin(i);
    EffectLibrary::Status status = m_library.pluginStatus(i);
    if (EffectLibrary::Status::OK == status) {
      m_host->UnloadPlugin(plugin);
      status = EffectLibrary::Status::NotLoaded;
    }
  }
}

OfxMeshEffectHandle EffectRegistryEntry::getDescriptor(int effectIndex)
{
  if (!isValid())
    return nullptr;

  if (effectIndex < 0 || effectIndex >= m_descriptors.size())
    return nullptr;

  OfxMeshEffectHandle &desc = m_descriptors[effectIndex];

  OfxPlugin *plugin = m_library.plugin(effectIndex);

  EffectLibrary::Status status = m_library.pluginStatus(effectIndex);

  assert(m_host != nullptr);
  if (EffectLibrary::Status::NotLoaded == status) {
    if (m_host->LoadPlugin(plugin)) {
      status = EffectLibrary::Status::OK;
    }
    else {
      ERR_LOG << "Error while loading plugin!\n";
      status = EffectLibrary::Status::Error;
      return nullptr;
    }
  }

  if (!m_host->GetDescriptor(plugin, desc)) {
    desc = nullptr;
  }

  return desc;
}

} // namespace OpenMfx
