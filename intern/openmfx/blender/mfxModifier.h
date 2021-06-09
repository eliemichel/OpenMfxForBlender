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

/**
 * This is called from C code and handles the connection with the C++
 * based implementation of the modifier (mfxRuntime) that is stored in
 * the 'runtime' field of the ModifierData.
 */

#ifdef __cplusplus
extern "C" {
#endif

#include "../host/mfxParamType.h"

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

/**
 * Copy from runtime data (runtime_data->registry) to RNA (fxmd->effect_info)
 * the meta information about effects contained in the bundle.
 * Populate fxmd->num_effects and fxmd->effect_info unless runtime_data->is_plugin_valid if false
 */
void mfx_Modifier_reload_effect_info(OpenMfxModifierData *fxmd);

/**
 * Called when the "plugin path" field is changed.
 * It completes runtime_set_plugin_path by updating DNA data (fxmd).
 */
void mfx_Modifier_on_plugin_changed(OpenMfxModifierData *fxmd);

/**
 * Called when the user switched to another effect within the same plugin bundle.
 */
void mfx_Modifier_on_effect_changed(OpenMfxModifierData *fxmd);

void mfx_Modifier_free_runtime_data(void *runtime_data);

/**
 * Actually run the modifier, calling the cook action of the plugin
 */
Mesh *mfx_Modifier_do(OpenMfxModifierData *fxmd, Mesh *mesh, Object *object);

/**
 * Copy parameter_info, effect_info.
 * Must be called *after* blender's modifier_copyData_generic()
 */
void mfx_Modifier_copydata(OpenMfxModifierData *source,
                           OpenMfxModifierData *destination);

void mfx_Modifier_before_updateDepsgraph(OpenMfxModifierData *fxmd);

#ifdef __cplusplus
}
#endif

#endif // __MFX_MODIFIER_H__
