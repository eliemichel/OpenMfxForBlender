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
 * Utility functions using Blender functions
 */

#ifndef __MFX_UTILS_H__
#define __MFX_UTILS_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "DNA_node_types.h"

#include "mfxPluginRegistry.h"

/**
 * Get absolute path (ui file browser returns relative path for saved files)
 */
void MFX_normalize_plugin_path(char *dest_path, const char *src_path);

const PluginRegistry *MFX_get_plugin_registry(bNode *node);

int MFX_component_size(const char *componentType);

#ifdef __cplusplus
}
#endif

#endif // __MFX_UTILS_H__
