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

#include "EffectRegistry.h"
#include "EffectRegistryEntry.h"
#include "Host.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <cassert>
#include <cstring>

namespace OpenMfx {

EffectRegistry &EffectRegistry::GetInstance()
{
  static EffectRegistry s_instance;
  return s_instance;
}

EffectRegistry::EffectRegistry()
{
  m_first_entry = NULL;
}

EffectRegistry::~EffectRegistry()
{
  EffectRegistryEntry *it = m_first_entry;
  EffectRegistryEntry *next_it;
  while (NULL != it) {
    next_it = it->next();
    delete it;
    it = next_it;
  }
  m_first_entry = NULL;
}

void EffectRegistry::setHost(Host *host)
{
  m_host = host;
}

EffectLibrary *EffectRegistry::getLibrary(const char *ofx_filepath)
{
  EffectRegistryEntry *entry = find(ofx_filepath);

  if (NULL == entry) {
    LOG << "[get_library] NEW library for " << ofx_filepath;
    entry = add(ofx_filepath);
  }
  else {
    LOG << "[get_Library] reusing library for " << ofx_filepath;
  }

  entry->incrementReferences();

  if (false == entry->isValid()) {
    return nullptr;
  }

  return &entry->library();
}

void EffectRegistry::releaseLibrary(const EffectLibrary *library)
{
  EffectRegistryEntry *entry = find(library);
  if (nullptr == entry) {
    return;
  }

  entry->decrementReferences();

  if (false == entry->isReferenced()) {
    remove(entry);
  }
}

void EffectRegistry::incrementLibraryReference(const EffectLibrary* library)
{
  EffectRegistryEntry *entry = find(library);
  if (nullptr == entry) {
    return;
  }
  entry->incrementReferences();
}

OfxMeshEffectHandle EffectRegistry::getEffectDescriptor(const EffectLibrary* library,
                                                            int effectIndex)
{
  EffectRegistryEntry *entry = find(library);
  if (nullptr == entry) {
    return nullptr;
  }
  return entry->getDescriptor(effectIndex);
}

EffectRegistryEntry *EffectRegistry::find(const char *filename) const
{
  EffectRegistryEntry *it = m_first_entry;
  while (NULL != it) {
    if (0 == strcmp(it->filename(), filename)) {
      return it;
    }
    it = it->next();
  }
  return NULL;
}

EffectRegistryEntry *EffectRegistry::find(const EffectLibrary *library) const
{
  EffectRegistryEntry *it = m_first_entry;
  while (NULL != it) {
    if (&it->library() == library) {
      return it;
    }
    it = it->next();
  }
  return NULL;
}

EffectRegistryEntry *EffectRegistry::add(const char *filename)
{
  assert(m_host != nullptr);

  // Create and init entry
  EffectRegistryEntry *entry = new EffectRegistryEntry(filename, m_host);

  // Insert at head
  entry->setNext(m_first_entry);
  m_first_entry = entry;
  return entry;
}

void EffectRegistry::remove(EffectRegistryEntry *entry)
{
  if (NULL == entry)
    return;

  if (m_first_entry == entry) {
    m_first_entry = entry->next();
  }

  EffectRegistryEntry *it = m_first_entry;
  while (NULL != it) {
    if (it->next() == entry) {
      it->setNext(entry->next());
    }
    it = it->next();
  }

  delete entry;
}

}  // namespace OpenMfx
