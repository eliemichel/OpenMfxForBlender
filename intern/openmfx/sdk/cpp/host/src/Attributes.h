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

#pragma once

#include "Properties.h"
#include "Collection.h"
#include "AttributeEnums.h"

#include <vector>
#include <string>
#include <utility>

struct OfxAttributeStruct {
  using AttributeType = OpenMfx::AttributeType;
  using AttributeAttachment = OpenMfx::AttributeAttachment;
  using AttributeSemantic = OpenMfx::AttributeSemantic;

  OfxAttributeStruct();

  OfxAttributeStruct(const OfxAttributeStruct &) = delete;
  OfxAttributeStruct &operator=(const OfxAttributeStruct &) = delete;

  OfxAttributeStruct(OfxAttributeStruct &&) = default;
  OfxAttributeStruct &operator=(OfxAttributeStruct &&) = default;

  void deep_copy_from(const OfxAttributeStruct& other);

  AttributeAttachment attachment() const;
  const std::string& name() const;

  AttributeType type() const;
  void setType(AttributeType type);

  int componentCount() const;
  void setComponentCount(int componentCount);

  AttributeSemantic semantic() const;
  void setSemantic(AttributeSemantic semantic);

  void* data() const;
  void setData(void *data);

  int byteStride() const;
  void setByteStride(int byteStride);

  bool copy_data_from(const OfxAttributeStruct &source, int start, int count);

  // For Collection
  using Index = std::pair<AttributeAttachment, std::string>;
  void setIndex(const Index &index);
  Index index() const;

 public:
  OfxPropertySetStruct properties;

 private:
  std::string m_name;
  AttributeAttachment m_attachment = AttributeAttachment::Invalid;
};

struct OfxAttributeSetStruct : OpenMfx::Collection<OfxAttributeStruct> {
};

namespace OpenMfx {
typedef OfxAttributeStruct Attribute;
typedef OfxAttributeSetStruct AttributeSet;
} // namespace OpenMfx
