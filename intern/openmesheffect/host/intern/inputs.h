/*
 * Copyright 2019 - 2020 Elie Michel
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

/** \file
 * \ingroup openmesheffect
 *
 */

#ifndef __MFX_INPUTS_H__
#define __MFX_INPUTS_H__

#include "properties.h"
#include "mesh.h"

typedef struct OfxMeshInputStruct {
    const char *name;
    OfxPropertySetStruct properties;
    OfxMeshStruct mesh;
    OfxHost *host; // weak pointer, do not deep copy
} OfxMeshInputStruct;

typedef struct OfxMeshInputSetStruct {
    int num_inputs;
    OfxMeshInputStruct **inputs;
    OfxHost *host; // weak pointer, do not deep copy
} OfxMeshInputSetStruct;

// OfxInputStruct

void init_input(OfxMeshInputStruct *input);
void free_input(OfxMeshInputStruct *input);
void deep_copy_input(OfxMeshInputStruct *destination, const OfxMeshInputStruct *source);

// OfxInputSetStruct

int find_input(const OfxMeshInputSetStruct *input_set, const char *input);
void append_inputs(OfxMeshInputSetStruct *input_set, int count);
int ensure_input(OfxMeshInputSetStruct *input_set, const char *input);
void init_input_set(OfxMeshInputSetStruct *input_set);
void free_input_set(OfxMeshInputSetStruct *input_set);
void deep_copy_input_set(OfxMeshInputSetStruct *destination, const OfxMeshInputSetStruct *source);

#endif // __MFX_INPUTS_H__
