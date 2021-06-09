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

#ifndef __MFX_ATTRIBUTES_H__
#define __MFX_ATTRIBUTES_H__

#include "properties.h"

enum class AttributeAttachment {
  Invalid = -1,
  Point,
  Corner,
  Face,
  Mesh,
};

struct OfxAttributeStruct {
 public:
  OfxAttributeStruct();
  ~OfxAttributeStruct();

  // Disable copy, we handle it explicitely
  OfxAttributeStruct(const OfxAttributeStruct &) = delete;
  OfxAttributeStruct &operator=(const OfxAttributeStruct &) = delete;

  void set_name(const char *name);

  void deep_copy_from(const OfxAttributeStruct &other);

 public:
  char *name; // points to memory owned by this object
  AttributeAttachment attachment;
  OfxPropertySetStruct properties;
};

struct OfxAttributeSetStruct {
 public:
  OfxAttributeSetStruct();
  ~OfxAttributeSetStruct();

  // Disable copy, we handle it explicitely
  OfxAttributeSetStruct(const OfxAttributeSetStruct &) = delete;
  OfxAttributeSetStruct &operator=(const OfxAttributeSetStruct &) = delete;

  int find(AttributeAttachment attachment, const char *attribute) const;
  void append(int count);
  int ensure(AttributeAttachment attachment, const char *attribute);

  void deep_copy_from(const OfxAttributeSetStruct &other);

 public:
  int num_attributes;
  OfxAttributeStruct **attributes;
};

#endif // __MFX_ATTRIBUTES_H__
