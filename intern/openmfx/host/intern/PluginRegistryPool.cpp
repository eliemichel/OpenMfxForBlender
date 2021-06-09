/*
 * Copyright 2019-2020 Elie Michel
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

#include "PluginRegistryPool.h"

#include <assert.h>
#include <string.h>

// // PluginRegistryPoolEntry

PluginRegistryPoolEntry::PluginRegistryPoolEntry(const char *filename)
{
  m_is_valid = load_registry(&m_registry, filename);
  size_t len = strlen(filename);
  m_filename = new char[len + 1];
  strncpy(m_filename, filename, len + 1);
  m_count = 0;
}

PluginRegistryPoolEntry::~PluginRegistryPoolEntry()
{
  assert(m_count == 0);
  if (NULL != m_filename) {
    delete[] m_filename;
    m_filename = NULL;
  }
  free_registry(&m_registry);
}

PluginRegistry &PluginRegistryPoolEntry::registry()
{
  return m_registry;
}

PluginRegistryPoolEntry *PluginRegistryPoolEntry::next() const
{
    return m_next;
}

void PluginRegistryPoolEntry::setNext(PluginRegistryPoolEntry *other)
{
  m_next = other;
}

const char *PluginRegistryPoolEntry::filename() const
{
  return m_filename;
}

bool PluginRegistryPoolEntry::isValid() const
{
  return m_is_valid;
}

void PluginRegistryPoolEntry::incrementReferences()
{
  ++m_count;
}

void PluginRegistryPoolEntry::decrementReferences()
{
  --m_count;
}

bool PluginRegistryPoolEntry::isReferenced() const
{
  return m_count > 0;
}

// // PluginRegistryPool

PluginRegistryPool &PluginRegistryPool::getInstance()
{
  static PluginRegistryPool s_instance;
  return s_instance;
}

PluginRegistryPool::PluginRegistryPool()
{
  m_first_entry = NULL;
}

PluginRegistryPool::~PluginRegistryPool()
{
  PluginRegistryPoolEntry *it = m_first_entry;
  PluginRegistryPoolEntry *next_it;
  while (NULL != it) {
    next_it = it->next();
    delete it;
    it = next_it;
  }
  m_first_entry = NULL;
}

PluginRegistryPoolEntry *PluginRegistryPool::find(const char *filename) const
{
  PluginRegistryPoolEntry *it = m_first_entry;
  while (NULL != it) {
    if (0 == strcmp(it->filename(), filename)) {
      return it;
    }
    it = it->next();
  }
  return NULL;
}

PluginRegistryPoolEntry *PluginRegistryPool::add(const char *filename)
{
  // Create and init entry
  PluginRegistryPoolEntry *entry = new PluginRegistryPoolEntry(filename);

  // Insert at head
  entry->setNext(m_first_entry);
  m_first_entry = entry;
  return entry;
}

void PluginRegistryPool::remove(PluginRegistryPoolEntry *entry)
{
  if (NULL == entry)
    return;

  if (m_first_entry == entry) {
    m_first_entry = entry->next();
  }

  PluginRegistryPoolEntry *it = m_first_entry;
  while (NULL != it) {
    if (it->next() == entry) {
      it->setNext(entry->next());
    }
    it = it->next();
  }

  delete entry;
}
