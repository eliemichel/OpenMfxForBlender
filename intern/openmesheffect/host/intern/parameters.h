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

#ifndef __MFX_PARAMETERS_H__
#define __MFX_PARAMETERS_H__

#include "properties.h"

typedef union OfxParamValueStruct {
    void *as_pointer;
    const char *as_const_char;
    char *as_char;
    double as_double;
    int as_int;
} OfxParamValueStruct;

typedef enum ParamType {
    PARAM_TYPE_UNKNOWN = -1,
    PARAM_TYPE_DOUBLE,
    PARAM_TYPE_INT,
    PARAM_TYPE_STRING,
} ParamType;

typedef struct OfxParamStruct {
    char *name;
    OfxParamValueStruct value[4];
    ParamType type;
    OfxPropertySetStruct properties;
} OfxParamStruct;

typedef struct OfxParamSetStruct {
    int num_parameters;
    OfxParamStruct **parameters;
    OfxPropertySetStruct *effect_properties; // weak pointer
} OfxParamSetStruct;

// // OfxParamStruct

void init_parameter(OfxParamStruct *param);
void free_parameter(OfxParamStruct *param);
void parameter_set_type(OfxParamStruct *param, ParamType type); // public
void parameter_realloc_string(OfxParamStruct *param, int size); // public

// // OfxParamSetStruct

void deep_copy_parameter(OfxParamStruct *destination, const OfxParamStruct *source);
int find_parameter(OfxParamSetStruct *param_set, const char *param);
void append_parameters(OfxParamSetStruct *param_set, int count);
int ensure_parameter(OfxParamSetStruct *param_set, const char *parameter);
void init_parameter_set(OfxParamSetStruct *param_set);
void free_parameter_set(OfxParamSetStruct *param_set);
void deep_copy_parameter_set(OfxParamSetStruct *destination, const OfxParamSetStruct *source);

// // Utils

ParamType parse_parameter_type(const char *str);
size_t parameter_type_dimensions(ParamType type);

// // Parameter Suite Entry Points

#include "ofxParam.h"

extern const OfxParameterSuiteV1 gParameterSuiteV1;

// See ofxParam.h for docstrings

OfxStatus paramDefine(OfxParamSetHandle paramSet,
	                  const char *paramType,
                      const char *name,
                      OfxPropertySetHandle *propertySet);
OfxStatus paramGetHandle(OfxParamSetHandle paramSet,
                         const char *name,
                         OfxParamHandle *param,
                         OfxPropertySetHandle *propertySet);
OfxStatus paramSetGetPropertySet(OfxParamSetHandle paramSet,
                                 OfxPropertySetHandle *propHandle);
OfxStatus paramGetPropertySet(OfxParamHandle param,
                              OfxPropertySetHandle *propHandle);
OfxStatus paramGetValue(OfxParamHandle paramHandle, ...);
OfxStatus paramGetValueAtTime(OfxParamHandle paramHandle, OfxTime time, ...);
OfxStatus paramGetDerivative(OfxParamHandle paramHandle, OfxTime time, ...);
OfxStatus paramGetIntegral(OfxParamHandle paramHandle, OfxTime time1, OfxTime time2, ...);
OfxStatus paramSetValue(OfxParamHandle paramHandle, ...);
OfxStatus paramSetValueAtTime(OfxParamHandle paramHandle, OfxTime time, ...);
OfxStatus paramGetNumKeys(OfxParamHandle paramHandle, unsigned int *numberOfKeys);
OfxStatus paramGetKeyTime(OfxParamHandle paramHandle,
                          unsigned int nthKey,
                          OfxTime *time);
OfxStatus paramGetKeyIndex(OfxParamHandle paramHandle,
                           OfxTime time,
                           int direction,
                           int *index);
OfxStatus paramDeleteKey(OfxParamHandle paramHandle, OfxTime time);
OfxStatus paramDeleteAllKeys(OfxParamHandle paramHandle);
OfxStatus paramCopy(OfxParamHandle paramTo,
                    OfxParamHandle paramFrom,
                    OfxTime dstOffset,
                    const OfxRangeD *frameRange);
OfxStatus paramEditBegin(OfxParamSetHandle paramSet, const char *name);
OfxStatus paramEditEnd(OfxParamSetHandle paramSet);

#endif // __MFX_PARAMETERS_H__
