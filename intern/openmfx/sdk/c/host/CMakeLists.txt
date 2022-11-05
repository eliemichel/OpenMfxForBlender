# ***** BEGIN APACHE 2 LICENSE BLOCK *****
#
# Copyright 2019-2022 Elie Michel
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#
# ***** END APACHE 2 LICENSE BLOCK *****

#----------------------------------------------------------

file(GLOB SRC src/binary_utils/*.c src/binary_utils/*.h)

add_library(OpenMfx_Sdk_C_BinaryUtils ${SRC})

target_link_libraries(
    OpenMfx_Sdk_C_BinaryUtils
    PUBLIC
        OpenMfx::Sdk::C::Common
)

target_include_directories(
    OpenMfx_Sdk_C_BinaryUtils
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_property(TARGET OpenMfx_Sdk_C_BinaryUtils PROPERTY FOLDER "OpenMfx/Sdk/C")
add_library(OpenMfx::Sdk::C::BinaryUtils ALIAS OpenMfx_Sdk_C_BinaryUtils)

#----------------------------------------------------------

file(GLOB SRC src/*.c src/*.h)

add_library(OpenMfx_Sdk_C_Host ${SRC})

target_link_libraries(
    OpenMfx_Sdk_C_Host
    PUBLIC
        OpenMfx::Sdk::C::Common
        OpenMfx::Sdk::C::BinaryUtils
)

target_include_directories(
    OpenMfx_Sdk_C_Host
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_property(TARGET OpenMfx_Sdk_C_Host PROPERTY FOLDER "OpenMfx/Sdk/C")
add_library(OpenMfx::Sdk::C::Host ALIAS OpenMfx_Sdk_C_Host)
