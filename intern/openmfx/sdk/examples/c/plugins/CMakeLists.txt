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

macro(add_mfx_plugin TargetShortname)
	set(options)
	set(oneValueArgs)
	set(multiValueArgs SRC LIBS)
	cmake_parse_arguments(ARG "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})
	set(Target "OpenMfx_Example_C_Plugin_${TargetShortname}")

	add_library(${Target} SHARED ${ARG_SRC})
	set_target_properties(${Target} PROPERTIES SUFFIX ".ofx" PREFIX "")

	target_link_libraries(
		${Target}
		PRIVATE
			OpenMfx::Sdk::C::Plugin
			${ARG_LIBS}
	)

	set_property(TARGET ${Target} PROPERTY FOLDER "OpenMfx/Examples")
	target_treat_warnings_as_errors(${Target})
endmacro()

add_mfx_plugin(
	sample
	SRC
		mfx_sample_plugin.c
)

add_mfx_plugin(
	test_parameters_plugin
	SRC
		mfx_test_parameters_plugin.c
)

add_mfx_plugin(
	identity
	SRC
		mfx_identity_plugin.c
)

add_mfx_plugin(
	mirror
	SRC
		mfx_mirror_plugin.c
)

add_mfx_plugin(
	color_to_uv
	SRC
		mfx_color_to_uv.c
)

add_mfx_plugin(
	uv_transform
	SRC
		mfx_uv_transform.c
)

