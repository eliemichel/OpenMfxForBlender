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

namespace OpenMfx {

enum class ParameterType {
    Unknown = -1,
    Integer,
    Integer2d,
    Integer3d,
    Double,
    Double2d,
    Double3d,
    Rgb,
    Rgba,
    Boolean,
    Choice,
    String,
    Custom,
    PushButton,
    Group,
    Page,
};

/**
 * Convert a type string from MeshEffect API to its local enum counterpart
 */
ParameterType parameterTypeAsEnum(const char* mfxType);

/**
 * Return the number of components in the parameter type
 */
int parameterTypeDimension(ParameterType type);

} // namespace OpenMfx
