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

set(INC
  .
)

set(INC_PRIV
)

set(SRC
  mfxHost.cpp
  mfxHost.h
  mfxPluginRegistry.h
  mfxPluginRegistryPool.h
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
  intern/mfxPluginRegistryPool.cpp
  intern/PluginRegistryPool.h
  intern/PluginRegistryPool.cpp

  intern/parameterSuite.h
  intern/parameterSuite.cpp
  intern/propertySuite.h
  intern/propertySuite.cpp
  intern/meshEffectSuite.h
  intern/meshEffectSuite.cpp
  intern/messageSuite.h
  intern/messageSuite.cpp
)

set(LIB_PRIV
  OpenMfx::Core
)

set(LIB
  OpenMfx::Utils
)

add_library(Host "${SRC}")
target_include_directories(Host PRIVATE "${INC_PRIV}" PUBLIC "${INC}")
target_link_libraries(Host PRIVATE "${LIB_PRIV}"  PUBLIC "${LIB}")

set_property(TARGET Host PROPERTY FOLDER "OpenMfx")
add_library(OpenMfx::Host ALIAS Host)
