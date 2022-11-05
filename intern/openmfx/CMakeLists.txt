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
# The Original Code is Copyright (C) 2019, Elie Michel
# All rights reserved.
# ***** END GPL LICENSE BLOCK *****

option(OPENMFX_BUILD_EXAMPLES "Build example projects" OFF)

# Main target (C API)
add_library(OpenMfx INTERFACE)
target_include_directories(OpenMfx INTERFACE include/)
add_library(OpenMfx::Core ALIAS OpenMfx)

add_subdirectory(sdk)

add_subdirectory(blender)

if(WITH_GTESTS)
  add_subdirectory(test)
endif()

