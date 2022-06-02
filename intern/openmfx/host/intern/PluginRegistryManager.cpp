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

#include "PluginRegistryManager.h"
#include "MfxHost.h"

#include <assert.h>
#include <string.h>

#ifndef NDEBUG
#include <cstdio>
#  define LOG printf
#else
#  define LOG(...)
#endif

namespace OpenMfx {

// // PluginRegistryManagerEntry

PluginRegistryManagerEntry::PluginRegistryManagerEntry(const char *filename, MfxHost *host)
    : m_host(host), m_count(0), m_next(nullptr)
{
  m_is_valid = load_registry(&m_registry, filename);
  size_t len = strlen(filename);
  m_filename = new char[len + 1];
  strncpy(m_filename, filename, len + 1);

  m_descriptors.resize(m_registry.num_plugins);
  fill(m_descriptors.begin(), m_descriptors.end(), nullptr);
}

PluginRegistryManagerEntry::~PluginRegistryManagerEntry()
{
  releaseDescriptors();
  assert(m_count == 0);
  if (NULL != m_filename) {
    delete[] m_filename;
    m_filename = NULL;
  }
  free_registry(&m_registry);
}

PluginRegistry &PluginRegistryManagerEntry::registry()
{
  return m_registry;
}

PluginRegistryManagerEntry *PluginRegistryManagerEntry::next() const
{
  return m_next;
}

void PluginRegistryManagerEntry::setNext(PluginRegistryManagerEntry *other)
{
  m_next = other;
}

const char *PluginRegistryManagerEntry::filename() const
{
  return m_filename;
}

bool PluginRegistryManagerEntry::isValid() const
{
  return m_is_valid;
}

void PluginRegistryManagerEntry::incrementReferences()
{
  ++m_count;
}

void PluginRegistryManagerEntry::decrementReferences()
{
  --m_count;
}

bool PluginRegistryManagerEntry::isReferenced() const
{
  return m_count > 0;
}

void PluginRegistryManagerEntry::releaseDescriptors()
{
  assert(m_host != nullptr);
  for (auto &desc : m_descriptors) {
    if (nullptr != desc) {
      m_host->ReleaseDescriptor(desc);
      desc = nullptr;
    }
  }
}

OfxMeshEffectHandle PluginRegistryManagerEntry::getDescriptor(int effectIndex)
{
  if (!isValid())
    return nullptr;

  if (effectIndex < 0 || effectIndex >= m_descriptors.size())
    return nullptr;

  OfxMeshEffectHandle &desc = m_descriptors[effectIndex];

  OfxPlugin *plugin = m_registry.plugins[effectIndex];

  OfxPluginStatus &status = m_registry.status[effectIndex];

  assert(m_host != nullptr);
  if (OfxPluginStatNotLoaded == status) {
    if (m_host->LoadPlugin(plugin)) {
      status = OfxPluginStatOK;
    }
    else {
      LOG("Error while loading plugin!\n");
      status = OfxPluginStatError;
      return nullptr;
    }
  }

  if (!m_host->GetDescriptor(plugin, desc)) {
    desc = nullptr;
  }

  return desc;
}

// // PluginRegistryManager

PluginRegistryManager &PluginRegistryManager::getInstance()
{
  static PluginRegistryManager s_instance;
  return s_instance;
}

PluginRegistryManager::PluginRegistryManager()
{
  m_first_entry = NULL;
}

PluginRegistryManager::~PluginRegistryManager()
{
  PluginRegistryManagerEntry *it = m_first_entry;
  PluginRegistryManagerEntry *next_it;
  while (NULL != it) {
    next_it = it->next();
    delete it;
    it = next_it;
  }
  m_first_entry = NULL;
}

void PluginRegistryManager::setHost(MfxHost *host)
{
  m_host = host;
}

PluginRegistry *PluginRegistryManager::getRegistry(const char *ofx_filepath)
{
  PluginRegistryManagerEntry *entry = find(ofx_filepath);

  if (NULL == entry) {
    LOG("[get_registry] NEW registry for %s\n", ofx_filepath);
    entry = add(ofx_filepath);
  }
  else {
    LOG("[get_registry] reusing registry for %s\n", ofx_filepath);
  }

  entry->incrementReferences();

  if (false == entry->isValid()) {
    return nullptr;
  }

  return &entry->registry();
}

void PluginRegistryManager::releaseRegistry(const PluginRegistry *registry)
{
  PluginRegistryManagerEntry *entry = find(registry);
  if (nullptr == entry) {
    return;
  }

  entry->decrementReferences();

  if (false == entry->isReferenced()) {
    remove(entry);
  }
}

OfxMeshEffectHandle PluginRegistryManager::getEffectDescriptor(const PluginRegistry *registry,
                                                            int effectIndex)
{
  PluginRegistryManagerEntry *entry = find(registry);
  if (nullptr == entry) {
    return nullptr;
  }
  return entry->getDescriptor(effectIndex);
}

PluginRegistryManagerEntry *PluginRegistryManager::find(const char *filename) const
{
  PluginRegistryManagerEntry *it = m_first_entry;
  while (NULL != it) {
    if (0 == strcmp(it->filename(), filename)) {
      return it;
    }
    it = it->next();
  }
  return NULL;
}

PluginRegistryManagerEntry *PluginRegistryManager::find(const PluginRegistry *registry) const
{
  PluginRegistryManagerEntry *it = m_first_entry;
  while (NULL != it) {
    if (&it->registry() == registry) {
      return it;
    }
    it = it->next();
  }
  return NULL;
}

PluginRegistryManagerEntry *PluginRegistryManager::add(const char *filename)
{
  assert(m_host != nullptr);

  // Create and init entry
  PluginRegistryManagerEntry *entry = new PluginRegistryManagerEntry(filename, m_host);

  // Insert at head
  entry->setNext(m_first_entry);
  m_first_entry = entry;
  return entry;
}

void PluginRegistryManager::remove(PluginRegistryManagerEntry *entry)
{
  if (NULL == entry)
    return;

  if (m_first_entry == entry) {
    m_first_entry = entry->next();
  }

  PluginRegistryManagerEntry *it = m_first_entry;
  while (NULL != it) {
    if (it->next() == entry) {
      it->setNext(entry->next());
    }
    it = it->next();
  }

  delete entry;
}

}  // namespace OpenMfx
