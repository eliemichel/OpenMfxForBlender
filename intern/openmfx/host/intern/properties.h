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

/** \file
 * \ingroup openmesheffect
 *
 */

#ifndef __MFX_PROPERTIES_H__
#define __MFX_PROPERTIES_H__

#include "Collection.h"

#include <string>

union OfxPropertyValueStruct {
    void *as_pointer;
    const char *as_const_char;
    char *as_char;
    double as_double;
    int as_int;
};

struct OfxPropertyStruct {
 public:
  OfxPropertyStruct() {}

  // Disable copy, we handle it explicitely
  OfxPropertyStruct(const OfxPropertyStruct &) = delete;
  OfxPropertyStruct &operator=(const OfxPropertyStruct &) = delete;

  OfxPropertyStruct(OfxPropertyStruct&&) = default;
  OfxPropertyStruct& operator=(OfxPropertyStruct&&) = default;

  void deep_copy_from(const OfxPropertyStruct &other);

  // For Collection
  using Index = std::string;
  void setIndex(const Index& index) { m_name = index; }
  Index index() const { return m_name; }

 public:
  OfxPropertyValueStruct value[4];

private:
    std::string m_name;
};

namespace OpenMfx {

enum PropertyType {
  PROP_TYPE_POINTER,
  PROP_TYPE_STRING,
  PROP_TYPE_DOUBLE,
  PROP_TYPE_INT,
};

// TODO: use kOfxPropType instead
enum class PropertySetContext {
  Host,        // kOfxTypeMeshEffectHost
  MeshEffect,  // kOfxTypeMeshEffect, kOfxTypeMeshEffectInstance
  Input,       // kOfxTypeMeshEffectInput
  Mesh,        // kOfxTypeMesh
  Param,       // kOfxTypeParameter
  Attrib,
  ActionIdentityIn,
  ActionIdentityOut,
  Other,
  // kOfxTypeParameterInstance
};

}  // namespace OpenMfx

// // OfxPropertySetStruct

struct OfxPropertySetStruct : OpenMfx::Collection<OfxPropertyStruct> {
  using PropertySetContext = OpenMfx::PropertySetContext;
  using PropertyType = OpenMfx::PropertyType;

 public:
  OfxPropertySetStruct(PropertySetContext context);

 public:
  static bool check_property_context(PropertySetContext context,
                                     PropertyType type,
                                     const char *property);

 public:
  PropertySetContext context; // TODO: use this rather than generic property set objects
};

#endif // __MFX_PROPERTIES_H__
