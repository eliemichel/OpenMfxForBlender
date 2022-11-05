#include "parameterSuite.h"
#include "types.h"

#include <string.h>
#include <stdarg.h>
#include <stdio.h>

OfxStatus paramDefine(OfxParamSetHandle paramSet,
                      const char *paramType,
                      const char *name,
                      OfxPropertySetHandle *propertySet)
{
  printf("[host] paramDefine(paramSet %p, %s, %s)\n", paramSet, paramType, name);
  int param_index = 0;
  while (paramSet->entries[param_index].is_valid) ++param_index;
  if (param_index == 16) return kOfxStatErrMemory;
  OfxParamHandle param = &paramSet->entries[param_index];

  paramInit(param);
  strncpy(param->type, paramType, 64);
  strncpy(param->name, name, 64);

  if (NULL != propertySet) {
    *propertySet = (OfxPropertySetHandle)&param->properties;
  }

  return kOfxStatOK;
}

OfxStatus paramGetHandle(OfxParamSetHandle paramSet,
                         const char *name,
                         OfxParamHandle *paramHandle,
                         OfxPropertySetHandle *propertySet)
{
  printf("[host] paramGetHandle(paramSet %p, %s)\n", paramSet, name);
  for (int i = 0 ; i < 16 && paramSet->entries[i].is_valid ; ++i) {
    OfxParamHandle param = &paramSet->entries[i];
    if (0 == strncmp(name, param->name, 64)) {
      *paramHandle = param;
      if (NULL != propertySet) {
        *propertySet = (OfxPropertySetHandle)&param->properties;
      }
      return kOfxStatOK;
    }
  }
  return kOfxStatErrBadHandle;
}

OfxStatus paramGetValue(OfxParamHandle paramHandle, ...) {
  va_list argp;
  va_start(argp, paramHandle);

  if (0 == strncmp(paramHandle->type, kOfxParamTypeInteger, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeDouble, 64)) {
    double *value = va_arg(argp, double*);
    *value = paramHandle->values[0].as_double;
    return kOfxStatOK;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeBoolean, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeChoice, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeRGBA, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeRGB, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeDouble2D, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeInteger2D, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeDouble3D, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeInteger3D, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeString, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeCustom, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypeGroup, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypePage, 64)) {
    return kOfxStatErrUnsupported;
  } else if (0 == strncmp(paramHandle->type, kOfxParamTypePushButton, 64)) {
    return kOfxStatErrUnsupported;
  } else {
    return kOfxStatErrBadHandle;
  }
}

const OfxParameterSuiteV1 parameterSuiteV1 = {
  paramDefine, // OfxStatus (*paramDefine)(OfxParamSetHandle paramSet, const char *paramType, const char *name, OfxPropertySetHandle *propertySet);
  paramGetHandle, // OfxStatus (*paramGetHandle)(OfxParamSetHandle paramSet, const char *name, OfxParamHandle *param, OfxPropertySetHandle *propertySet);
  NULL, // OfxStatus (*paramSetGetPropertySet)(OfxParamSetHandle paramSet, OfxPropertySetHandle *propHandle);
  NULL, // OfxStatus (*paramGetPropertySet)(OfxParamHandle param, OfxPropertySetHandle *propHandle);
  paramGetValue, // OfxStatus (*paramGetValue)(OfxParamHandle paramHandle, ...);
  NULL, // OfxStatus (*paramGetValueAtTime)(OfxParamHandle paramHandle, OfxTime time, ...);
  NULL, // OfxStatus (*paramGetDerivative)(OfxParamHandle paramHandle, OfxTime time, ...);
  NULL, // OfxStatus (*paramGetIntegral)(OfxParamHandle paramHandle, OfxTime time1, OfxTime time2, ...);
  NULL, // OfxStatus (*paramSetValue)(OfxParamHandle paramHandle, ...);
  NULL, // OfxStatus (*paramSetValueAtTime)(OfxParamHandle paramHandle, OfxTime time, ...);
  NULL, // OfxStatus (*paramGetNumKeys)(OfxParamHandle paramHandle, unsigned int  *numberOfKeys);
  NULL, // OfxStatus (*paramGetKeyTime)(OfxParamHandle paramHandle, unsigned int nthKey, OfxTime *time);
  NULL, // OfxStatus (*paramGetKeyIndex)(OfxParamHandle paramHandle, OfxTime time, int direction, int *index);
  NULL, // OfxStatus (*paramDeleteKey)(OfxParamHandle paramHandle, OfxTime time);
  NULL, // OfxStatus (*paramDeleteAllKeys)(OfxParamHandle paramHandle);
  NULL, // OfxStatus (*paramCopy)(OfxParamHandle paramTo, OfxParamHandle  paramFrom, OfxTime dstOffset, const OfxRangeD *frameRange);
  NULL, // OfxStatus (*paramEditBegin)(OfxParamSetHandle paramSet, const char *name); 
  NULL, // OfxStatus (*paramEditEnd)(OfxParamSetHandle paramSet);
};
