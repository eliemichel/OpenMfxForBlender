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

#include "ofxCore.h"

struct OfxMeshInputStruct {
 public:
  OfxMeshInputStruct();
  ~OfxMeshInputStruct();

  // Disable copy, we handle it explicitely
  OfxMeshInputStruct(const OfxMeshInputStruct &) = delete;
  OfxMeshInputStruct &operator=(const OfxMeshInputStruct &) = delete;

  void deep_copy_from(const OfxMeshInputStruct &other);

 public:
  const char *name;
  OfxPropertySetStruct properties;
  OfxAttributeSetStruct requested_attributes; // not technically attributes, e.g. data info are not used
  OfxMeshStruct mesh;
  OfxHost *host; // weak pointer, do not deep copy
};

struct OfxMeshInputSetStruct {
 public:
  OfxMeshInputSetStruct();
  ~OfxMeshInputSetStruct();

  // Disable copy, we handle it explicitely
  OfxMeshInputSetStruct(const OfxMeshInputSetStruct &) = delete;
  OfxMeshInputSetStruct &operator=(const OfxMeshInputSetStruct &) = delete;

  int find(const char *input) const;
  void append(int count);
  int ensure(const char *input);

  void deep_copy_from(const OfxMeshInputSetStruct &other);

 public:
  int num_inputs;
  OfxMeshInputStruct **inputs;
  OfxHost *host; // weak pointer, do not deep copy
};

#endif // __MFX_INPUTS_H__
