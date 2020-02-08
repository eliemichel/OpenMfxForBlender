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

#include "attributes.h"

// // OfxMeshAttributeStruct

void init_attribute(OfxAttributeStruct *attribute)
{
  init_properties(&attribute->properties);
  attribute->properties.context = PROP_CTX_ATTRIB;
}

void free_attribute(OfxAttributeStruct *attribute)
{
  free_properties(&attribute->properties);
  free_array(attribute->name);
}

void set_name_attribute(OfxAttributeStruct *attribute, const char *name)
{
  // deep copy attribute name
  attribute->name = (char*)malloc_array(sizeof(char), strlen(name) + 1, "attribute name");
  strcpy(attribute->name, name);
}

void deep_copy_attribute(OfxAttributeStruct *destination, const OfxAttributeStruct *source)
{
  // deep string copy
  destination->name = (char*)malloc_array(sizeof(char), strlen(source->name) + 1, "attribute name");
  strcpy(destination->name, source->name);

  destination->attachment = source->attachment;
  deep_copy_property_set(&destination->properties, &source->properties);
}

// // OfxAttributeSetStruct

int find_attribute(const OfxAttributeSetStruct *attribute_set, AttributeAttachment attachment, const char *attribute) {
  for (int i = 0 ; i < attribute_set->num_attributes ; ++i) {
    if (attribute_set->attributes[i]->attachment == attachment
      && 0 == strcmp(attribute_set->attributes[i]->name, attribute)) {
      return i;
    }
  }
  return -1;
}

void append_attributes(OfxAttributeSetStruct *attribute_set, int count) {
  int old_num_attributes = attribute_set->num_attributes;
  OfxAttributeStruct **old_attributes = attribute_set->attributes;
  attribute_set->num_attributes += count;
  attribute_set->attributes = malloc_array(sizeof(OfxAttributeStruct*), attribute_set->num_attributes, "attributes");
  for (int i = 0 ; i < attribute_set->num_attributes; ++i){
    OfxAttributeStruct *attribute;
    if (i < old_num_attributes) {
      attribute = old_attributes[i];
    } else {
      attribute = malloc_array(sizeof(OfxAttributeStruct), 1, "attribute");
      init_attribute(attribute);
    }
    attribute_set->attributes[i] = attribute;
  }
  if (NULL != old_attributes) {
    free_array(old_attributes);
  }
}

int ensure_attribute(OfxAttributeSetStruct *attribute_set, AttributeAttachment attachment, const char *attribute) {
  int i = find_attribute(attribute_set, attachment, attribute);
  if (i == -1) {
    append_attributes(attribute_set, 1);
    i = attribute_set->num_attributes - 1;
    attribute_set->attributes[i]->attachment = attachment;
    set_name_attribute(attribute_set->attributes[i], attribute);
  }
  return i;
}

void init_attribute_set(OfxAttributeSetStruct *attribute_set) {
  attribute_set->num_attributes = 0;
  attribute_set->attributes = NULL;
}

void free_attribute_set(OfxAttributeSetStruct *attribute_set) {
  for (int i = 0 ; i < attribute_set->num_attributes; ++i){
    free_attribute(attribute_set->attributes[i]);
  }
  attribute_set->num_attributes = 0;
  if (NULL != attribute_set->attributes) {
    free_array(attribute_set->attributes);
    attribute_set->attributes = NULL;
  }
}

void deep_copy_attribute_set(OfxAttributeSetStruct *destination, const OfxAttributeSetStruct *source) {
  init_attribute_set(destination);
  append_attributes(destination, source->num_attributes);
  for (int i = 0 ; i < destination->num_attributes ; ++i) {
    deep_copy_attribute(destination->attributes[i], source->attributes[i]);
  }
}

