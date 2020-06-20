/**
 * Open Mesh Effect modifier for Blender
 * Copyright (C) 2019 Elie Michel
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
 * \ingroup openmesheffect
 *
 * This is an implementation of an OpenFX host specialized toward the Mesh
 * Effect API (rather than the Image Effect API like most OpenFX host
 * implementations are.)
 */

#ifndef __MFX_MODIFIER_H__
#define __MFX_MODIFIER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "DNA_meshdata_types.h"
#include "DNA_modifier_types.h"
#include "DNA_object_types.h"
#include "DNA_mesh_types.h"

#ifndef RNA_RUNTIME
#  include "MOD_modifiertypes.h"
#endif

// Callback types required by mfxHost
typedef Mesh* (*MeshNewFunc)(int, int, int, int, int); // BKE_mesh_new_nomain
typedef void (*MeshPostFunc)(Mesh*); // post processing applied to new mesh

void mfx_Modifier_reload_effect_info(OpenMeshEffectModifierData *fxmd);

/**
 * Called when the "plugin path" field is changed.
 * It completes runtime_set_plugin_path by updating DNA data (fxmd).
 */
void mfx_Modifier_on_plugin_changed(OpenMeshEffectModifierData *fxmd);

/**
 * Called when the user switched to another effect within the same plugin bundle.
 */
void mfx_Modifier_on_effect_changed(OpenMeshEffectModifierData *fxmd);

void mfx_Modifier_free_runtime_data(void *runtime_data);

/**
 * Actually run the modifier, calling the cook action of the plugin
 */
Mesh *mfx_Modifier_do(OpenMeshEffectModifierData *fxmd, Mesh *mesh);

/**
 * Copy parameter_info, effect_info.
 * Must be called *after* blender's modifier_copyData_generic()
 */
void mfx_Modifier_copyData(OpenMeshEffectModifierData *source,
                           OpenMeshEffectModifierData *destination);

#ifdef __cplusplus
}
#endif

#endif // __MFX_MODIFIER_H__
