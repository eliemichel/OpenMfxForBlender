/*
 * Copyright 2019 - 2022 Elie Michel
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

#include "Properties.h"
#include "Mesh.h"
#include "Collection.h"

#include <ofxCore.h>

#include <string>

struct OfxMeshInputStruct {
 public:
  OfxMeshInputStruct();

  // Disable copy, we handle it explicitely
  OfxMeshInputStruct(const OfxMeshInputStruct &) = delete;
  OfxMeshInputStruct &operator=(const OfxMeshInputStruct &) = delete;

  OfxMeshInputStruct(OfxMeshInputStruct &&) = default;
  OfxMeshInputStruct &operator=(OfxMeshInputStruct &&) = default;

  void deep_copy_from(const OfxMeshInputStruct &other);

  // For Collection
  using Index = std::string;
  void setIndex(const Index &index)
  {
    m_name = index;
  }
  Index index() const
  {
    return m_name;
  }

  const std::string &name() const
  {
    return m_name;
  }

 public:
  OfxPropertySetStruct properties;
  OfxAttributeSetStruct
      requested_attributes;  // not technically attributes, e.g. data info are not used
  OfxMeshStruct mesh;
  OfxHost *host;  // weak pointer, do not deep copy

 private:
  std::string m_name;
};

struct OfxMeshInputSetStruct : OpenMfx::Collection<OfxMeshInputStruct> {
 public:
  OfxMeshInputSetStruct();

 protected:
  void onNewItem(OfxMeshInputStruct &input) override;

 public:
  OfxHost *host;  // weak pointer, do not deep copy
};

namespace OpenMfx {
typedef OfxMeshInputStruct MeshInput;
typedef OfxMeshInputSetStruct MeshInputSet;
} // namespace OpenMfx
