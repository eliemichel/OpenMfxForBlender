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
# The Original Code is Copyright (C) 2019 - 2020, Elie Michel
# All rights reserved.
# ***** END GPL LICENSE BLOCK *****

set(INC
  .
  ../host
  ../openfx
  ../util/include
  ../../guardedalloc
  ../../../source/blender/makesdna
  ../../../source/blender/modifiers
  ../../../source/blender/blenlib
  ../../../source/blender/blenkernel
)

set(INC_SYS
  intern
)

set(SRC
  mfxModifier.h
  intern/mfxModifier.cpp
  intern/mfxCallbacks.h
  intern/mfxCallbacks.cpp
  intern/mfxRuntime.h
  intern/mfxRuntime.cpp
  intern/mfxConvert.h
  intern/mfxConvert.cpp
)

set(LIB
  OpenMfx::Host
  OpenMfx::Core
)

blender_add_lib(bf_intern_openmfx "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
