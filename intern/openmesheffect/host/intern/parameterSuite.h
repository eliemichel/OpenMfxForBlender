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

 /** \file
  * \ingroup openmesheffect
  *
  */

#ifndef __MFX_PARAMETER_SUITE_H__
#define __MFX_PARAMETER_SUITE_H__

// // Parameter Suite Entry Points

#include "ofxParam.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const OfxParameterSuiteV1 gParameterSuiteV1;

  // See ofxParam.h for docstrings

OfxStatus paramDefine(OfxParamSetHandle paramSet,
    const char* paramType,
    const char* name,
    OfxPropertySetHandle* propertySet);
OfxStatus paramGetHandle(OfxParamSetHandle paramSet,
    const char* name,
    OfxParamHandle* param,
    OfxPropertySetHandle* propertySet);
OfxStatus paramSetGetPropertySet(OfxParamSetHandle paramSet,
    OfxPropertySetHandle* propHandle);
OfxStatus paramGetPropertySet(OfxParamHandle param,
    OfxPropertySetHandle* propHandle);
OfxStatus paramGetValue(OfxParamHandle paramHandle, ...);
OfxStatus paramGetValueAtTime(OfxParamHandle paramHandle, OfxTime time, ...);
OfxStatus paramGetDerivative(OfxParamHandle paramHandle, OfxTime time, ...);
OfxStatus paramGetIntegral(OfxParamHandle paramHandle, OfxTime time1, OfxTime time2, ...);
OfxStatus paramSetValue(OfxParamHandle paramHandle, ...);
OfxStatus paramSetValueAtTime(OfxParamHandle paramHandle, OfxTime time, ...);
OfxStatus paramGetNumKeys(OfxParamHandle paramHandle, unsigned int* numberOfKeys);
OfxStatus paramGetKeyTime(OfxParamHandle paramHandle,
    unsigned int nthKey,
    OfxTime* time);
OfxStatus paramGetKeyIndex(OfxParamHandle paramHandle,
    OfxTime time,
    int direction,
    int* index);
OfxStatus paramDeleteKey(OfxParamHandle paramHandle, OfxTime time);
OfxStatus paramDeleteAllKeys(OfxParamHandle paramHandle);
OfxStatus paramCopy(OfxParamHandle paramTo,
    OfxParamHandle paramFrom,
    OfxTime dstOffset,
    const OfxRangeD* frameRange);
OfxStatus paramEditBegin(OfxParamSetHandle paramSet, const char* name);
OfxStatus paramEditEnd(OfxParamSetHandle paramSet);

#ifdef __cplusplus
}
#endif

#endif // __MFX_PARAMETER_SUITE_H__

