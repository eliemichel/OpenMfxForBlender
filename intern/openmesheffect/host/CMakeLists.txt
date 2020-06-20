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
  mfxHost.c
  mfxHost.h
  mfxPluginRegistry.h
  mfxPluginRegistryPool.h
  intern/attributes.h
  intern/attributes.c
  intern/properties.h
  intern/properties.c
  intern/parameters.h
  intern/parameters.c
  intern/inputs.h
  intern/inputs.c
  intern/mesh.h
  intern/mesh.c
  intern/mesheffect.h
  intern/mesheffect.c
  intern/messages.h
  intern/messages.c
  intern/mfxPluginRegistry.c
  intern/mfxPluginRegistryPool.c
)

set(LIB_PRIV
  openmesheffect_openfx
)

set(LIB
  openmesheffect_util
)

add_library(openmesheffect_host "${SRC}")
target_include_directories(openmesheffect_host PRIVATE "${INC_PRIV}" PUBLIC "${INC}")
target_link_libraries(openmesheffect_host PRIVATE "${LIB_PRIV}"  PUBLIC "${LIB}")

set_property(TARGET openmesheffect_host PROPERTY FOLDER "openmesheffect")
