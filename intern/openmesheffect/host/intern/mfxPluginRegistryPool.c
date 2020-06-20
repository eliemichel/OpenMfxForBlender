/*
 * Copyright 2019 Elie Michel
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

#include <stdio.h>
#include <string.h>

#include "mfxPluginRegistryPool.h"

#include "util/memory_util.h"

#include "properties.h"

typedef struct PluginRegistryPoolEntry {
  PluginRegistry registry;
  char *filename;
  bool is_valid;
  int count;  // reference counter

  struct PluginRegistryPoolEntry *next; // chained list
} PluginRegistryPoolEntry;

typedef struct PluginRegistryPool {
  PluginRegistryPoolEntry *first_entry;
} PluginRegistryPool; 

PluginRegistryPool *gPluginRegistryPool = NULL;

/////////////////////////////////////////////////////////////

static void entry_init(PluginRegistryPoolEntry *entry, const char *filename)
{
  entry->is_valid = load_registry(&entry->registry, filename);
  size_t len = strlen(filename);
  entry->filename = malloc_array(sizeof(char), len + 1, "ofx filename");
  strncpy(entry->filename, filename, len + 1);
  entry->count = 0;
}

static void entry_free(PluginRegistryPoolEntry *entry)
{
  if (NULL != entry->filename) {
    free_array(entry->filename);
    entry->filename = NULL;
  }
  free_registry(&entry->registry);
}

/////////////////////////////////////////////////////////////

static void pool_free(PluginRegistryPool *pool)
{
  PluginRegistryPoolEntry *it = pool->first_entry;
  PluginRegistryPoolEntry *next_it;
  while (NULL != it) {
    entry_free(it);
    next_it = it->next;
    free_array(it);
    it = next_it;
  }
  pool->first_entry = NULL;
}

static PluginRegistryPoolEntry *pool_find(PluginRegistryPool *pool, const char *filename)
{
  PluginRegistryPoolEntry *it = pool->first_entry;
  while (NULL != it) {
    if (0 == strcmp(it->filename, filename)) {
      return it;
    }
    it = it->next;
  }
  return NULL;
}

static PluginRegistryPoolEntry *pool_add(PluginRegistryPool *pool, const char *filename)
{
  // Create and init entry
  PluginRegistryPoolEntry *entry = malloc_array(
    sizeof(PluginRegistryPoolEntry), 1, "plugin registry pool entry");
  entry_init(entry, filename);

  // Insert at head
  entry->next = pool->first_entry;
  pool->first_entry = entry;
  return entry;
}

static void pool_remove(PluginRegistryPool *pool,
                                            PluginRegistryPoolEntry *entry)
{
  if (NULL == entry)
    return;

  if (pool->first_entry == entry) {
    pool->first_entry = entry->next;
  }

  PluginRegistryPoolEntry *it = pool->first_entry;
  while (NULL != it) {
    if (it->next == entry) {
      it->next = entry->next;
    }
    it = it->next;
  }

  entry_free(entry);
  free_array(entry);
}

static void ensure_pool()
{
  if (NULL == gPluginRegistryPool) {
    gPluginRegistryPool = malloc_array(sizeof(PluginRegistryPool), 1, "global plugin registry pool");
    gPluginRegistryPool->first_entry = NULL;
  }
}

static void release_pool()
{
  if (NULL != gPluginRegistryPool) {
    pool_free(gPluginRegistryPool);
    free_array(gPluginRegistryPool);
    gPluginRegistryPool = NULL;
  }
}

PluginRegistry *get_registry(const char *ofx_filepath)
{
  ensure_pool();
  PluginRegistryPoolEntry *entry = pool_find(gPluginRegistryPool, ofx_filepath);
  
  if (NULL == entry) {
    printf("[get_registry] NEW registry for %s\n", ofx_filepath);
    entry = pool_add(gPluginRegistryPool, ofx_filepath);
  } else {
    printf("[get_registry] reusing registry for %s\n", ofx_filepath);
  }

  entry->count += 1;

  if (false == entry->is_valid) {
    return NULL;
  }

  return &entry->registry;
}

void release_registry(const char *ofx_filepath)
{
  printf("[release_registry] releasing registry for %s\n", ofx_filepath);
  ensure_pool();
  PluginRegistryPoolEntry *entry = pool_find(gPluginRegistryPool, ofx_filepath);
  if (NULL == entry) {
    printf("ERROR: Trying to release plugin that is not loaded; %s\n", ofx_filepath);
    return;
  }

  entry->count -= 1;

  if (entry->count == 0) {
    printf("[release_registry] removing registry for %s\n", ofx_filepath);
    pool_remove(gPluginRegistryPool, entry);
  }

  if (NULL == gPluginRegistryPool->first_entry) {
    release_pool();
  }
}
