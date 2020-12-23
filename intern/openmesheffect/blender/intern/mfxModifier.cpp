/**
 * Open Mesh Effect modifier for Blender
 * Copyright (C) 2019 - 2020 Elie Michel
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
 */


#include "MEM_guardedalloc.h"

#include "mfxCallbacks.h"
#include "mfxRuntime.h"
#include "mfxConvert.h"

#include "DNA_mesh_types.h"      // Mesh
#include "DNA_meshdata_types.h"  // MVert

#include "BKE_main.h"  // BKE_main_blendfile_path_from_global
#include "BKE_mesh.h"  // BKE_mesh_new_nomain
#include "BKE_modifier.h"  // BKE_modifier_setError

#include "BLI_math_vector.h"
#include "BLI_path_util.h"
#include "BLI_string.h"

/**
 * Ensure that fxmd->modifier.runtime points to a valid OpenMeshEffectRuntime and return
 * this poitner, correctly casted.
 * (idempotent)
 */
static OpenMeshEffectRuntime *ensure_runtime(OpenMeshEffectModifierData *fxmd) {
  
  // Init
  OpenMeshEffectRuntime *runtime = (OpenMeshEffectRuntime *)fxmd->modifier.runtime;
  if (NULL == runtime) {
    runtime = new OpenMeshEffectRuntime();
    fxmd->modifier.runtime = runtime;
  }

  // Update
  runtime->set_plugin_path(fxmd->plugin_path);
  runtime->set_effect_index(fxmd->active_effect_index);

  if (false == runtime->is_plugin_valid()) {
    BKE_modifier_set_error(NULL, &fxmd->modifier, "Could not load ofx plugins!");
  }

  return (OpenMeshEffectRuntime *)fxmd->modifier.runtime;
}

void mfx_Modifier_reload_effect_info(OpenMeshEffectModifierData *fxmd) {

  OpenMeshEffectRuntime *runtime = ensure_runtime(fxmd);
  runtime->reload_effect_info(fxmd);

}

void mfx_Modifier_on_plugin_changed(OpenMeshEffectModifierData *fxmd) {

  mfx_Modifier_reload_effect_info(fxmd);
  mfx_Modifier_on_effect_changed(fxmd);

}

void mfx_Modifier_on_effect_changed(OpenMeshEffectModifierData *fxmd) {
  
  OpenMeshEffectRuntime *runtime = ensure_runtime(fxmd);
  runtime->reload_parameters(fxmd);
  runtime->reload_extra_inputs(fxmd);


}

void mfx_Modifier_free_runtime_data(void * runtime_data)
{
  
  OpenMeshEffectRuntime * runtime = (OpenMeshEffectRuntime *)runtime_data;
  if (NULL != runtime) {
    delete runtime;
  }
  
}

Mesh * mfx_Modifier_do(OpenMeshEffectModifierData *fxmd, Mesh *mesh, Object *object)
{
  
  OpenMeshEffectRuntime *runtime = ensure_runtime(fxmd);
  Mesh *output_mesh = runtime->cook(fxmd, mesh, object);

  return output_mesh;
}

void mfx_Modifier_copydata(OpenMeshEffectModifierData *source, OpenMeshEffectModifierData *destination)
{
  if (source->parameters) {
    destination->parameters = (OpenMeshEffectParameter *)MEM_dupallocN(source->parameters);
  }

  if (source->extra_inputs) {
    destination->extra_inputs = (OpenMeshEffectInput *)MEM_dupallocN(source->extra_inputs);
  }

  if (source->effects) {
    destination->effects = (OpenMeshEffectEffect *)MEM_dupallocN(source->effects);
  }

  OpenMeshEffectRuntime *runtime = (OpenMeshEffectRuntime *)source->modifier.runtime;
  if (NULL != runtime) {
    runtime->set_message_in_rna(destination);
  }
}
