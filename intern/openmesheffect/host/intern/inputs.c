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

#include "inputs.h"

// // OfxInputStruct

void init_input(OfxMeshInputStruct *input) {
  init_properties(&input->properties);
  input->properties.context = PROP_CTX_INPUT;
  init_mesh(&input->mesh);
  input->host = NULL;
}

void free_input(OfxMeshInputStruct *input) {
  free_properties(&input->properties);
  free_mesh(&input->mesh);
  free_array(input);
}

void deep_copy_input(OfxMeshInputStruct *destination, const OfxMeshInputStruct *source) {
  destination->name = source->name; // weak pointer?
  deep_copy_property_set(&destination->properties, &source->properties);
  deep_copy_mesh(&destination->mesh, &source->mesh);
  destination->host = source->host; // not deep copied, as this is a weak pointer
}

// // OfxInputSetStruct

int find_input(const OfxMeshInputSetStruct *input_set, const char *input) {
  for (int i = 0 ; i < input_set->num_inputs ; ++i) {
    if (0 == strcmp(input_set->inputs[i]->name, input)) {
      return i;
    }
  }
  return -1;
}

void append_inputs(OfxMeshInputSetStruct *input_set, int count) {
  int old_num_input = input_set->num_inputs;
  OfxMeshInputStruct **old_inputs = input_set->inputs;
  input_set->num_inputs += count;
  input_set->inputs = malloc_array(sizeof(OfxMeshInputStruct*), input_set->num_inputs, "inputs");
  for (int i = 0 ; i < input_set->num_inputs ; ++i){
    OfxMeshInputStruct *input;
    if (i < old_num_input) {
      input = old_inputs[i];
    } else {
      input = malloc_array(sizeof(OfxMeshInputStruct), 1, "input");
      input->host = input_set->host;
      init_input(input);
    }
    input_set->inputs[i] = input;
  }
  if (NULL != old_inputs) {
    free_array(old_inputs);
  }
}

int ensure_input(OfxMeshInputSetStruct *input_set, const char *input) {
  int i = find_input(input_set, input);
  if (i == -1) {
    append_inputs(input_set, 1);
    i = input_set->num_inputs - 1;
    input_set->inputs[i]->name = input;
  }
  return i;
}

void init_input_set(OfxMeshInputSetStruct *input_set) {
  input_set->num_inputs = 0;
  input_set->inputs = NULL;
  input_set->host = NULL;
}

void free_input_set(OfxMeshInputSetStruct *input_set) {
  for (int i = 0 ; i < input_set->num_inputs ; ++i){
    free_input(input_set->inputs[i]);
  }
  input_set->num_inputs = 0;
  if (NULL != input_set->inputs) {
    free_array(input_set->inputs);
    input_set->inputs = NULL;
  }
}

void deep_copy_input_set(OfxMeshInputSetStruct *destination, const OfxMeshInputSetStruct *source) {
  init_input_set(destination);
  append_inputs(destination, source->num_inputs);
  for (int i = 0 ; i < destination->num_inputs ; ++i) {
    deep_copy_input(destination->inputs[i], source->inputs[i]);
  }
  destination->host = source->host; // not deep copied, as this is a weak pointer
}

