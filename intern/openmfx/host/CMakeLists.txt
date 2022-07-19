# ***** BEGIN APACHE 2 LICENSE BLOCK *****
#
# Copyright 2019 Elie Michel
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
  mfxHost.cpp
  mfxHost.h
  mfxPluginRegistry.h
  mfxPluginRegistryManager.h
  PluginRegistryManager.h

  intern/Allocator.h
  intern/attributes.h
  intern/attributes.cpp
  intern/properties.h
  intern/properties.cpp
  intern/parameters.h
  intern/parameters.cpp
  intern/inputs.h
  intern/inputs.cpp
  intern/mesh.h
  intern/mesh.cpp
  intern/mesheffect.h
  intern/mesheffect.cpp
  intern/messages.h
  intern/messages.cpp
  intern/mfxPluginRegistry.cpp
  intern/mfxPluginRegistryManager.cpp
  intern/PluginRegistryManager.cpp
  intern/MfxHost.h
  intern/MfxHost.cpp
  intern/Collection.h

  intern/parameterSuite.h
  intern/parameterSuite.cpp
  intern/propertySuite.h
  intern/propertySuite.cpp
  intern/meshEffectSuite.h
  intern/meshEffectSuite.cpp
  intern/messageSuite.h
  intern/messageSuite.cpp

  util/binary_util.h
  util/mfx_util.h
  intern/util/binary_util.c
  intern/util/ofx_util.h
  intern/util/ofx_util.c

  sdk/MfxAttributeProps.h
  sdk/MfxAttributeProps.cpp
)

set(INC_PRIV
)

set(INC
  ${CMAKE_CURRENT_SOURCE_DIR}
)

set(LIB_PRIV
  OpenMfx::Core
)

set(LIB
#  OpenMfx::Utils
)

add_library(OpenMfx_Host "${SRC}")
target_include_directories(OpenMfx_Host PRIVATE "${INC_PRIV}" PUBLIC "${INC}")
target_link_libraries(OpenMfx_Host PRIVATE "${LIB_PRIV}"  PUBLIC "${LIB}")

set_property(TARGET OpenMfx_Host PROPERTY FOLDER "OpenMfx")
add_library(OpenMfx::Host ALIAS OpenMfx_Host)
