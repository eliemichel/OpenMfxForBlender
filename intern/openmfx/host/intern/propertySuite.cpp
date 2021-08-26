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

#include "propertySuite.h"
#include "properties.h"

using namespace OpenMfx;

// // Property Suite Entry Points

const OfxPropertySuiteV1 gPropertySuiteV1 = {
    /* propSetPointer */ propSetPointer,
    /* propSetString */ propSetString,
    /* propSetDouble */ propSetDouble,
    /* propSetInt */ propSetInt,
    /* propSetPointerN */ propSetPointerN,
    /* propSetStringN */ propSetStringN,
    /* propSetDoubleN */ propSetDoubleN,
    /* propSetIntN */ propSetIntN,
    /* propGetPointer */ propGetPointer,
    /* propGetString */ propGetString,
    /* propGetDouble */ propGetDouble,
    /* propGetInt */ propGetInt,
    /* propGetPointerN */ propGetPointerN,
    /* propGetStringN */ propGetStringN,
    /* propGetDoubleN */ propGetDoubleN,
    /* propGetIntN */ propGetIntN,
    /* propReset */ propReset,
    /* propGetDimension */ propGetDimension};

OfxStatus propSetPointer(OfxPropertySetHandle properties,
                         const char *property,
                         int index,
                         void *value)
{
    OfxPropertySetStruct& props = *properties;

  if (false == OfxPropertySetStruct::check_property_context(
      props.context, PROP_TYPE_POINTER, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = props.ensure(property);
  props[i].value[index].as_pointer = value;
  return kOfxStatOK;
}

OfxStatus propSetString(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        const char *value)
{
    OfxPropertySetStruct& props = *properties;

  if (false == OfxPropertySetStruct::check_property_context(
      props.context, PROP_TYPE_STRING, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }

  int i = props.ensure(property);
  props[i].value[index].as_const_char = value;
  return kOfxStatOK;
}

OfxStatus propSetDouble(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        double value)
{
    OfxPropertySetStruct& props = *properties;
  if (false == OfxPropertySetStruct::check_property_context(
                   props.context, PROP_TYPE_DOUBLE, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }

  int i = props.ensure(property);
  props[i].value[index].as_double = value;
  return kOfxStatOK;
}

OfxStatus propSetInt(OfxPropertySetHandle properties, const char *property, int index, int value)
{
  if (false ==
      OfxPropertySetStruct::check_property_context(properties->context, PROP_TYPE_INT, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }

  OfxPropertySetStruct& props = *properties;
  int i = props.ensure(property);
  props[i].value[index].as_int = value;
  return kOfxStatOK;
}

OfxStatus propSetPointerN(OfxPropertySetHandle properties,
                          const char *property,
                          int count,
                          void *const *value)
{
  for (int i = 0; i < count; ++i) {
    OfxStatus status = propSetPointer(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propSetStringN(OfxPropertySetHandle properties,
                         const char *property,
                         int count,
                         const char *const *value)
{
  for (int i = 0; i < count; ++i) {
    OfxStatus status = propSetString(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propSetDoubleN(OfxPropertySetHandle properties,
                         const char *property,
                         int count,
                         const double *value)
{
  for (int i = 0; i < count; ++i) {
    OfxStatus status = propSetDouble(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propSetIntN(OfxPropertySetHandle properties,
                      const char *property,
                      int count,
                      const int *value)
{
  for (int i = 0; i < count; ++i) {
    OfxStatus status = propSetInt(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propGetPointer(OfxPropertySetHandle properties,
                         const char *property,
                         int index,
                         void **value)
{
    OfxPropertySetStruct& props = *properties;
  if (false == OfxPropertySetStruct::check_property_context(
                   props.context, PROP_TYPE_POINTER, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = props.ensure(property);
  *value = props[i].value[index].as_pointer;
  return kOfxStatOK;
}

OfxStatus propGetString(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        char **value)
{
    OfxPropertySetStruct& props = *properties;
  if (false == OfxPropertySetStruct::check_property_context(
                   props.context, PROP_TYPE_STRING, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = props.ensure(property);
  *value = props[i].value[index].as_char;
  return kOfxStatOK;
}

OfxStatus propGetDouble(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        double *value)
{
    OfxPropertySetStruct& props = *properties;
  if (false == OfxPropertySetStruct::check_property_context(
                   props.context, PROP_TYPE_DOUBLE, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = props.ensure(property);
  *value = props[i].value[index].as_double;
  return kOfxStatOK;
}

OfxStatus propGetInt(OfxPropertySetHandle properties, const char *property, int index, int *value)
{
    OfxPropertySetStruct& props = *properties;
  if (false ==
      OfxPropertySetStruct::check_property_context(props.context, PROP_TYPE_INT, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = props.ensure(property);
  *value = props[i].value[index].as_int;
  return kOfxStatOK;
}

OfxStatus propGetPointerN(OfxPropertySetHandle properties,
                          const char *property,
                          int count,
                          void **value)
{
  for (int i = 0; i < count; ++i) {
    OfxStatus status = propGetPointer(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propGetStringN(OfxPropertySetHandle properties,
                         const char *property,
                         int count,
                         char **value)
{
  for (int i = 0; i < count; ++i) {
    OfxStatus status = propGetString(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propGetDoubleN(OfxPropertySetHandle properties,
                         const char *property,
                         int count,
                         double *value)
{
  for (int i = 0; i < count; ++i) {
    OfxStatus status = propGetDouble(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propGetIntN(OfxPropertySetHandle properties, const char *property, int count, int *value)
{
  for (int i = 0; i < count; ++i) {
    OfxStatus status = propGetInt(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

OfxStatus propReset(OfxPropertySetHandle properties, const char *property)
{
  (void)properties;
  (void)property;
  return kOfxStatReplyDefault;
}

OfxStatus propGetDimension(OfxPropertySetHandle properties, const char *property, int *count)
{
  (void)properties;
  (void)property;
  *count = 4;
  return kOfxStatOK;
}
