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
  include
)

set(INC_PRIV
	include/util
)

set(SRC
  intern/ofx_util.c
  intern/memory_util.c
  intern/binary_util.c
  intern/plugin_support.c

  include/util/ofx_util.h
  include/util/memory_util.h
  include/util/binary_util.h
  include/util/path_util.h
  include/util/plugin_support.h
)

set(LIB
  OpenMfx::Core
)

add_library(Utils "${SRC}")
target_include_directories(Utils PRIVATE "${INC_PRIV}" PUBLIC "${INC}")
target_link_libraries(Utils PRIVATE "${LIB}")

set_property(TARGET Utils PROPERTY FOLDER "OpenMfx")
add_library(OpenMfx::Utils ALIAS Utils)
