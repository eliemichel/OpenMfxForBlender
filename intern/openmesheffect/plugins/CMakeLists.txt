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

set(SRC
  mfx_sample_plugin.c
)

set(LIB
  openmesheffect_openfx
  openmesheffect_util
)

add_library(openmesheffect_sample_plugin SHARED ${SRC})
target_include_directories(openmesheffect_sample_plugin PRIVATE "${INC}")
target_link_libraries(openmesheffect_sample_plugin PRIVATE "${LIB}")

set_property(TARGET openmesheffect_sample_plugin PROPERTY FOLDER "openmesheffect")
set_target_properties(openmesheffect_sample_plugin PROPERTIES SUFFIX ".ofx")
