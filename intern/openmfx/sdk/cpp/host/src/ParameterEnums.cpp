/*
 * Copyright 2019-2022 Elie Michel
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

#include "ParameterEnums.h"

#include <ofxParam.h>

#include <cstring>
#include <cstdio>

namespace OpenMfx {

ParameterType parameterTypeAsEnum(const char* mfxType)
{
    if (0 == strcmp(mfxType, kOfxParamTypeInteger)) {
        return ParameterType::Integer;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeInteger2D)) {
        return ParameterType::Integer2d;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeInteger3D)) {
        return ParameterType::Integer3d;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeDouble)) {
        return ParameterType::Double;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeDouble2D)) {
        return ParameterType::Double2d;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeDouble3D)) {
        return ParameterType::Double3d;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeRGB)) {
        return ParameterType::Rgb;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeRGBA)) {
        return ParameterType::Rgba;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeBoolean)) {
        return ParameterType::Boolean;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeChoice)) {
        return ParameterType::Choice;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeString)) {
        return ParameterType::String;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeCustom)) {
        return ParameterType::Custom;
    }
    if (0 == strcmp(mfxType, kOfxParamTypePushButton)) {
        return ParameterType::PushButton;
    }
    if (0 == strcmp(mfxType, kOfxParamTypeGroup)) {
        return ParameterType::Group;
    }
    if (0 == strcmp(mfxType, kOfxParamTypePage)) {
        return ParameterType::Page;
    }
    return ParameterType::Unknown;
}

int parameterTypeDimension(ParameterType type)
{
    switch (type) {
    case ParameterType::Integer:
    case ParameterType::Double:
    case ParameterType::Boolean:
    case ParameterType::String:
        return 1;
    case ParameterType::Integer2d:
    case ParameterType::Double2d:
        return 2;
    case ParameterType::Integer3d:
    case ParameterType::Double3d:
    case ParameterType::Rgb:
        return 3;
    case ParameterType::Rgba:
        return 4;
    default:
        return 1;
    }
}

} // namespace OpenMfx
