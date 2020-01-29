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
  intern/mfxModifier.c
  intern/mfxCallbacks.h
  intern/mfxCallbacks.c
  intern/mfxRuntime.h
  intern/mfxRuntime.c
)

set(LIB
  openmesheffect_host
  openmesheffect_openfx
)

blender_add_lib(bf_intern_openmesheffect "${SRC}" "${INC}" "${INC_SYS}" "${LIB}")
