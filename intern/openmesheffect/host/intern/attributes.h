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

typedef enum AttributeAttachment {
  ATTR_ATTACH_INVALID = -1,
  ATTR_ATTACH_POINT,
  ATTR_ATTACH_VERTEX,
  ATTR_ATTACH_FACE,
  ATTR_ATTACH_MESH,
} AttributeAttachment;

typedef struct OfxAttributeStruct {
  const char *name;
  AttributeAttachment attachment;
  OfxPropertySetStruct properties;
} OfxAttributeStruct;

typedef struct OfxAttributeSetStruct {
    int num_attributes;
    OfxAttributeStruct **attributes;
} OfxAttributeSetStruct;

// OfxMeshAttributeStruct

void init_attribute(OfxAttributeStruct *attribute);
void free_attribute(OfxAttributeStruct *attribute);
void deep_copy_attribute(OfxAttributeStruct *destination, const OfxAttributeStruct *source);

// OfxAttributeSetStruct

int find_attribute(const OfxAttributeSetStruct *attribute_set, AttributeAttachment attachment, const char *attribute);
void append_attributes(OfxAttributeSetStruct *attribute_set, int count);
int ensure_attribute(OfxAttributeSetStruct *attribute_set, AttributeAttachment attachment, const char *attribute);
void init_attribute_set(OfxAttributeSetStruct *attribute_set);
void free_attribute_set(OfxAttributeSetStruct *attribute_set);
void deep_copy_attribute_set(OfxAttributeSetStruct *destination, const OfxAttributeSetStruct *source);

#endif // __MFX_ATTRIBUTES_H__
