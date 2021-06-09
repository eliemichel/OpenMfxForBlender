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

#include <stdbool.h>

union OfxPropertyValueStruct {
    void *as_pointer;
    const char *as_const_char;
    char *as_char;
    double as_double;
    int as_int;
};

struct OfxPropertyStruct {
 public:
  OfxPropertyStruct();
  ~OfxPropertyStruct();

  // Disable copy, we handle it explicitely
  OfxPropertyStruct(const OfxPropertyStruct &) = delete;
  OfxPropertyStruct &operator=(const OfxPropertyStruct &) = delete;


  void deep_copy_from(const OfxPropertyStruct &other);

 public:
  const char *name;
  OfxPropertyValueStruct value[4];
};

enum PropertyType {
	PROP_TYPE_POINTER,
	PROP_TYPE_STRING,
	PROP_TYPE_DOUBLE,
	PROP_TYPE_INT,
};

// TODO: use kOfxPropType instead
enum class PropertySetContext {
  Host, // kOfxTypeMeshEffectHost
  MeshEffect, // kOfxTypeMeshEffect, kOfxTypeMeshEffectInstance
  Input, // kOfxTypeMeshEffectInput
  Mesh, // kOfxTypeMesh
  Param, // kOfxTypeParameter
  Attrib,
  ActionIdentityIn,
  ActionIdentityOut,
  Other,
  // kOfxTypeParameterInstance
};

// // OfxPropertySetStruct

struct OfxPropertySetStruct {
 public:
  OfxPropertySetStruct(PropertySetContext context);
  ~OfxPropertySetStruct();

  // Disable copy, we handle it explicitely
  OfxPropertySetStruct(const OfxPropertySetStruct &) = delete;
  OfxPropertySetStruct &operator=(const OfxPropertySetStruct &) = delete;

  int find_property(const char *property) const;
  void append_properties(int count);
  void remove_property(int index);
  int ensure_property(const char *property);

  void deep_copy_from(const OfxPropertySetStruct &other);

 public:
  static bool check_property_context(PropertySetContext context,
                                     PropertyType type,
                                     const char *property);

 public:
  PropertySetContext context; // TODO: use this rather than generic property set objects
  int num_properties;
  OfxPropertyStruct **properties;
};

#endif // __MFX_PROPERTIES_H__
