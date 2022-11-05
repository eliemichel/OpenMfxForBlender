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
# Minimal Plugin SDK

file(GLOB SRC src/Plugin/*.cpp src/Plugin/*.h)

add_library(OpenMfx_Sdk_Cpp_Plugin ${SRC})

target_link_libraries(
    OpenMfx_Sdk_Cpp_Plugin
    PUBLIC
        OpenMfx::Core
)

target_include_directories(
    OpenMfx_Sdk_Cpp_Plugin
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# Add -fPIC on unix systems
set_property(TARGET OpenMfx_Sdk_Cpp_Plugin PROPERTY POSITION_INDEPENDENT_CODE ON)

set_property(TARGET OpenMfx_Sdk_Cpp_Plugin PROPERTY FOLDER "OpenMfx/Sdk/Cpp")
add_library(OpenMfx::Sdk::Cpp::Plugin ALIAS OpenMfx_Sdk_Cpp_Plugin)
# Alternate target alias for backward compatibility
add_library(OpenMfx::CppPluginSupport ALIAS OpenMfx_Sdk_Cpp_Plugin)

#----------------------------------------------------------
# BMesh helper data structure

file(GLOB SRC src/BMesh/*.cpp src/BMesh/*.h)

add_library(OpenMfx_Sdk_Cpp_BMesh ${SRC})

target_link_libraries(
    OpenMfx_Sdk_Cpp_BMesh
    PUBLIC
        OpenMfx::Core
    PRIVATE
        glm
)

target_include_directories(
    OpenMfx_Sdk_Cpp_BMesh
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set_property(TARGET OpenMfx_Sdk_Cpp_BMesh PROPERTY FOLDER "OpenMfx/Sdk/Cpp")
add_library(OpenMfx::Sdk::Cpp::BMesh ALIAS OpenMfx_Sdk_Cpp_BMesh)
