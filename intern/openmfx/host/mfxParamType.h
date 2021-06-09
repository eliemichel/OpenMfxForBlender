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

#ifndef __MFX_PARAM_TYPE_H__
#define __MFX_PARAM_TYPE_H__

/**
 * Enum version of kOfxParamType* constants, used rather than strings for
 * efficiency reasons.
 * TODO: Expose to PYTHON
 */
typedef enum ParamType {
    PARAM_TYPE_UNKNOWN = -1,
    PARAM_TYPE_INTEGER,
    PARAM_TYPE_INTEGER_2D,
    PARAM_TYPE_INTEGER_3D,
    PARAM_TYPE_DOUBLE,
    PARAM_TYPE_DOUBLE_2D,
    PARAM_TYPE_DOUBLE_3D,
    PARAM_TYPE_RGB,
    PARAM_TYPE_RGBA,
    PARAM_TYPE_BOOLEAN,
    PARAM_TYPE_CHOICE,
    PARAM_TYPE_STRING,
    PARAM_TYPE_CUSTOM,
    PARAM_TYPE_PUSH_BUTTON,
    PARAM_TYPE_GROUP,
    PARAM_TYPE_PAGE,
} ParamType;

#endif __MFX_PARAM_TYPE_H__
