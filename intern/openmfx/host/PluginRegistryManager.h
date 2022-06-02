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

#include <vector>

namespace OpenMfx {

class MfxHost;

// // PluginRegistryManagerEntry

class PluginRegistryManagerEntry {
 public:
  PluginRegistryManagerEntry(const char *filename, MfxHost *host);
  ~PluginRegistryManagerEntry();

  // Disable copy, we handle it explicitely
  PluginRegistryManagerEntry(const PluginRegistryManagerEntry &) = delete;
  PluginRegistryManagerEntry &operator=(const PluginRegistryManagerEntry &) = delete;

  PluginRegistry &registry();

  PluginRegistryManagerEntry *next() const;
  void setNext(PluginRegistryManagerEntry *other);

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

  /**
   * Initialize the descriptor if it does not already exist.
   * @return nullptr if the index is not valid
   */
  OfxMeshEffectHandle getDescriptor(int effectIndex);

 private:
  /**
   * Release all descriptors initialized for this registry
   */
  void releaseDescriptors();

 private:
  PluginRegistry m_registry;
  char *m_filename;
  bool m_is_valid;
  int m_count;  // reference counter
  std::vector<OfxMeshEffectHandle> m_descriptors;
  MfxHost *m_host;  // needed for descriptor management

  PluginRegistryManagerEntry *m_next;  // chained list
};

// // PluginRegistryManager

class PluginRegistryManager {
 public:
  /**
   * Returns the singleton instance. Use this rather than allocating your own pool
   */
  static PluginRegistryManager &GetInstance();

 public:
  PluginRegistryManager();
  ~PluginRegistryManager();

  // Disable copy, we handle it explicitely
  PluginRegistryManager(const PluginRegistryManager &) = delete;
  PluginRegistryManager &operator=(const PluginRegistryManager &) = delete;

  /**
   * Set the host before getting any registry.
   */
  void setHost(MfxHost *host);

  /**
   * Access the global plugin registry pool. There is one registry per ofx file,
   * and this pool ensures that the same registry is not loaded twice.
   * For each call to get_registry, a call to release_registry must be issued eventually
   * You must have called setHost() first
   */
  PluginRegistry *getRegistry(const char *ofx_filepath);
  void releaseRegistry(const PluginRegistry *registry);

  /**
   * Increment the reference counter of a registry. One must call releaseRegistry() as many
   * time as this was called (additional to the call that must be issued for getRegistry).
   */
  void incrementRegistryReference(const PluginRegistry *registry);

  /**
   * The handle remains valid until releaseRegistry() is called
   */
  OfxMeshEffectHandle getEffectDescriptor(const PluginRegistry *registry, int effectIndex);

 private:
  PluginRegistryManagerEntry *find(const char *filename) const;
  PluginRegistryManagerEntry *find(const PluginRegistry *registry) const;
  PluginRegistryManagerEntry *add(const char *filename);
  void remove(PluginRegistryManagerEntry *entry);

 private:
  PluginRegistryManagerEntry *m_first_entry;
  MfxHost *m_host;  // needed for descriptor management
};

}  // namespace OpenMfx

#endif // __MFX_PLUGIN_REGISTRY_POOL_PRIVATE_H__

