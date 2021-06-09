# ***** BEGIN APACHE 2 LICENSE BLOCK *****
#
# Copyright 2019-2021 Elie Michel
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

macro(MFX_PLUGIN TARGET SRC LIB)
	add_library(${TARGET} SHARED ${SRC})
	target_include_directories(${TARGET} PRIVATE "${INC}")
	target_link_libraries(${TARGET} PRIVATE "${LIB}")
	set_property(TARGET ${TARGET} PROPERTY FOLDER "openmesheffect")
	set_target_properties(${TARGET} PROPERTIES SUFFIX ".ofx" PREFIX "")
endmacro()

set(LIB
  OpenMfx::Core
  OpenMfx::Utils
)

MFX_PLUGIN(openmfx_sample_plugin mfx_sample_plugin.c "${LIB}")
MFX_PLUGIN(openmfx_test_parameters_plugin mfx_test_parameters_plugin.c "${LIB}")
MFX_PLUGIN(openmfx_identity_plugin mfx_identity_plugin.c "${LIB}")
MFX_PLUGIN(openmfx_mirror_plugin mfx_mirror_plugin.c "${LIB}")
MFX_PLUGIN(openmfx_color_to_uv mfx_color_to_uv.c "${LIB}")
MFX_PLUGIN(openmfx_uv_transform mfx_uv_transform.c "${LIB}")
