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

#ifndef __MFX_PLUGIN_REGISTRY_POOL_PRIVATE_H__
#define __MFX_PLUGIN_REGISTRY_POOL_PRIVATE_H__

#include "mfxPluginRegistry.h"

// // PluginRegistryPoolEntry

class PluginRegistryPoolEntry {
 public:
  PluginRegistryPoolEntry(const char *filename);
  ~PluginRegistryPoolEntry();

  // Disable copy, we handle it explicitely
  PluginRegistryPoolEntry(const PluginRegistryPoolEntry &) = delete;
  PluginRegistryPoolEntry &operator=(const PluginRegistryPoolEntry &) = delete;

  PluginRegistry &registry();

  PluginRegistryPoolEntry *next() const;
  void setNext(PluginRegistryPoolEntry *other);

  const char *filename() const;

  bool isValid() const;

  /**
   * Use these resp. before and after handling this plugin registry to user code
   */
  void incrementReferences();
  void decrementReferences();

  /**
   * Tells whether the entry is being used (ref counter == 0)
   */
  bool isReferenced() const;

 private:
  PluginRegistry m_registry;
  char *m_filename;
  bool m_is_valid;
  int m_count;  // reference counter

  PluginRegistryPoolEntry *m_next;  // chained list
};

// // PluginRegistryPool

class PluginRegistryPool {
 public:
  /**
   * Returns the singleton instance. Use this rather than allocating your own pool
   */
  static PluginRegistryPool &getInstance();

 public:
  PluginRegistryPool();
  ~PluginRegistryPool();

  // Disable copy, we handle it explicitely
  PluginRegistryPool(const PluginRegistryPool &) = delete;
  PluginRegistryPool &operator=(const PluginRegistryPool &) = delete;

  PluginRegistryPoolEntry *find(const char *filename) const;
  PluginRegistryPoolEntry *add(const char *filename);
  void remove(PluginRegistryPoolEntry *entry);

 private:
  PluginRegistryPoolEntry *m_first_entry;
};

#endif // __MFX_PLUGIN_REGISTRY_POOL_PRIVATE_H__

