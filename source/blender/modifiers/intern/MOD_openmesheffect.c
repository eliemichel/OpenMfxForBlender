/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2005 Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup modifiers
 */

#include "MEM_guardedalloc.h"

#include <stdio.h>

#include "BLI_utildefines.h"

#include "BKE_mesh.h"

#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"

#include "MOD_modifiertypes.h"

#include "mfxModifier.h"

// Modifier API

static Mesh *applyModifier(struct ModifierData *md,
                           const struct ModifierEvalContext *ctx,
                           struct Mesh *mesh)
{
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;
  return mfx_Modifier_do(fxmd, mesh);
}

static void initData(struct ModifierData *md)
{
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;
  fxmd->asset_index = -1;
  fxmd->num_assets = 0;
  fxmd->asset_info = NULL;
  fxmd->num_parameters = 0;
  fxmd->parameter_info = NULL;
}

static void copyData(const ModifierData *md, ModifierData *target, const int flag)
{
  OpenMeshEffectModifierData *tfxmd = (OpenMeshEffectModifierData *)target;

  modifier_copyData_generic(md, target, flag);

  //tfxmd->?
}

static bool dependsOnTime(struct ModifierData *md)
{
  // TODO: May depend on the HDA file
  return true;
}

static bool dependsOnNormals(struct ModifierData *md)
{
  // TODO: May depend on the HDA file (but harder to detect than time dependency -> add a user toggle)
  return true;
}

static void freeRuntimeData(void *runtime_data)
{
  if (runtime_data == NULL) {
    return;
  }
  printf("freeRuntimeData on pointer %p.\n", runtime_data);
  mfx_Modifier_free_runtime_data(runtime_data);
}

static void freeData(struct ModifierData *md)
{
  freeRuntimeData(md->runtime);
}

ModifierTypeInfo modifierType_OpenMeshEffect = {
    /* name */ "Open Mesh Effect",
    /* structName */ "OpenMeshEffectModifierData",
    /* structSize */ sizeof(OpenMeshEffectModifierData),
    /* type */ eModifierTypeType_Constructive,
    /* flags */ eModifierTypeFlag_AcceptsMesh,

    /* copyData */ copyData,

    /* deformVerts */ NULL,
    /* deformMatrices */ NULL,
    /* deformVertsEM */ NULL,
    /* deformMatricesEM */ NULL,
    /* applyModifier */ applyModifier,

    /* initData */ initData,
    /* requiredDataMask */ NULL,
    /* freeData */ freeData,
    /* isDisabled */ NULL,
    /* updateDepsgraph */ NULL,
    /* dependsOnTime */ dependsOnTime,
    /* dependsOnNormals */ dependsOnNormals,
    /* foreachObjectLink */ NULL,
    /* foreachIDLink */ NULL,
    /* foreachTexLink */ NULL,
    /* freeRuntimeData */ freeRuntimeData,
};
