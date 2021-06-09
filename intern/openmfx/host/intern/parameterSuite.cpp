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

#include "parameterSuite.h"
#include "parameters.h"

#include <stdarg.h>

// //Parameter Suite Entry Points

const OfxParameterSuiteV1 gParameterSuiteV1 = {
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
    /* paramEditEnd */           paramEditEnd
};

OfxStatus paramDefine(OfxParamSetHandle paramSet,
    const char* paramType,
    const char* name,
    OfxPropertySetHandle* propertySet) {
  int i = paramSet->find(name);
    if (-1 != i) {
        return kOfxStatErrExists;
    }
    i = paramSet->ensure(name);
    paramSet->parameters[i]->set_type(parse_parameter_type(paramType));
    if (NULL != propertySet) {
        *propertySet = &paramSet->parameters[i]->properties;
    }
    return kOfxStatOK;
}

OfxStatus paramGetHandle(OfxParamSetHandle paramSet,
    const char* name,
    OfxParamHandle* param,
    OfxPropertySetHandle* propertySet) {
  int i = paramSet->find(name);
    if (-1 == i) {
        return kOfxStatErrUnknown; // parameter not found
    }
    *param = paramSet->parameters[i];
    if (NULL != propertySet) {
        *propertySet = &paramSet->parameters[i]->properties;
    }
    return kOfxStatOK;
}

OfxStatus paramSetGetPropertySet(OfxParamSetHandle paramSet,
    OfxPropertySetHandle* propHandle) {
    *propHandle = paramSet->effect_properties;
    return kOfxStatOK;
}

OfxStatus paramGetPropertySet(OfxParamHandle param,
    OfxPropertySetHandle* propHandle) {
    *propHandle = &param->properties;
    return kOfxStatOK;
}

OfxStatus paramGetValue(OfxParamHandle paramHandle, ...) {
    size_t dimensions = parameter_type_dimensions(paramHandle->type);
    va_list valist;
    va_start(valist, paramHandle);
    for (size_t i = 0; i < dimensions; ++i) {
        switch (paramHandle->type) {
        case PARAM_TYPE_INTEGER:
        case PARAM_TYPE_INTEGER_2D:
        case PARAM_TYPE_INTEGER_3D:
            *va_arg(valist, int*) = paramHandle->value[i].as_int;
            break;
        case PARAM_TYPE_DOUBLE:
        case PARAM_TYPE_DOUBLE_2D:
        case PARAM_TYPE_DOUBLE_3D:
        case PARAM_TYPE_RGB:
        case PARAM_TYPE_RGBA:
            *va_arg(valist, double*) = paramHandle->value[i].as_double;
            break;
        case PARAM_TYPE_BOOLEAN:
            *va_arg(valist, bool*) = paramHandle->value[i].as_bool;
            break;
        case PARAM_TYPE_STRING:
            // TODO: check memory management
            *va_arg(valist, char**) = paramHandle->value[i].as_char;
            break;
        case PARAM_TYPE_UNKNOWN:
            // TODO
            break;
        }
    }
    va_end(valist);
    return kOfxStatOK;
}

OfxStatus paramGetValueAtTime(OfxParamHandle paramHandle, OfxTime time, ...) {
    (void)paramHandle;
    (void)time;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramGetDerivative(OfxParamHandle paramHandle, OfxTime time, ...) {
    (void)paramHandle;
    (void)time;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramGetIntegral(OfxParamHandle paramHandle, OfxTime time1, OfxTime time2, ...) {
    (void)paramHandle;
    (void)time1;
    (void)time2;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramSetValue(OfxParamHandle paramHandle, ...) {
    size_t dimensions = parameter_type_dimensions(paramHandle->type);
    va_list args;
    va_start(args, paramHandle);
    for (size_t i = 0; i < dimensions; ++i) {
        switch (paramHandle->type) {
        case PARAM_TYPE_INTEGER:
        case PARAM_TYPE_INTEGER_2D:
        case PARAM_TYPE_INTEGER_3D:
            paramHandle->value[i].as_int = va_arg(args, int);
            break;
        case PARAM_TYPE_DOUBLE:
        case PARAM_TYPE_DOUBLE_2D:
        case PARAM_TYPE_DOUBLE_3D:
        case PARAM_TYPE_RGB:
        case PARAM_TYPE_RGBA:
            paramHandle->value[i].as_double = va_arg(args, double);
            break;
        case PARAM_TYPE_BOOLEAN:
            paramHandle->value[i].as_bool = va_arg(args, bool);
            break;
        case PARAM_TYPE_STRING:
            // TODO: check memory management
            paramHandle->value[i].as_char = va_arg(args, char*);
            break;
        case PARAM_TYPE_UNKNOWN:
            // TODO
            break;
        }
    }
    va_end(args);
    return kOfxStatOK;
}

OfxStatus paramSetValueAtTime(OfxParamHandle paramHandle, OfxTime time, ...) {
    (void)paramHandle;
    (void)time;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramGetNumKeys(OfxParamHandle paramHandle, unsigned int* numberOfKeys) {
    (void)paramHandle;
    (void)numberOfKeys;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramGetKeyTime(OfxParamHandle paramHandle,
    unsigned int nthKey,
    OfxTime* time) {
    (void)paramHandle;
    (void)nthKey;
    (void)time;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramGetKeyIndex(OfxParamHandle paramHandle,
    OfxTime time,
    int direction,
    int* index) {
    (void)paramHandle;
    (void)time;
    (void)direction;
    (void)index;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramDeleteKey(OfxParamHandle paramHandle, OfxTime time) {
    (void)paramHandle;
    (void)time;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramDeleteAllKeys(OfxParamHandle paramHandle) {
    (void)paramHandle;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramCopy(OfxParamHandle paramTo,
    OfxParamHandle paramFrom,
    OfxTime dstOffset,
    const OfxRangeD* frameRange) {
    (void)paramTo;
    (void)paramFrom;
    (void)dstOffset;
    (void)frameRange;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramEditBegin(OfxParamSetHandle paramSet, const char* name) {
    (void)paramSet;
    (void)name;
    // TODO
    return kOfxStatErrUnsupported;
}

OfxStatus paramEditEnd(OfxParamSetHandle paramSet) {
    (void)paramSet;
    // TODO
    return kOfxStatErrUnsupported;
}

