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

#include <string.h>

#include "util/memory_util.h"

#include "mesh.h"

// // OfxInputStruct

void init_mesh(OfxMeshStruct *mesh) {
  init_properties(&mesh->properties);
  init_attribute_set(&mesh->attributes);
  mesh->properties.context = PROP_CTX_MESH;
}

void free_mesh(OfxMeshStruct *mesh) {
  free_properties(&mesh->properties);
  free_attribute_set(&mesh->attributes);
}

void deep_copy_mesh(OfxMeshStruct *destination, const OfxMeshStruct *source) {
  deep_copy_property_set(&destination->properties, &source->properties);
  deep_copy_attribute_set(&destination->attributes, &source->attributes);
}
