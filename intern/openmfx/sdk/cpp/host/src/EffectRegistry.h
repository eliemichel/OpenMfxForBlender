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

#pragma once

#include "MeshEffect.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <vector>

namespace OpenMfx {

class Host;
class EffectRegistryEntry;
class EffectLibrary;

/**
 * The effect registry takes care of loading each .ofx file only once even if
 * the same one is used by multiple objects.
 * 
 * Make sure to set a pointer to the host using setHost() prior to loading any
 * effect library.
 */
class EffectRegistry {
 public:
  /**
   * Returns the singleton instance. Use this rather than allocating your own pool
   */
  static EffectRegistry &GetInstance();

 public:
  EffectRegistry();
  ~EffectRegistry();
  MOVE_ONLY(EffectRegistry)

  /**
   * Set the host before getting any registry.
   */
  void setHost(Host *host);

  /**
   * Access the global plugin registry pool. There is one registry per ofx file,
   * and this pool ensures that the same registry is not loaded twice.
   * For each call to get_registry, a call to release_registry must be issued eventually
   * You must have called setHost() first
   */
  EffectLibrary *getLibrary(const char *ofx_filepath);
  void releaseLibrary(const EffectLibrary *registry);

  /**
   * Increment the reference counter of a registry. One must call releaseLibrary() as many
   * time as this was called (additional to the call that must be issued for getLibrary).
   */
  void incrementLibraryReference(const EffectLibrary *library);

  /**
   * The handle remains valid until releaseLibrary() is called
   */
  OfxMeshEffectHandle getEffectDescriptor(const EffectLibrary *library, int effectIndex);

 private:
  EffectRegistryEntry *find(const char *filename) const;
  EffectRegistryEntry *find(const EffectLibrary *library) const;
  EffectRegistryEntry *add(const char *filename);
  void remove(EffectRegistryEntry *entry);

 private:
  EffectRegistryEntry *m_first_entry;
  Host *m_host;  // needed for descriptor management
};

}  // namespace OpenMfx
