/**
 * OpenMfx node for Blender
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
 */

#include "MFX_util.h"

#include "BKE_main.h" // BKE_main_blendfile_path_from_global

#include "DNA_node_types.h"

#include "BLI_path_util.h"
#include "BLI_string.h"

#include "MFX_node_runtime.h"

void MFX_normalize_plugin_path(char* dest_path, const char* src_path)
{
    BLI_strncpy(dest_path, src_path, FILE_MAX);
    const char* base_path =
        BKE_main_blendfile_path_from_global();  // TODO: How to get a bMain object here to avoid
                                                // "from_global()"?
    if (NULL != base_path) {
        BLI_path_abs(dest_path, base_path);
    }
}

const PluginRegistry *MFX_get_plugin_registry(bNode *node)
{
  NodeGeometryOpenMfxRuntimeHandle *runtime = ((NodeGeometryOpenMfx *)(node->storage))->runtime;
  return &runtime->registry();
}
