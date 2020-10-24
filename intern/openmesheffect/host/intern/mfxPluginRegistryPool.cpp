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

#include "mfxPluginRegistryPool.h"
#include "PluginRegistryPool.h"

#include <cstdio>

PluginRegistry *get_registry(const char *ofx_filepath)
{
  PluginRegistryPool & pluginRegistryPool = PluginRegistryPool::getInstance();
  PluginRegistryPoolEntry *entry = pluginRegistryPool.find(ofx_filepath);
  
  if (NULL == entry) {
    printf("[get_registry] NEW registry for %s\n", ofx_filepath);
    entry = pluginRegistryPool.add(ofx_filepath);
  } else {
    printf("[get_registry] reusing registry for %s\n", ofx_filepath);
  }

  entry->incrementReferences();

  if (false == entry->isValid()) {
    return NULL;
  }

  return &entry->registry();
}

void release_registry(const char *ofx_filepath)
{
  printf("[release_registry] releasing registry for %s\n", ofx_filepath);
  PluginRegistryPool &pluginRegistryPool = PluginRegistryPool::getInstance();
  PluginRegistryPoolEntry *entry = pluginRegistryPool.find(ofx_filepath);
  if (NULL == entry) {
    printf("ERROR: Trying to release plugin that is not loaded; %s\n", ofx_filepath);
    return;
  }

  entry->decrementReferences();

  if (false == entry->isReferenced()) {
    printf("[release_registry] removing registry for %s\n", ofx_filepath);
    pluginRegistryPool.remove(entry);
  }
}
