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

#pragma once

#include "Properties.h"
#include "ParameterEnums.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <cstddef>

union OfxParamValueStruct {
    void* as_pointer;
    const char* as_const_char;
    char* as_char;
    int as_int;
    double as_double;
    bool as_bool;
};

// // OfxParamStruct

struct OfxParamStruct {
public:
    OfxParamStruct();
    ~OfxParamStruct();
    MOVE_ONLY(OfxParamStruct)

        void set_type(OpenMfx::ParameterType type);
    void realloc_string(int size);

    void deep_copy_from(const OfxParamStruct& other);

public:
    char* name;
    OfxParamValueStruct value[4];
    OpenMfx::ParameterType type;
    OpenMfx::PropertySet properties;
};

// // OfxParamSetStruct

struct OfxParamSetStruct {
public:
    OfxParamSetStruct();
    ~OfxParamSetStruct();
    MOVE_ONLY(OfxParamSetStruct)

        int find(const char* param) const;
    void append(int count);
    int ensure(const char* parameter);

    void deep_copy_from(const OfxParamSetStruct& other);

    int count() const { return num_parameters; }
    OfxParamStruct& operator[](int i) { return *parameters[i]; }
    const OfxParamStruct& operator[](int i) const { return *parameters[i]; }

public:
    OpenMfx::PropertySet* effect_properties; // weak pointer

private:
    int num_parameters;
    OfxParamStruct** parameters;
};

namespace OpenMfx {
typedef OfxParamStruct Param;
typedef OfxParamSetStruct ParamSet;
} // namespace OpenMfx
