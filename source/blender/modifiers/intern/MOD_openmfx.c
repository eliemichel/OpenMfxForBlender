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

#include "BLI_utildefines.h"

#include "DNA_mesh_types.h"
#include "BKE_context.h"
#include "BKE_screen.h"
#include "BKE_modifier.h"
#include "BKE_anim_data.h"
#include "BKE_lib_query.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "RNA_access.h"

#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"
#include "DNA_screen_types.h"

#include "MOD_modifiertypes.h"
#include "MOD_ui_common.h"
#include "MOD_util.h"

#include "BLO_read_write.h"

#include "mfxModifier.h"

#include <stdio.h>

// Modifier API

static Mesh *modifyMesh(ModifierData *md,
                           const ModifierEvalContext *ctx,
                           Mesh *mesh)
{
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;
  return mfx_Modifier_do(fxmd, mesh, ctx->object);
}

static void initData(struct ModifierData *md)
{
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;
  fxmd->active_effect_index = -1;
  fxmd->num_effects = 0;
  fxmd->effects = NULL;
  fxmd->num_parameters = 0;
  fxmd->parameters = NULL;
  fxmd->num_extra_inputs = 0;
  fxmd->extra_inputs = NULL;
  fxmd->message[0] = '\0';
}

static void copyData(const ModifierData *md, ModifierData *target, const int flag)
{
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;
  OpenMeshEffectModifierData *tfxmd = (OpenMeshEffectModifierData *)target;

  // A bit dirty to modify the copy source, but otherwise this would have to be in readfile.c,
  // which I don't want to depend on mfxModifier.h
  mfx_Modifier_reload_effect_info(fxmd);

  BKE_modifier_copydata_generic(md, target, flag);

  mfx_Modifier_copydata(fxmd, tfxmd);
}

static void requiredDataMask(Object *UNUSED(ob),
                             ModifierData *UNUSED(md),
                             CustomData_MeshMasks *r_cddata_masks)
{
  /* ask for extra attibutes.
     maybe there could be a mechanism in OpenMeshEffect to have a plugin explicitely
     ask for input attributes, so that we can avoid feeding all of them to addons
     that are not using it. */
  r_cddata_masks->lmask |= CD_MLOOPUV;
  r_cddata_masks->lmask |= CD_MLOOPCOL;
}

static void updateDepsgraph(ModifierData *md, const ModifierUpdateDepsgraphContext *ctx)
{
  printf("updateDepsgraph\n");
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;
  mfx_Modifier_before_updateDepsgraph(fxmd);

  bool do_add_own_transform = false; // TODO: if depend on main input's transform turn this true
  for (int i = 0; i < fxmd->num_extra_inputs; i++) {
    if (NULL == fxmd->extra_inputs[i].connected_object) {
      continue;
    }
    if (fxmd->extra_inputs[i].request_geometry) {
      DEG_add_object_relation(ctx->node,
                              fxmd->extra_inputs[i].connected_object,
                              DEG_OB_COMP_GEOMETRY,
                              "OpenMeshEffect Modifier Input Geometry");
      do_add_own_transform = true;
    }
    if (fxmd->extra_inputs[i].request_transform) {
      DEG_add_object_relation(ctx->node,
                              fxmd->extra_inputs[i].connected_object,
                              DEG_OB_COMP_TRANSFORM,
                              "OpenMeshEffect Modifier Input Transform");
      do_add_own_transform = true;
    }
  }

  if (do_add_own_transform) {
    DEG_add_modifier_to_transform_relation(ctx->node, "OpenMeshEffect Modifier Self");
  }
}

static bool dependsOnTime(struct ModifierData *md)
{
  // TODO: May depend on the OFX file
  return true;
}

static bool dependsOnNormals(struct ModifierData *md)
{
  // TODO: May depend on the OFX file (but harder to detect than time dependency -> add a user toggle)
  return true;
}

static void foreachIDLink(ModifierData *md, Object *ob, IDWalkFunc walk, void *userData)
{
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;
  for (int i = 0; i < fxmd->num_extra_inputs; i++) {
    walk(userData, ob, (ID **)&fxmd->extra_inputs[i].connected_object, IDWALK_CB_NOP);
  }
}

static void freeRuntimeData(void *runtime_data)
{
  if (runtime_data == NULL) {
    return;
  }
  mfx_Modifier_free_runtime_data(runtime_data);
}

static void freeData(struct ModifierData *md)
{
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;

  freeRuntimeData(md->runtime);
  md->runtime = NULL;

  if (fxmd->parameters) {
    MEM_freeN(fxmd->parameters);
    fxmd->parameters = NULL;
  }

  if (fxmd->extra_inputs) {
    MEM_freeN(fxmd->extra_inputs);
    fxmd->extra_inputs = NULL;
  }

  if (fxmd->effects) {
    MEM_freeN(fxmd->effects);
    fxmd->effects = NULL;
  }
}

static void panel_draw(const bContext *C, Panel *panel)
{
  uiLayout *row;
  uiLayout *layout = panel->layout;

  PointerRNA ob_ptr;
  PointerRNA *ptr = modifier_panel_get_property_pointers(panel, &ob_ptr);

  uiItemR(layout, ptr, "plugin_path", UI_ITEM_R_EXPAND, NULL, ICON_NONE);
  uiItemS(layout);

  uiItemR(layout, ptr, "effect_enum", 0, NULL, ICON_NONE);
  uiItemS(layout);

  char *label;
  CollectionPropertyIterator iter;
  for (RNA_collection_begin(ptr, "extra_inputs", &iter); iter.valid;
       RNA_property_collection_next(&iter)) {
    PointerRNA input_ptr = iter.ptr;
    label = RNA_string_get_alloc(&input_ptr, "label", NULL, 0);
    uiItemR(layout, &input_ptr, "connected_object", 0, label, ICON_NONE);
    MEM_freeN(label);
  }

  int type;
  for (RNA_collection_begin(ptr, "parameters", &iter); iter.valid;
       RNA_property_collection_next(&iter)) {
    PointerRNA param_ptr = iter.ptr;
    row = uiLayoutRow(layout, true);

    label = RNA_string_get_alloc(&param_ptr, "label", NULL, 0);
    uiItemL(row, label, ICON_NONE);
    MEM_freeN(label);
    
    type = RNA_int_get(&param_ptr, "type");
    switch (type) {
      case PARAM_TYPE_INTEGER:
        uiItemR(row, &param_ptr, "integer_value", 0, "", ICON_NONE);  // IFACE_("")?
        break;
      case PARAM_TYPE_INTEGER_2D:
        uiItemR(row, &param_ptr, "integer2d_value", 0, "", ICON_NONE);
        break;
      case PARAM_TYPE_INTEGER_3D:
        uiItemR(row, &param_ptr, "integer3d_value", 0, "", ICON_NONE);
        break;
      case PARAM_TYPE_DOUBLE:
        uiItemR(row, &param_ptr, "float_value", 0, "", ICON_NONE);
        break;
      case PARAM_TYPE_DOUBLE_2D:
        uiItemR(row, &param_ptr, "float2d_value", 0, "", ICON_NONE);
        break;
      case PARAM_TYPE_DOUBLE_3D:
        uiItemR(row, &param_ptr, "float3d_value", 0, "", ICON_NONE);
        break;
      case PARAM_TYPE_RGB:
        uiItemR(row, &param_ptr, "rgb_value", 0, "", ICON_NONE);
        break;
      case PARAM_TYPE_RGBA:
        uiItemR(row, &param_ptr, "rgba_value", 0, "", ICON_NONE);
        break;
      case PARAM_TYPE_BOOLEAN:
        uiItemR(row, &param_ptr, "boolean_value", 0, "", ICON_NONE);
        break;
      case PARAM_TYPE_STRING:
        uiItemR(row, &param_ptr, "string_value", 0, "", ICON_NONE);
        break;
    }
  }

  modifier_panel_end(layout, ptr);
}

static void panelRegister(ARegionType *region_type)
{
  PanelType *panel_type = modifier_panel_register(region_type, eModifierType_OpenMeshEffect, panel_draw);
}

static void blendWrite(BlendWriter *writer, const ModifierData *md)
{
  const OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;

  printf("At write, extra inputs are:\n");
  for (int i = 0; i < fxmd->num_extra_inputs; ++i) {
    printf(" - %p\n", fxmd->extra_inputs[i].connected_object);
  }

  BLO_write_struct_array(writer,
                         OpenMeshEffectParameter,
                         fxmd->num_parameters,
                         fxmd->parameters);
  BLO_write_struct_array(writer,
                         OpenMeshEffectInput,
                         fxmd->num_extra_inputs,
                         fxmd->extra_inputs);
}

static void blendRead(BlendDataReader *reader, ModifierData *md)
{
  OpenMeshEffectModifierData *fxmd = (OpenMeshEffectModifierData *)md;

  fxmd->parameters = BLO_read_data_address(reader, &fxmd->parameters);
  fxmd->extra_inputs = BLO_read_data_address(reader, &fxmd->extra_inputs);

  printf("At read, before remap, extra inputs are:\n");
  for (int i = 0; i < fxmd->num_extra_inputs; ++i) {
    printf(" - %p\n", fxmd->extra_inputs[i].connected_object);
  }

  // FIXME: For some reason the look up table used by BLO_read_get_new_data_address
  // is not ready yet at this stage.
  for (int i = 0; i < fxmd->num_extra_inputs; ++i) {
    fxmd->extra_inputs[i].connected_object = BLO_read_get_new_data_address(
        reader, fxmd->extra_inputs[i].connected_object);
  }

  printf("At read, after remap, extra inputs are:\n");
  for (int i = 0; i < fxmd->num_extra_inputs; ++i) {
    printf(" - %p\n", fxmd->extra_inputs[i].connected_object);
  }

  // Effect list will be reloaded from plugin
  fxmd->num_effects = 0;
  fxmd->effects = NULL;
}

ModifierTypeInfo modifierType_OpenMeshEffect = {
    /* name */ "Open Mesh Effect",
    /* structName */ "OpenMeshEffectModifierData",
    /* structSize */ sizeof(OpenMeshEffectModifierData),
    /* srna */ &RNA_OpenMeshEffectModifier,
    /* type */ eModifierTypeType_Constructive,
    /* flags */ eModifierTypeFlag_AcceptsMesh | eModifierTypeFlag_SupportsMapping |
        eModifierTypeFlag_SupportsEditmode | eModifierTypeFlag_EnableInEditmode,
    /* icon */ ICON_MOD_ARRAY,
    /* copyData */ copyData,
    /* deformVerts */ NULL,
    /* deformMatrices */ NULL,
    /* deformVertsEM */ NULL,
    /* deformMatricesEM */ NULL,
    /* modifyMesh */ modifyMesh,
    /* modifyHair */ NULL,
    /* modifyPointCloud */ NULL,
    /* modifyVolume */ NULL,
    /* initData */ initData,
    /* requiredDataMask */ requiredDataMask,
    /* freeData */ freeData,
    /* isDisabled */ NULL,
    /* updateDepsgraph */ updateDepsgraph,
    /* dependsOnTime */ dependsOnTime,
    /* dependsOnNormals */ dependsOnNormals,
    /* foreachIDLink */ foreachIDLink,
    /* foreachTexLink */ NULL,
    /* freeRuntimeData */ freeRuntimeData,
    /* uiPanel */ panelRegister,
    /* blendWrite */ blendWrite,
    /* blendRead */ blendRead,
};
