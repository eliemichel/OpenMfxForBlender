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

set(SRC
  src/Allocator.h
  src/Attributes.h
  src/Attributes.cpp
  src/AttributeProps.h
  src/AttributeProps.cpp
  src/AttributeEnums.h
  src/AttributeEnums.cpp
  src/EffectLibrary.h
  src/EffectLibrary.cpp
  src/EffectRegistry.h
  src/EffectRegistry.cpp
  src/EffectRegistryEntry.h
  src/EffectRegistryEntry.cpp
  src/Host.h
  src/Host.cpp
  src/Properties.h
  src/Properties.cpp
  src/PropertyEnums.h
  src/Parameters.h
  src/Parameters.cpp
  src/ParameterEnums.h
  src/ParameterEnums.cpp
  src/Inputs.h
  src/Inputs.cpp
  src/Mesh.h
  src/Mesh.cpp
  src/MeshProps.h
  src/MeshProps.cpp
  src/MeshEffect.h
  src/MeshEffect.cpp
  src/messages.h
  src/messages.cpp
  src/Collection.h

  src/parameterSuite.h
  src/parameterSuite.cpp
  src/propertySuite.h
  src/propertySuite.cpp
  src/meshEffectSuite.h
  src/meshEffectSuite.cpp
  src/messageSuite.h
  src/messageSuite.cpp

  src/util/binary_util.h
  src/util/binary_util.c
)

add_library(OpenMfx_Sdk_Cpp_Host ${SRC})

target_include_directories(
  OpenMfx_Sdk_Cpp_Host
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
  PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

set(NOT_MSVC $<NOT:$<CXX_COMPILER_ID:MSVC>>)

target_link_libraries(
  OpenMfx_Sdk_Cpp_Host
  PUBLIC
    OpenMfx::Sdk::Cpp::Common
    $<${NOT_MSVC}:dl> # for dlopen
)

set_property(TARGET OpenMfx_Sdk_Cpp_Host PROPERTY FOLDER "OpenMfx/Sdk/Cpp")
add_library(OpenMfx::Sdk::Cpp::Host ALIAS OpenMfx_Sdk_Cpp_Host)
# Alternate target alias for backward compatibility
add_library(OpenMfx::Host ALIAS OpenMfx_Sdk_Cpp_Host)
