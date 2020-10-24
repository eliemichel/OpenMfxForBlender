/*
 * Copyright 2019-2020 Elie Michel
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

OfxAttributeStruct::OfxAttributeStruct()
{
  properties.context = PROP_CTX_ATTRIB;
}

OfxAttributeStruct::~OfxAttributeStruct()
{
  free_array(name);
}

void OfxAttributeStruct::set_name(const char *name)
{
  // deep copy attribute name
  this->name = (char*)malloc_array(sizeof(char), strlen(name) + 1, "attribute name");
  strcpy(this->name, name);
}

void OfxAttributeStruct::deep_copy_from(const OfxAttributeStruct &other)
{
  // deep string copy
  this->name = (char*)malloc_array(sizeof(char), strlen(other.name) + 1, "attribute name");
  strcpy(this->name, other.name);

  this->attachment = other.attachment;
  this->properties.deep_copy_from(other.properties);
}

// // OfxAttributeSetStruct

OfxAttributeSetStruct::OfxAttributeSetStruct()
{
  num_attributes = 0;
  attributes = NULL;
}

OfxAttributeSetStruct::~OfxAttributeSetStruct()
{
  for (int i = 0; i < this->num_attributes; ++i) {
    delete this->attributes[i];
  }
  this->num_attributes = 0;
  if (NULL != this->attributes) {
    free_array(this->attributes);
    this->attributes = NULL;
  }
}

int OfxAttributeSetStruct::find(AttributeAttachment attachment, const char *attribute) const
{
  for (int i = 0 ; i < this->num_attributes ; ++i) {
    if (this->attributes[i]->attachment == attachment
      && 0 == strcmp(this->attributes[i]->name, attribute)) {
      return i;
    }
  }
  return -1;
}

void OfxAttributeSetStruct::append(int count)
{
  int old_num_attributes = this->num_attributes;
  OfxAttributeStruct **old_attributes = this->attributes;
  this->num_attributes += count;
  this->attributes = (OfxAttributeStruct **)malloc_array(
      sizeof(OfxAttributeStruct *), this->num_attributes, "attributes");
  for (int i = 0; i < this->num_attributes; ++i) {
    OfxAttributeStruct *attribute;
    if (i < old_num_attributes) {
      attribute = old_attributes[i];
    } else {
      attribute = new OfxAttributeStruct();
    }
    this->attributes[i] = attribute;
  }
  if (NULL != old_attributes) {
    free_array(old_attributes);
  }
}

int OfxAttributeSetStruct::ensure(AttributeAttachment attachment, const char *attribute)
{
  int i = find(attachment, attribute);
  if (i == -1) {
    append(1);
    i = this->num_attributes - 1;
    this->attributes[i]->attachment = attachment;
    this->attributes[i]->set_name(attribute);
  }
  return i;
}

void OfxAttributeSetStruct::deep_copy_from(const OfxAttributeSetStruct &other)
{
  append(other.num_attributes);
  for (int i = 0 ; i < this->num_attributes ; ++i) {
    this->attributes[i]->deep_copy_from(*other.attributes[i]);
  }
}

