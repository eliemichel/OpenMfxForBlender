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

set(Target OpenMfx_Example_Cpp_Plugin)
file(GLOB SRC *.hpp *.cpp)

add_library(${Target} SHARED ${SRC})
set_target_properties(${Target} PROPERTIES SUFFIX ".ofx")

target_link_libraries(
	${Target}
	PRIVATE
		OpenMfx::Sdk::Cpp::Plugin
)

target_treat_warnings_as_errors(${Target})
set_property(TARGET ${Target} PROPERTY FOLDER "OpenMfx/Examples")

