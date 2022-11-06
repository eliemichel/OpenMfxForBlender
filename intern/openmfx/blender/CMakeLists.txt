# ***** BEGIN GPL LICENSE BLOCK *****
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software Foundation,
# Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
#
# The Original Code is Copyright (C) 2019 - 2022, Elie Michel
# All rights reserved.
# ***** END GPL LICENSE BLOCK *****

# For some reason this is not automatically discovered...
set(OPENMFX_SDK_INCLUDES "")
get_target_property(INC OpenMfx::Sdk::Cpp::Host INCLUDE_DIRECTORIES)
list(APPEND OPENMFX_SDK_INCLUDES ${INC})
get_target_property(INC OpenMfx::Sdk::Cpp::Common INCLUDE_DIRECTORIES)
list(APPEND OPENMFX_SDK_INCLUDES ${INC})
get_target_property(INC OpenMfx::Core INTERFACE_INCLUDE_DIRECTORIES)
list(APPEND OPENMFX_SDK_INCLUDES ${INC})
list(REMOVE_DUPLICATES OPENMFX_SDK_INCLUDES)

set(INC
  .
  ../include
  ../../guardedalloc
  ../../../source/blender/makesdna
  ../../../source/blender/modifiers
  ../../../source/blender/blenlib
  ../../../source/blender/blenkernel
  ../../../source/blender/depsgraph
  ../../../source/blender/functions
  ${OPENMFX_SDK_INCLUDES}
)

set(INC_SYS
  intern
)

set(SRC
  MFX_modifier.h
  MFX_node_runtime.h
  MFX_util.h
  
  intern/BlenderMfxHost.h
  intern/BlenderMfxHost.cpp
  intern/MFX_convert.h
  intern/convert.cpp
  intern/modifier.cpp
  intern/modifier_runtime.h
  intern/modifier_runtime.cpp
  intern/node_runtime.cpp
  intern/util.cpp
)

set(LIB
  OpenMfx::Sdk::Cpp::Host
  OpenMfx::Sdk::Cpp::Common
  OpenMfx::Core
)

blender_add_lib(bf_intern_openmfx "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
