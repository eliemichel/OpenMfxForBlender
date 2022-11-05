/**
 * OpenMfx modifier for Blender
 * Copyright (C) 2019 - 2022 Elie Michel
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

/** \file
 * \ingroup openmfx
 * Provides a C interface to some OpenMfx object attributes
 */

#ifndef __MFX_UTILS_H__
#define __MFX_UTILS_H__

#include "DNA_node_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Get absolute path (ui file browser returns relative path for saved files)
 */
void MFX_normalize_plugin_path(char *dest_path, const char *src_path);

/**
 * @return the number of effects available in the OpenMfx effect library
 * currently loaded, or 0 if no library is loaded.
 */
int MFX_node_get_effect_count(bNode *node);

/**
 * @param effect_index is assumed to be between 0 included
 * and MFX_node_get_effect_count(node) excluded.
 */
const char *MFX_node_get_effect_identifier(bNode *node, int effect_index);

#ifdef __cplusplus
}
#endif

#endif // __MFX_UTILS_H__
