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

#ifndef __MFX_PROPERTY_SUITE_H__
#define __MFX_PROPERTY_SUITE_H__


// // Property Suite Entry Points

#include "ofxProperty.h"

#ifdef __cplusplus
extern "C" {
#endif

extern const OfxPropertySuiteV1 gPropertySuiteV1;

// See ofxProperty.h for docstrings

OfxStatus propSetPointer(OfxPropertySetHandle properties,
                         const char *property,
                         int index,
                         void *value);
OfxStatus propSetString(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        const char *value);
OfxStatus propSetDouble(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        double value);
OfxStatus propSetInt(OfxPropertySetHandle properties, const char *property, int index, int value);
OfxStatus propSetPointerN(OfxPropertySetHandle properties,
                          const char *property,
                          int count,
                          void *const *value);
OfxStatus propSetStringN(OfxPropertySetHandle properties,
                         const char *property,
                         int count,
                         const char *const *value);
OfxStatus propSetDoubleN(OfxPropertySetHandle properties,
                         const char *property,
                         int count,
                         const double *value);
OfxStatus propSetIntN(OfxPropertySetHandle properties,
                      const char *property,
                      int count,
                      const int *value);
OfxStatus propGetPointer(OfxPropertySetHandle properties,
                         const char *property,
                         int index,
                         void **value);
OfxStatus propGetString(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        char **value);
OfxStatus propGetDouble(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        double *value);
OfxStatus propGetInt(OfxPropertySetHandle properties, const char *property, int index, int *value);
OfxStatus propGetPointerN(OfxPropertySetHandle properties,
                          const char *property,
                          int count,
                          void **value);
OfxStatus propGetStringN(OfxPropertySetHandle properties,
                         const char *property,
                         int count,
                         char **value);
OfxStatus propGetDoubleN(OfxPropertySetHandle properties,
                         const char *property,
                         int count,
                         double *value);
OfxStatus propGetIntN(OfxPropertySetHandle properties,
                      const char *property,
                      int count,
                      int *value);
OfxStatus propReset(OfxPropertySetHandle properties, const char *property);
OfxStatus propGetDimension(OfxPropertySetHandle properties, const char *property, int *count);

#ifdef __cplusplus
}
#endif

#endif // __MFX_PROPERTY_SUITE_H__
