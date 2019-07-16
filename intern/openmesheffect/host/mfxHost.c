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
 */

#include <string.h>
#include <stdio.h>
#include <stdarg.h>

#include "ofxProperty.h"
#include "ofxParam.h"
#include "ofxMeshEffect.h"

#include "util/ofx_util.h"
#include "util/memory_util.h"

#include "mfxHost.h"

#ifdef _WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif

// OFX PROPERTIES SUITE

// // OfxPropertySetStruct

static void deep_copy_property(OfxPropertyStruct *destination, const OfxPropertyStruct *source) {
  destination->name = source->name; // weak pointer?
  destination->value[0] = source->value[0];
  destination->value[1] = source->value[1];
  destination->value[2] = source->value[2];
  destination->value[3] = source->value[3];
}

static int find_property(OfxPropertySetHandle properties, const char *property) {
  for (int i = 0 ; i < properties->num_properties ; ++i) {
    if (0 == strcmp(properties->properties[i]->name, property)) {
      return i;
    }
  }
  return -1;
}

static void append_properties(OfxPropertySetHandle properties, int count) {
  int old_num_properties = properties->num_properties;
  OfxPropertyStruct **old_properties = properties->properties;
  properties->num_properties += count;
  properties->properties = malloc_array(sizeof(OfxPropertyStruct*), properties->num_properties, "properties");
  for (int i = 0 ; i < properties->num_properties ; ++i){
    properties->properties[i] = i < old_num_properties ? old_properties[i] : malloc_array(sizeof(OfxPropertyStruct), 1, "property");
  }
  if (NULL != old_properties) {
    free_array(old_properties);
  }
}

static int ensure_property(OfxPropertySetHandle properties, const char *property) {
  int i = find_property(properties, property);
  if (i == -1) {
    append_properties(properties, 1);
    i = properties->num_properties - 1;
    properties->properties[i]->name = property;
  }
  return i;
}

static void init_properties(OfxPropertySetHandle properties) {
  properties->num_properties = 0;
  properties->properties = NULL;
  properties->context = PROP_CTX_OTHER;
}

static void free_properties(OfxPropertySetHandle properties) {
  for (int i = 0 ; i < properties->num_properties ; ++i){
    free_array(properties->properties[i]);
  }
  properties->num_properties = 0;
  if (NULL != properties->properties) {
    free_array(properties->properties);
    properties->properties = NULL;
  }
}

static void deep_copy_property_set(OfxPropertySetStruct *destination, const OfxPropertySetStruct *source) {
  init_properties(destination);
  append_properties(destination, source->num_properties);
  for (int i = 0 ; i < destination->num_properties ; ++i) {
    deep_copy_property(destination->properties[i], source->properties[i]);
  }
  destination->context = source->context;
}

static bool check_property_context(OfxPropertySetHandle propertySet, const char *property) {
  switch (propertySet->context) {
  case PROP_CTX_MESH_EFFECT:
    return (
      0 == strcmp(property, kOfxMeshEffectPropContext) ||
      false
    );
  case PROP_CTX_INPUT:
    return (
      0 == strcmp(property, kOfxPropLabel) ||
      false
    );
  case PROP_CTX_OTHER:
  default:
    return true;
  }
}

// // Property Suite Entry Points

static OfxStatus propSetPointer(OfxPropertySetHandle properties, const char *property, int index, void *value) {
  if (false == check_property_context(properties, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = ensure_property(properties, property);
  properties->properties[i]->value[index].as_pointer = value;
  return kOfxStatOK;
}

static OfxStatus propSetString(OfxPropertySetHandle properties, const char *property, int index, const char *value) {
  if (false == check_property_context(properties, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = ensure_property(properties, property);
  properties->properties[i]->value[index].as_const_char = value;
  return kOfxStatOK;
}

static OfxStatus propSetDouble(OfxPropertySetHandle properties, const char *property, int index, double value) {
  if (false == check_property_context(properties, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = ensure_property(properties, property);
  properties->properties[i]->value[index].as_double = value;
  return kOfxStatOK;
}

static OfxStatus propSetInt(OfxPropertySetHandle properties, const char *property, int index, int value) {
  if (false == check_property_context(properties, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = ensure_property(properties, property);
  properties->properties[i]->value[index].as_int = value;
  return kOfxStatOK;
}

static OfxStatus propSetPointerN(OfxPropertySetHandle properties, const char *property, int count, void *const*value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propSetPointer(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

static OfxStatus propSetStringN(OfxPropertySetHandle properties, const char *property, int count, const char *const*value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propSetString(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

static OfxStatus propSetDoubleN(OfxPropertySetHandle properties, const char *property, int count, const double *value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propSetDouble(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

static OfxStatus propSetIntN(OfxPropertySetHandle properties, const char *property, int count, const int *value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propSetInt(properties, property, i, value[i]);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

static OfxStatus propGetPointer(OfxPropertySetHandle properties, const char *property, int index, void **value) {
  if (false == check_property_context(properties, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = ensure_property(properties, property);
  *value = properties->properties[i]->value[index].as_pointer;
  return kOfxStatOK;
}

static OfxStatus propGetString(OfxPropertySetHandle properties, const char *property, int index, char **value) {
  if (false == check_property_context(properties, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = ensure_property(properties, property);
  *value = properties->properties[i]->value[index].as_char;
  return kOfxStatOK;
}

static OfxStatus propGetDouble(OfxPropertySetHandle properties, const char *property, int index, double *value) {
  if (false == check_property_context(properties, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = ensure_property(properties, property);
  *value = properties->properties[i]->value[index].as_double;
  return kOfxStatOK;
}

static OfxStatus propGetInt(OfxPropertySetHandle properties, const char *property, int index, int *value) {
  if (false == check_property_context(properties, property)) {
    return kOfxStatErrBadHandle;
  }
  if (index < 0 || index >= 4) {
    return kOfxStatErrBadIndex;
  }
  int i = ensure_property(properties, property);
  *value = properties->properties[i]->value[index].as_int;
  return kOfxStatOK;
}

static OfxStatus propGetPointerN(OfxPropertySetHandle properties, const char *property, int count, void **value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propGetPointer(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

static OfxStatus propGetStringN(OfxPropertySetHandle properties, const char *property, int count, char **value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propGetString(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

static OfxStatus propGetDoubleN(OfxPropertySetHandle properties, const char *property, int count, double *value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propGetDouble(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

static OfxStatus propGetIntN(OfxPropertySetHandle properties, const char *property, int count, int *value) {
  for (int i = 0 ; i < count ; ++i) {
    OfxStatus status = propGetInt(properties, property, i, value + i);
    if (kOfxStatOK != status) {
      return status;
    }
  }
  return kOfxStatOK;
}

static OfxStatus propReset(OfxPropertySetHandle properties, const char *property) {
  (void)properties;
  (void)property;
  return kOfxStatReplyDefault;
}

static OfxStatus propGetDimension(OfxPropertySetHandle properties, const char *property, int *count) {
  (void)properties;
  (void)property;
  *count = 4;
  return kOfxStatOK;
}


// OFX PARAMETERS SUITE

// // OfxParamStruct

static void init_parameter(OfxParamHandle param) {
  param->type = PARAM_TYPE_DOUBLE;
  param->name = NULL;
  init_properties(&param->properties);
}

static void free_parameter(OfxParamHandle param) {
  free_properties(&param->properties);
  if (PARAM_TYPE_STRING == param->type) {
    free_array(param->value[0].as_char);
  }
  if (NULL != param->name) {
    free_array(param->name);
  }
}

void parameter_set_type(OfxParamHandle param, ParamType type) {
  if (param->type == type) {
    return;
  }

  if (PARAM_TYPE_STRING == param->type) {
    free_array(param->value[0].as_char);
    param->value[0].as_char = NULL;
  }

  param->type = type;

  if (PARAM_TYPE_STRING == param->type) {
    param->value[0].as_char = NULL;
    parameter_realloc_string(param, 1);
  }
}

void parameter_realloc_string(OfxParamHandle param, int size) {
  if (NULL != param->value[0].as_char) {
    free_array(param->value[0].as_char);
  }
  param->value[0].as_char = malloc_array(sizeof(char), size, "parameter string value");
  param->value[0].as_char[0] = '\0';
}

// // OfxParamSetStruct

static void deep_copy_parameter(OfxParamStruct *destination, const OfxParamStruct *source) {
  destination->name = source->name;
  if (NULL != destination->name) {
    destination->name = malloc_array(sizeof(char), strlen(source->name), "parameter name");
    strcpy(destination->name, source->name);
  }
  destination->type = source->type;
  destination->value[0] = source->value[0];
  destination->value[1] = source->value[1];
  destination->value[2] = source->value[2];
  destination->value[3] = source->value[3];

  // Strings are dynamically allocated, so deep copy must allocate new data
  if (destination->type == PARAM_TYPE_STRING) {
    int n = strlen(source->value[0].as_char);
    destination->value[0].as_char = NULL;
    parameter_realloc_string(destination, n + 1);
    strcpy(destination->value[0].as_char, source->value[0].as_char);
  }

  deep_copy_property_set(&destination->properties, &source->properties);
}

static int find_parameter(OfxParamSetHandle param_set, const char *param) {
  for (int i = 0 ; i < param_set->num_parameters ; ++i) {
    if (0 == strcmp(param_set->parameters[i]->name, param)) {
      return i;
    }
  }
  return -1;
}

static void append_parameters(OfxParamSetHandle param_set, int count) {
  int old_num_parameters = param_set->num_parameters;
  OfxParamStruct **old_parameters = param_set->parameters;
  param_set->num_parameters += count;
  param_set->parameters = malloc_array(sizeof(OfxParamStruct*), param_set->num_parameters, "parameters");
  for (int i = 0 ; i < param_set->num_parameters ; ++i){
    if (i < old_num_parameters) {
      param_set->parameters[i] = old_parameters[i];
    } else {
      param_set->parameters[i] = malloc_array(sizeof(OfxParamStruct), 1, "parameter");
      init_parameter(param_set->parameters[i]);
    }
  }
  if (NULL != old_parameters) {
    free_array(old_parameters);
  }
}

static int ensure_parameter(OfxParamSetHandle param_set, const char *parameter) {
  int i = find_parameter(param_set, parameter);
  if (i == -1) {
    append_parameters(param_set, 1);
    i = param_set->num_parameters - 1;
    param_set->parameters[i]->name = malloc_array(sizeof(char), strlen(parameter), "parameter name");
    strcpy(param_set->parameters[i]->name, parameter);
  }
  return i;
}

static void init_parameter_set(OfxParamSetHandle param_set) {
  param_set->num_parameters = 0;
  param_set->parameters = NULL;
  param_set->effect_properties = NULL;
}

static void free_parameter_set(OfxParamSetHandle param_set) {
  for (int i = 0 ; i < param_set->num_parameters ; ++i){
    free_parameter(param_set->parameters[i]);
    free_array(param_set->parameters[i]);
  }
  param_set->num_parameters = 0;
  if (NULL != param_set->parameters) {
    free_array(param_set->parameters);
    param_set->parameters = NULL;
  }
}

static void deep_copy_parameter_set(OfxParamSetStruct *destination, const OfxParamSetStruct *source) {
  init_parameter_set(destination);
  append_parameters(destination, source->num_parameters);
  for (int i = 0 ; i < destination->num_parameters ; ++i) {
    deep_copy_parameter(destination->parameters[i], source->parameters[i]);
  }
  destination->effect_properties = source->effect_properties;
}

// // Parameter Suite Entry Points

static ParamType parse_parameter_type(const char *str) {
  if (0 == strcmp(str, kOfxParamTypeDouble)) {
    return PARAM_TYPE_DOUBLE;
  }
  if (0 == strcmp(str, kOfxParamTypeInteger)) {
    return PARAM_TYPE_INT;
  }
  if (0 == strcmp(str, kOfxParamTypeString)) {
    return PARAM_TYPE_STRING;
  }
  return PARAM_TYPE_UNKNOWN;
}

static size_t parameter_type_dimensions(ParamType type) {
  switch (type) {
  case PARAM_TYPE_DOUBLE:
  case PARAM_TYPE_INT:
  case PARAM_TYPE_STRING:
    return 1;
  default:
    return 1;
  }
}

static OfxStatus paramDefine(OfxParamSetHandle paramSet,
                             const char *paramType,
                             const char *name,
                             OfxPropertySetHandle *propertySet) {
  int i = find_parameter(paramSet, name);
  if (-1 != i) {
    return kOfxStatErrExists;
  }
  i = ensure_parameter(paramSet, name);
  parameter_set_type(paramSet->parameters[i], parse_parameter_type(paramType));
  if (NULL != propertySet) {
    *propertySet = &paramSet->parameters[i]->properties;
  }
  return kOfxStatOK;
}

static OfxStatus paramGetHandle(OfxParamSetHandle paramSet,
                                const char *name,
                                OfxParamHandle *param,
                                OfxPropertySetHandle *propertySet) {
  int i = find_parameter(paramSet, name);
  if (-1 == i) {
    return kOfxStatErrUnknown; // parameter not found
  }
  *param = paramSet->parameters[i];
  if (NULL !=  propertySet) {
    *propertySet = &paramSet->parameters[i]->properties;
  }
  return kOfxStatOK;
}

static OfxStatus paramSetGetPropertySet(OfxParamSetHandle paramSet,
                                        OfxPropertySetHandle *propHandle) {
  *propHandle = paramSet->effect_properties;
  return kOfxStatOK;
}

static OfxStatus paramGetPropertySet(OfxParamHandle param,
                                     OfxPropertySetHandle *propHandle) {
  *propHandle = &param->properties;
  return kOfxStatOK;
}

static OfxStatus paramGetValue(OfxParamHandle paramHandle, ...) {
  size_t dimensions = parameter_type_dimensions(paramHandle->type);
  va_list valist;
  va_start(valist, paramHandle);
  for (size_t i = 0 ; i < dimensions ; ++i) {
    switch (paramHandle->type) {
    case PARAM_TYPE_DOUBLE:
      *va_arg(valist, double*) = paramHandle->value[i].as_double;
      break;
    case PARAM_TYPE_INT:
      *va_arg(valist, int*) = paramHandle->value[i].as_int;
      break;
    }
  }
  va_end(valist);
  return kOfxStatOK;
}

static OfxStatus paramGetValueAtTime(OfxParamHandle paramHandle, OfxTime time, ...) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramGetDerivative(OfxParamHandle paramHandle, OfxTime time, ...) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramGetIntegral(OfxParamHandle paramHandle, OfxTime time1, OfxTime time2, ...) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramSetValue(OfxParamHandle paramHandle, ...) {
  size_t dimensions = parameter_type_dimensions(paramHandle->type);
  va_list args;
  va_start(args, paramHandle);
  for (size_t i = 0 ; i < dimensions ; ++i) {
    switch (paramHandle->type) {
    case PARAM_TYPE_DOUBLE:
    {
      double d = va_arg(args, double);
      printf("double: %f\n", d);
      paramHandle->value[i].as_double = d;
      break;
    }
    case PARAM_TYPE_INT:
      paramHandle->value[i].as_int = va_arg(args, int);
      break;
    }
  }
  va_end(args);
  return kOfxStatOK;
}

static OfxStatus paramSetValueAtTime(OfxParamHandle  paramHandle, OfxTime time, ...) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramGetNumKeys(OfxParamHandle  paramHandle, unsigned int  *numberOfKeys) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramGetKeyTime(OfxParamHandle  paramHandle,
                                 unsigned int nthKey,
                                 OfxTime *time) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramGetKeyIndex(OfxParamHandle  paramHandle,
                                  OfxTime time,
                                  int     direction,
                                  int    *index) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramDeleteKey(OfxParamHandle  paramHandle, OfxTime time) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramDeleteAllKeys(OfxParamHandle  paramHandle) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramCopy(OfxParamHandle  paramTo, OfxParamHandle  paramFrom, OfxTime dstOffset, const OfxRangeD *frameRange) {
  return kOfxStatErrUnsupported;
}

static OfxStatus paramEditBegin(OfxParamSetHandle paramSet, const char *name) {
  return kOfxStatErrUnsupported;
}
 
static OfxStatus paramEditEnd(OfxParamSetHandle paramSet) {
  return kOfxStatErrUnsupported;
}

// OFX MESH EFFECT SUITE

// // OfxInputStruct

static void init_input(OfxMeshInputStruct *input) {
  init_properties(&input->properties);
  input->properties.context = PROP_CTX_INPUT;
  init_properties(&input->mesh);
  input->mesh.context = PROP_CTX_MESH;
}

static void free_input(OfxMeshInputStruct *input) {
  free_properties(&input->properties);
  free_properties(&input->mesh);
  free_array(input);
}

static void deep_copy_input(OfxMeshInputStruct *destination, const OfxMeshInputStruct *source) {
  destination->name = source->name; // weak pointer?
  deep_copy_property_set(&destination->properties, &source->properties);
  deep_copy_property_set(&destination->mesh, &source->mesh);
  destination->host = source->host; // not deep copied, as this is a weak pointer
}

// // OfxInputSetStruct

static int find_input(const OfxMeshInputSetStruct *input_set, const char *input) {
  for (int i = 0 ; i < input_set->num_inputs ; ++i) {
    if (0 == strcmp(input_set->inputs[i]->name, input)) {
      return i;
    }
  }
  return -1;
}

static void append_inputs(OfxMeshInputSetStruct *input_set, int count) {
  int old_num_input = input_set->num_inputs;
  OfxMeshInputStruct **old_inputs = input_set->inputs;
  input_set->num_inputs += count;
  input_set->inputs = malloc_array(sizeof(OfxMeshInputStruct*), input_set->num_inputs, "inputs");
  for (int i = 0 ; i < input_set->num_inputs ; ++i){
    OfxMeshInputStruct *input;
    if (i < old_num_input) {
      input = old_inputs[i];
    } else {
      input = malloc_array(sizeof(OfxMeshInputStruct), 1, "input");
      input->host = input_set->host;
      init_input(input);
    }
    input_set->inputs[i] = input;
  }
  if (NULL != old_inputs) {
    free_array(old_inputs);
  }
}

static int ensure_input(OfxMeshInputSetStruct *input_set, const char *input) {
  int i = find_input(input_set, input);
  if (i == -1) {
    append_inputs(input_set, 1);
    i = input_set->num_inputs - 1;
    input_set->inputs[i]->name = input;
  }
  return i;
}

static void init_input_set(OfxMeshInputSetStruct *input_set) {
  input_set->num_inputs = 0;
  input_set->inputs = NULL;
}

static void free_input_set(OfxMeshInputSetStruct *input_set) {
  for (int i = 0 ; i < input_set->num_inputs ; ++i){
    free_input(input_set->inputs[i]);
  }
  input_set->num_inputs = 0;
  if (NULL != input_set->inputs) {
    free_array(input_set->inputs);
    input_set->inputs = NULL;
  }
}

static void deep_copy_input_set(OfxMeshInputSetStruct *destination, const OfxMeshInputSetStruct *source) {
  init_input_set(destination);
  append_inputs(destination, source->num_inputs);
  for (int i = 0 ; i < destination->num_inputs ; ++i) {
    deep_copy_input(destination->inputs[i], source->inputs[i]);
  }
  destination->host = source->host; // not deep copied, as this is a weak pointer
}

// // Mesh Effect

static void init_mesh_effect(OfxMeshEffectHandle meshEffectHandle) {
  meshEffectHandle->inputs.host = meshEffectHandle->host;
  init_input_set(&meshEffectHandle->inputs);
  init_properties(&meshEffectHandle->properties);
  init_parameter_set(&meshEffectHandle->parameters);
  meshEffectHandle->parameters.effect_properties = &meshEffectHandle->properties;
  meshEffectHandle->properties.context = PROP_CTX_MESH_EFFECT;
}

static void free_mesh_effect(OfxMeshEffectHandle meshEffectHandle) {
  free_input_set(&meshEffectHandle->inputs);
  free_properties(&meshEffectHandle->properties);
  free_parameter_set(&meshEffectHandle->parameters);
}

static void deep_copy_mesh_effect(OfxMeshEffectStruct *destination, const OfxMeshEffectStruct *source) {
  deep_copy_input_set(&destination->inputs, &source->inputs);
  deep_copy_property_set(&destination->properties, &source->properties);
  deep_copy_parameter_set(&destination->parameters, &source->parameters);
  destination->parameters.effect_properties = &destination->properties;
  destination->host = source->host; // not deep copied, as this is a weak pointer
}

// // Mesh Effect Suite Entry Points

static OfxStatus getPropertySet(OfxMeshEffectHandle meshEffect,
                                OfxPropertySetHandle *propHandle) {
  *propHandle = &meshEffect->properties;
  return kOfxStatOK;
}

static OfxStatus getParamSet(OfxMeshEffectHandle meshEffect,
                             OfxParamSetHandle *paramSet) {
  *paramSet = &meshEffect->parameters;
  return kOfxStatOK;
}

static OfxStatus inputDefine(OfxMeshEffectHandle meshEffect,
                             const char *name,
                             OfxPropertySetHandle *propertySet) {
  printf("Defining input '%s' on OfxMeshEffectHandle %p\n", name, meshEffect);
  int i = ensure_input(&meshEffect->inputs, name);
  *propertySet = &(meshEffect->inputs.inputs[i]->properties);
  return kOfxStatOK;
}

static OfxStatus inputGetHandle(OfxMeshEffectHandle meshEffect,
                                const char *name,
                                OfxMeshInputHandle *input,
                                OfxPropertySetHandle *propertySet) {
  int i = find_input(&meshEffect->inputs, name);
  if (-1 == i) {
    return kOfxStatErrUnknown; // bad name
  }
  *input = meshEffect->inputs.inputs[i];
  *propertySet = &(meshEffect->inputs.inputs[i]->properties);
  return kOfxStatOK;
}

static OfxStatus inputGetPropertySet(OfxMeshInputHandle input,
                                     OfxPropertySetHandle *propHandle) {
  *propHandle = &input->properties;
  return kOfxStatOK;
}

static OfxStatus inputGetMesh(OfxMeshInputHandle input,
                              OfxTime time,
                              OfxPropertySetHandle *meshHandle) {
  (void)time;
  OfxPropertySetHandle inputMeshHandle = &input->mesh;
  propSetPointer(inputMeshHandle, kOfxMeshPropInternalData, 0, NULL); // TODO: get this from input
  propSetPointer(inputMeshHandle, kOfxMeshPropHostHandle, 0, (void*)input->host);
  if (NULL == NULL) { // callback to get data from internal pointer
    propSetInt(inputMeshHandle, kOfxMeshPropPointCount, 0, 0);
    propSetInt(inputMeshHandle, kOfxMeshPropVertexCount, 0, 0);
    propSetInt(inputMeshHandle, kOfxMeshPropFaceCount, 0, 0);
    propSetPointer(inputMeshHandle, kOfxMeshPropPointData, 0, NULL);
    propSetPointer(inputMeshHandle, kOfxMeshPropVertexData, 0, NULL);
    propSetPointer(inputMeshHandle, kOfxMeshPropFaceData, 0, NULL);
  } else {
    // TODO
  }

  *meshHandle = inputMeshHandle;

  return kOfxStatOK;
}

static OfxStatus inputReleaseMesh(OfxPropertySetHandle meshHandle) {
  float *pointData;
  int *vertexData, *faceData;

  // Call internal callback before actually releasing data
  OfxHost *host;
  BeforeMeshReleaseCbFunc beforeMeshReleaseCb;
  propGetPointer(meshHandle, kOfxMeshPropHostHandle, 0, (void**)&host);
  if (NULL != host) {
    propGetPointer(host->host, kOfxHostPropBeforeMeshReleaseCb, 0, (void**)&beforeMeshReleaseCb);
    if (NULL != beforeMeshReleaseCb) {
      beforeMeshReleaseCb(host, meshHandle);
    }
  }

  propGetPointer(meshHandle, kOfxMeshPropPointData, 0, (void**)&pointData);
  propGetPointer(meshHandle, kOfxMeshPropVertexData, 0, (void**)&vertexData);
  propGetPointer(meshHandle, kOfxMeshPropFaceData, 0, (void**)&faceData);
  
  if (NULL != pointData) {
    free_array(pointData);
  }
  if (NULL != vertexData) {
    free_array(vertexData);
  }
  if (NULL != faceData) {
    free_array(faceData);
  }

  propSetInt(meshHandle, kOfxMeshPropPointCount, 0, 0);
  propSetInt(meshHandle, kOfxMeshPropVertexCount, 0, 0);
  propSetInt(meshHandle, kOfxMeshPropFaceCount, 0, 0);
  propSetPointer(meshHandle, kOfxMeshPropPointData, 0, NULL);
  propSetPointer(meshHandle, kOfxMeshPropVertexData, 0, NULL);
  propSetPointer(meshHandle, kOfxMeshPropFaceData, 0, NULL);

  return kOfxStatOK;
}

static OfxStatus meshAlloc(OfxPropertySetHandle meshHandle,
                           int pointCount,
                           int vertexCount,
                           int faceCount) {
  OfxStatus status;

  float *pointData = malloc_array(sizeof(float) * 3, pointCount, "point data");
  if (NULL == pointData) {
    return kOfxStatErrMemory;
  }

  int *vertexData = malloc_array(sizeof(int), vertexCount, "vertex data");
  if (NULL == vertexData) {
    free_array(pointData);
    return kOfxStatErrMemory;
  }

  int *faceData = malloc_array(sizeof(int) * 3, faceCount, "face data");
  if (NULL == faceData) {
    free_array(pointData);
    free_array(vertexData);
    return kOfxStatErrMemory;
  }

  status = propSetInt(meshHandle, kOfxMeshPropPointCount, 0, pointCount);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetInt(meshHandle, kOfxMeshPropVertexCount, 0, vertexCount);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetInt(meshHandle, kOfxMeshPropFaceCount, 0, faceCount);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetPointer(meshHandle, kOfxMeshPropPointData, 0, pointData);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetPointer(meshHandle, kOfxMeshPropVertexData, 0, vertexData);
  if (kOfxStatOK != status) {
    return status;
  }
  status = propSetPointer(meshHandle, kOfxMeshPropFaceData, 0, faceData);
  if (kOfxStatOK != status) {
    return status;
  }
  return kOfxStatOK;
}

static int ofxAbort(OfxMeshEffectHandle meshEffect) {
  (void)meshEffect;
  return 0;
}

// OFX SUITES MAIN

static const void * fetchSuite(OfxPropertySetHandle host,
                               const char *suiteName,
                               int suiteVersion) {

  static const OfxMeshEffectSuiteV1 meshEffectSuiteV1 = {
    /* getPropertySet */      getPropertySet,
    /* getParamSet */         getParamSet,
    /* inputDefine */         inputDefine,
    /* inputGetHandle */      inputGetHandle,
    /* inputGetPropertySet */ inputGetPropertySet,
    /* inputGetMesh */        inputGetMesh,
    /* inputReleaseMesh */    inputReleaseMesh,
    /* meshAlloc */           meshAlloc,
    /* abort */               ofxAbort
  };

  static const OfxParameterSuiteV1 parameterSuiteV1 = {
    /* paramDefine */            paramDefine,
    /* paramGetHandle */         paramGetHandle,
    /* paramSetGetPropertySet */ paramSetGetPropertySet,
    /* paramGetPropertySet */    paramGetPropertySet,
    /* paramGetValue */          paramGetValue,
    /* paramGetValueAtTime */    paramGetValueAtTime,
    /* paramGetDerivative */     paramGetDerivative,
    /* paramGetIntegral */       paramGetIntegral,
    /* paramSetValue */          paramSetValue,
    /* paramSetValueAtTime */    paramSetValueAtTime,
    /* paramGetNumKeys */        paramGetNumKeys,
    /* paramGetKeyTime */        paramGetKeyTime,
    /* paramGetKeyIndex */       paramGetKeyIndex,
    /* paramDeleteKey */         paramDeleteKey,
    /* paramDeleteAllKeys */     paramDeleteAllKeys,
    /* paramCopy */              paramCopy,
    /* paramEditBegin */         paramEditBegin,
    /* paramDeleteKey */         paramDeleteKey
  };

  static const OfxPropertySuiteV1 propertySuiteV1 = {
    /* propSetPointer */   propSetPointer,
    /* propSetString */    propSetString,
    /* propSetDouble */    propSetDouble,
    /* propSetInt */       propSetInt,
    /* propSetPointerN */  propSetPointerN,
    /* propSetStringN */   propSetStringN,
    /* propSetDoubleN */   propSetDoubleN,
    /* propSetIntN */      propSetIntN,
    /* propGetPointer */   propGetPointer,
    /* propGetString */    propGetString,
    /* propGetDouble */    propGetDouble,
    /* propGetInt */       propGetInt,
    /* propGetPointerN */  propGetPointerN,
    /* propGetStringN */   propGetStringN,
    /* propGetDoubleN */   propGetDoubleN,
    /* propGetIntN */      propGetIntN,
    /* propReset */        propReset,
    /* propGetDimension */ propGetDimension
  };

  (void)host; // TODO: check host?

  if (0 == strcmp(suiteName, kOfxMeshEffectSuite) && suiteVersion == 1) {
    return &meshEffectSuiteV1;
  }
  if (0 == strcmp(suiteName, kOfxParameterSuite) && suiteVersion == 1) {
    return &parameterSuiteV1;
  }
  if (0 == strcmp(suiteName, kOfxPropertySuite) && suiteVersion == 1) {
    return &propertySuiteV1;
  }
  return NULL;
}

// OFX MESH EFFECT HOST

typedef void (*OfxSetBundleDirectoryFunc)(const char *path);
typedef int (*OfxGetNumberOfPluginsFunc)(void);
typedef OfxPlugin *(*OfxGetPluginFunc)(int nth);

#ifdef _WIN32
LPVOID getLastErrorMessage() {
  LPVOID msg;
  FormatMessage(
    FORMAT_MESSAGE_ALLOCATE_BUFFER |
    FORMAT_MESSAGE_FROM_SYSTEM |
    FORMAT_MESSAGE_IGNORE_INSERTS,
    NULL,
    GetLastError(),
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
    (LPTSTR)&msg,
    0, NULL);
  return msg;
}
#endif // _WIN32

bool load_registry(PluginRegistry *registry, const char *ofx_filepath) {
#ifdef _WIN32
  FARPROC proc;
#else
  void *proc;
#endif
  OfxGetNumberOfPluginsFunc fOfxGetNumberOfPlugins;
  OfxGetPluginFunc fOfxGetPlugin;
  OfxSetBundleDirectoryFunc fOfxSetBundleDirectory;

  // Init registry
  registry->num_plugins = 0;
  registry->plugins = NULL;
  registry->status = NULL;
#ifdef _WIN32
  registry->hinstance = NULL; 
#else
  registry->handle = NULL;
#endif
  
  // Open ofx binary
#ifdef _WIN32
  registry->hinstance = LoadLibrary(TEXT(ofx_filepath));
  if (NULL == registry->hinstance) {
    LPVOID msg = getLastErrorMessage();
    printf("mfxHost: Unable to load plugin binary at path %s. LoadLibrary returned: %s\n", ofx_filepath, msg);
    LocalFree(msg);
    return false;
  }
#else
  registry->handle = dlopen(ofx_filepath, RTLD_LAZY | RTLD_LOCAL);
  if (NULL == registry->handle) {
    printf("mfxHost: Unable to load plugin binary at path %s. dlopen returned: %s\n", ofx_filepath, get_error());
    return false;
  }
#endif

#ifdef _WIN32
  proc = GetProcAddress(registry->hinstance, "OfxGetNumberOfPlugins");
#else
  proc = dlsym(registry->handle, "OfxGetNumberOfPlugins");
#endif
  fOfxGetNumberOfPlugins = (OfxGetNumberOfPluginsFunc)proc;

  if (NULL == proc) {
    printf("mfxHost: Unable to load symbol 'OfxGetNumberOfPlugins' from %s. ", ofx_filepath);
#ifdef _WIN32
    LPVOID msg = getLastErrorMessage();
    printf("GetProcAddress returned: %s\n", msg);
    LocalFree(msg);
#else
    printf("dlsym returned: %s\n", dlerror());
#endif
    free_registry(registry);
    return false;
  }

#ifdef _WIN32
  proc = GetProcAddress(registry->hinstance, "OfxGetPlugin");
#else
  proc = dlsym(registry->handle, "OfxGetPlugin");
#endif
  fOfxGetPlugin = (OfxGetPluginFunc)proc;

  if (NULL == proc) {
    printf("mfxHost: Unable to load symbol 'OfxGetPlugin' from %s. ", ofx_filepath);
#ifdef _WIN32
    LPVOID msg = getLastErrorMessage();
    printf("GetProcAddress returned: %s\n", msg);
    LocalFree(msg);
#else
    printf("dlsym returned: %s\n", dlerror());
#endif
    free_registry(registry);
    return false;
  }
  
#ifdef _WIN32
  proc = GetProcAddress(registry->hinstance, "OfxSetBundleDirectory");
#else
  proc = dlsym(registry->handle, "OfxSetBundleDirectory");
#endif
  fOfxSetBundleDirectory = (OfxSetBundleDirectoryFunc)proc;
  if (NULL == proc) {
    printf("mfxHost: Unable to load symbol 'OfxSetBundleDirectory' from %s. ", ofx_filepath);
#ifdef _WIN32
    LPVOID msg = getLastErrorMessage();
    printf("GetProcAddress returned: %s\n", msg);
    LocalFree(msg);
#else
    printf("dlsym returned: %s\n", dlerror());
#endif
  }

  {
    if (NULL != fOfxSetBundleDirectory) {
      char *bundle_directory = malloc_array(sizeof(char), strlen(ofx_filepath) + 1, "bundle directory");
      strcpy(bundle_directory, ofx_filepath);
      *strrchr(bundle_directory, '\\') = '\0'; // TODO: Forward slash
      fOfxSetBundleDirectory(bundle_directory);
    }
  }

  {
    int i, j, n;
    n = fOfxGetNumberOfPlugins();
    printf("Found %d plugins in %s.\n", n, ofx_filepath);

    // 1. Count number of supported plugins
    registry->num_plugins = 0;
    for (i = 0 ; i < n ; ++i) {
      OfxPlugin *plugin;
      plugin = fOfxGetPlugin(i);
      printf("Plugin #%d: %s (API %s, version %d)\n", i, plugin->pluginIdentifier, plugin->pluginApi, plugin->apiVersion);

      // API/Version check
      if (0 != strcmp(plugin->pluginApi, kOfxMeshEffectPluginApi)) {
        printf("Unsupported plugin API: %s (expected %s)", plugin->pluginApi, kOfxMeshEffectPluginApi);
        continue;
      }
      if (plugin->apiVersion != kOfxMeshEffectPluginApiVersion) {
        printf("Plugin API version mismatch: %d found, but %d expected", plugin->apiVersion, kOfxMeshEffectPluginApiVersion);
        continue;
      }

      ++registry->num_plugins;
    }

    printf("Found %d supported plugins in %s.\n", registry->num_plugins, ofx_filepath);

    if (registry->num_plugins > 0) {
      registry->plugins = malloc_array(sizeof(OfxPlugin*), registry->num_plugins, "mfx plugins");
      registry->status = malloc_array(sizeof(OfxPluginStatus), registry->num_plugins, "mfx plugins status");
    }

    for (i = 0, j = 0 ; i < n ; ++i) {
      OfxPlugin *plugin;
      plugin = fOfxGetPlugin(i);
      
      if (0 != strcmp(plugin->pluginApi, kOfxMeshEffectPluginApi)
        || plugin->apiVersion != kOfxMeshEffectPluginApiVersion) {
        continue;
      }
      
      printf("Plugin #%d in binary is #%d in plugin registry\n", i, j);
      registry->plugins[j] = plugin;
      registry->status[j] = OfxPluginStatNotLoaded; // will be loaded later on when needed
      ++j;
    }
  }

  return true;
}

void free_registry(PluginRegistry *registry) {
  registry->num_plugins = 0;
  if (NULL != registry->plugins) {
    free_array(registry->plugins);
    registry->plugins = NULL;
  }
  if (NULL != registry->status) {
    free_array(registry->status);
    registry->status = NULL;
  }
#ifdef _WIN32
  if (NULL != registry->hinstance) {
    FreeLibrary(registry->hinstance);
    registry->hinstance = NULL;
  }
#else
  if (NULL != registry->handle) {
    dlclose(registry->handle);
    registry->handle = NULL;
  }
#endif
}

OfxHost *gHost = NULL;
int gHostUse = 0;

OfxHost * getGlobalHost(void) {
  if (0 == gHostUse) {
    gHost = malloc_array(sizeof(OfxHost), 1, "global host");
    OfxPropertySetHandle hostProperties = malloc_array(sizeof(OfxPropertySetStruct), 1, "global host properties");
    init_properties(hostProperties);
    hostProperties->context = PROP_CTX_HOST;
    propSetPointer(hostProperties, kOfxHostPropBeforeMeshReleaseCb, 0, (void*)NULL);
    gHost->host = hostProperties;
    gHost->fetchSuite = fetchSuite;
  }
  ++gHostUse;
  return gHost;
}

void releaseGlobalHost(void) {
  if (--gHostUse == 0) {
    free_array(gHost->host);
    free_array(gHost);
    gHost = NULL;
  }
}

bool ofxhost_load_plugin(OfxHost *host, OfxPlugin *plugin) {
  OfxStatus status;

  plugin->setHost(host);

  status = plugin->mainEntry(kOfxActionLoad, NULL, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionLoad, status, getOfxStateName(status));

  if (kOfxStatReplyDefault == status) {
    printf("WARNING: The plugin '%s' ignored load action.\n", plugin->pluginIdentifier);
  }
  if (kOfxStatFailed == status) {
    printf("ERROR: The load action failed, no further actions will be passed to the plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while loading the plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  return true;
}

void ofxhost_unload_plugin(OfxPlugin *plugin) {
  OfxStatus status;
  
  status = plugin->mainEntry(kOfxActionUnload, NULL, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionUnload, status, getOfxStateName(status));

  if (kOfxStatReplyDefault == status) {
    printf("WARNING: The plugin '%s' ignored unload action.\n", plugin->pluginIdentifier);
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while unloading the plug-in '%s'.\n", plugin->pluginIdentifier);
  }

  plugin->setHost(NULL);
}

bool ofxhost_get_descriptor(OfxHost *host, OfxPlugin *plugin, OfxMeshEffectHandle *effectDescriptor) {
  OfxStatus status;
  OfxMeshEffectHandle effectHandle;

  *effectDescriptor = NULL;
  effectHandle = malloc_array(sizeof(OfxMeshEffectStruct), 1, "mesh effect descriptor");

  effectHandle->host = host;
  init_mesh_effect(effectHandle);

  status = plugin->mainEntry(kOfxActionDescribe, effectHandle, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionDescribe, status, getOfxStateName(status));

  if (kOfxStatErrMissingHostFeature == status) {
    printf("ERROR: The plugin '%s' lacks some host feature.\n", plugin->pluginIdentifier); // see message
    return false;
  }
  if (kOfxStatErrMemory == status) {
    printf("ERROR: Not enough memory for plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  if (kOfxStatFailed == status) {
    printf("ERROR: Error while describing plug-in '%s'.\n", plugin->pluginIdentifier); // see message
    return false;
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while describing plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }

  *effectDescriptor = effectHandle;

  return true;
}

void ofxhost_release_descriptor(OfxMeshEffectHandle effectDescriptor) {
  free_mesh_effect(effectDescriptor);
  free_array(effectDescriptor);
}

bool ofxhost_create_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectDescriptor, OfxMeshEffectHandle *effectInstance) {
  OfxStatus status;
  OfxMeshEffectHandle instance;

  *effectInstance = NULL;

  instance = malloc_array(sizeof(OfxMeshEffectStruct), 1, "mesh effect descriptor");
  deep_copy_mesh_effect(instance, effectDescriptor);

  status = plugin->mainEntry(kOfxActionCreateInstance, instance, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionCreateInstance, status, getOfxStateName(status));

  if (kOfxStatErrMemory == status) {
    printf("ERROR: Not enough memory for plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  if (kOfxStatFailed == status) {
    printf("ERROR: Error while creating an instance of plug-in '%s'.\n", plugin->pluginIdentifier); // see message
    return false;
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while creating an instance of plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }

  *effectInstance = instance;

  return true;
}

void ofxhost_destroy_instance(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance) {
  OfxStatus status;

  status = plugin->mainEntry(kOfxActionDestroyInstance, effectInstance, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxActionDestroyInstance, status, getOfxStateName(status));

  if (kOfxStatFailed == status) {
    printf("ERROR: Error while destroying an instance of plug-in '%s'.\n", plugin->pluginIdentifier); // see message
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while destroying an instance of plug-in '%s'.\n", plugin->pluginIdentifier);
  }

  free_mesh_effect(effectInstance);
  free_array(effectInstance);
}

bool ofxhost_cook(OfxPlugin *plugin, OfxMeshEffectHandle effectInstance) {
  OfxStatus status;

  status = plugin->mainEntry(kOfxMeshEffectActionCook, effectInstance, NULL, NULL);
  printf("%s action returned status %d (%s)\n", kOfxMeshEffectActionCook, status, getOfxStateName(status));

  if (kOfxStatErrMemory == status) {
    printf("ERROR: Not enough memory for plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  if (kOfxStatFailed == status) {
    printf("ERROR: Error while cooking an instance of plug-in '%s'.\n", plugin->pluginIdentifier); // see message
    return false;
  }
  if (kOfxStatErrFatal == status) {
    printf("ERROR: Fatal error while cooking an instance of plug-in '%s'.\n", plugin->pluginIdentifier);
    return false;
  }
  return true;
}

bool use_plugin(const PluginRegistry *registry, int plugin_index) {
  OfxPlugin *plugin = registry->plugins[plugin_index];
  printf("Using plugin #%d: %s\n", plugin_index, plugin->pluginIdentifier);

  // Set host (TODO: do this in load_plugins?)
  OfxHost *host = getGlobalHost();

  // Load action if not loaded yet
  if (OfxPluginStatNotLoaded == registry->status[plugin_index]) {
    if (ofxhost_load_plugin(host, plugin)) {
      registry->status[plugin_index] = OfxPluginStatOK;
    } else {
      registry->status[plugin_index] = OfxPluginStatError;
      return false;
    }
  }

  if (OfxPluginStatError == registry->status[plugin_index]) {
    return false;
  }

  // Describe action
  OfxMeshEffectHandle effectDescriptor;
  if (ofxhost_get_descriptor(host, plugin, &effectDescriptor)) {
    OfxMeshEffectHandle effectInstance;

    // DEBUG
    printf("After describing effect:\n");
    printf("  Found %d inputs:\n", effectDescriptor->inputs.num_inputs);
    for (int i = 0 ; i < effectDescriptor->inputs.num_inputs ; ++i) {
      printf("    #%d: %s\n", i, effectDescriptor->inputs.inputs[i]->name);
    }

    // Create Instance action
    if (ofxhost_create_instance(plugin, effectDescriptor, &effectInstance)) {
      ofxhost_cook(plugin, effectInstance);
      ofxhost_destroy_instance(plugin, effectInstance);
    }
    ofxhost_release_descriptor(effectDescriptor);
  }

  // Unload action (TODO: move into e.g. free_registry)
  //ofxhost_unload_plugin(plugin);
  //releaseGlobalHost();

  return true;
}
