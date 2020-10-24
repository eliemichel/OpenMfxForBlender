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

#ifndef __MFX_MESH_H__
#define __MFX_MESH_H__

#include "properties.h"
#include "attributes.h"

struct OfxMeshStruct {
 public:
  OfxMeshStruct();
  ~OfxMeshStruct();

  // Disable copy, we handle it explicitely
  OfxMeshStruct(const OfxMeshStruct &) = delete;
  OfxMeshStruct &operator=(const OfxMeshStruct &) = delete;

  void deep_copy_from(const OfxMeshStruct &other);

 public:
  OfxPropertySetStruct properties;
  OfxAttributeSetStruct attributes;
};

#endif // __MFX_MESH_H__
