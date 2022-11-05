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

#include "EffectLibrary.h"

#include <OpenMfx/Sdk/Cpp/Common>

namespace OpenMfx {

class Host;

/**
 * An entry of the effect registry corresponds to one loaded
 * effect library (.ofx file). It wraps it into a chained list.
 * Each library holds a reference counter, and when this drops to 0 the library
 * is unloaded automatically. It also holds a descriptor for each effect in the
 * library. The descriptor is lazyly loaded on demand.
 * 
 * This class is only instanciated by the EffectRegistry, a user of the SDK
 * should not interact with it unless they know whath they are doing.
 */
class EffectRegistryEntry {
 public:
  EffectRegistryEntry(const char *filename, Host *host);
  ~EffectRegistryEntry();
  MOVE_ONLY(EffectRegistryEntry)

  EffectLibrary &library();
  const char *filename() const;

  EffectRegistryEntry *next() const;
  void setNext(EffectRegistryEntry *other);

  bool isValid() const;

  /**
   * Use these resp. before and after handling this effect library to user code
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
  EffectLibrary m_library;
  char *m_filename;
  bool m_is_valid;
  int m_count;  // reference counter
  std::vector<OfxMeshEffectHandle> m_descriptors;
  Host *m_host;  // needed for descriptor management

  EffectRegistryEntry *m_next;  // chained list
};

}  // namespace OpenMfx
