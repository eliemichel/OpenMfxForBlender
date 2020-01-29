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
 */

#include "MEM_guardedalloc.h"

#include "mfxRuntime.h"

#include "DNA_mesh_types.h" // Mesh
#include "DNA_meshdata_types.h" // MVert

#include "BKE_mesh.h" // BKE_mesh_new_nomain
#include "BKE_main.h" // BKE_main_blendfile_path_from_global

#include "BLI_math_vector.h"
#include "BLI_string.h"
#include "BLI_path_util.h"

// PUBLIC API

static OpenMeshEffectRuntime *mfx_Modifier_runtime_ensure(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_runtime_ensure on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = (OpenMeshEffectRuntime*)fxmd->modifier.runtime;
  printf("RUNTIME DATA @%p\n", runtime_data);

  // Init
  if (NULL == runtime_data) {
    fxmd->modifier.runtime = MEM_callocN(sizeof(OpenMeshEffectRuntime), "mfx runtime");
    runtime_data = (OpenMeshEffectRuntime*)fxmd->modifier.runtime;
    runtime_init(runtime_data);
    printf("NEW RUNTIME DATA @%p\n", runtime_data);
  }

  // Update
  printf("runtime_data->current_plugin_path = '%s'\n", runtime_data->current_plugin_path);
  printf("fxmd->plugin_path = '%s'\n", fxmd->plugin_path);
  bool plugin_changed = 0 != strcmp(runtime_data->current_plugin_path, fxmd->plugin_path);
  printf(plugin_changed ? " -> changed!\n" : " -> did not changed.\n");
  
  if (plugin_changed) {
    if (false == runtime_set_plugin_path(runtime_data, fxmd->plugin_path)) {
      fxmd->plugin_path[0] = '\0';
      fxmd->asset_index = -1;
    }
  }

  bool asset_changed = plugin_changed || runtime_data->current_asset_index != fxmd->asset_index;
  bool is_plugin_valid = 0 != strcmp(runtime_data->current_plugin_path, "");

  if (asset_changed && is_plugin_valid) {
    runtime_set_asset_index(runtime_data, fxmd->asset_index);
  }

  printf("==/ mfx_Modifier_runtime_ensure\n");
  return (OpenMeshEffectRuntime *)fxmd->modifier.runtime;
}

/**
 * Called when the "plugin path" field is changed.
 * It completes runtime_set_plugin_path by updating DNA data (fxmd).
 */
void mfx_Modifier_on_plugin_changed(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_on_plugin_changed on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = mfx_Modifier_runtime_ensure(fxmd);

  // Free previous info
  if (NULL != fxmd->asset_info) {
    MEM_freeN(fxmd->asset_info);
    fxmd->asset_info = NULL;
  }

  fxmd->num_assets = runtime_data->registry.num_plugins;
  fxmd->asset_info = MEM_calloc_arrayN(sizeof(OpenMeshEffectAssetInfo), fxmd->num_assets, "mfx asset info");

  for (int i = 0 ; i < fxmd->num_assets ; ++i) {
    // Get asset name
    const char *name = runtime_data->registry.plugins[i]->pluginIdentifier;
    printf("Loading %s to RNA\n", name);
    strncpy(fxmd->asset_info[i].name, name, sizeof(fxmd->asset_info[i].name));
  }

  printf("==/ mfx_Modifier_on_plugin_changed\n");
}

void mfx_Modifier_on_library_changed(OpenMeshEffectModifierData *fxmd) {
  printf("==. mfx_Modifier_on_library_changed on data %p\n", fxmd);
}

void mfx_Modifier_on_asset_changed(OpenMeshEffectModifierData *fxmd) {
  printf("==. mfx_Modifier_on_asset_changed on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = mfx_Modifier_runtime_ensure(fxmd);

  if (0 == strcmp(runtime_data->current_plugin_path, "")) {
    printf("current_plugin_path is null\n");
    printf("==/ mfx_Modifier_on_asset_changed\n");
    return;
  }

  if (-1 == runtime_data->current_asset_index) {
    printf("no asset selected\n");
    printf("==/ mfx_Modifier_on_asset_changed\n");
    return;
  }

  // Reset parameter DNA
  if (NULL != fxmd->parameter_info) {
    MEM_freeN(fxmd->parameter_info);
    fxmd->parameter_info = NULL;
  }

  OfxParamSetHandle parameters = &runtime_data->effect_desc->parameters;

  fxmd->num_parameters = parameters->num_parameters;
  fxmd->parameter_info = MEM_calloc_arrayN(sizeof(OpenMeshEffectParameterInfo), fxmd->num_parameters, "openmesheffect parameter info");

  for (int i = 0 ; i < fxmd->num_parameters ; ++i) {
    strncpy(fxmd->parameter_info[i].name, parameters->parameters[i]->name, sizeof(fxmd->parameter_info[i].name));
    // TODO: get parameter label from ofx param description
    strncpy(fxmd->parameter_info[i].label, parameters->parameters[i]->name, sizeof(fxmd->parameter_info[i].label));
    fxmd->parameter_info[i].type = parameters->parameters[i]->type;
  }

  printf("==/ mfx_Modifier_on_asset_changed on data %p\n", fxmd);
}

void mfx_Modifier_free_runtime_data(void * runtime_data)
{
  OpenMeshEffectRuntime * rd = (OpenMeshEffectRuntime *)runtime_data;
  printf("== mfx_Modifier_free_runtime_data\n");
  if (runtime_data == NULL) {
    printf("runtime data is null\n");
    printf("==/ mfx_Modifier_free_runtime_data\n");
    return;
  }
  runtime_free(rd);
  MEM_freeN(rd);
  printf("==/ mfx_Modifier_free_runtime_data\n");
}

Mesh * mfx_Modifier_do(OpenMeshEffectModifierData *fxmd, Mesh *mesh)
{
  printf("== mfx_Modifier_do on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = mfx_Modifier_runtime_ensure(fxmd);

  if (false == runtime_ensure_effect_instance(runtime_data)) {
    printf("failed to get effect instance\n");
    printf("==/ mfx_Modifier_do\n");
    return NULL;
  }

  OfxHost *ofxHost = runtime_data->ofx_host;
  OfxMeshEffectSuiteV1 *meshEffectSuite = (OfxMeshEffectSuiteV1*)ofxHost->fetchSuite(ofxHost->host, kOfxMeshEffectSuite, 1);
  OfxPropertySuiteV1 *propertySuite = (OfxPropertySuiteV1*)ofxHost->fetchSuite(ofxHost->host, kOfxPropertySuite, 1);

  OfxMeshInputHandle input, output;
  meshEffectSuite->inputGetHandle(runtime_data->effect_instance, kOfxMeshMainInput, &input, NULL);
  meshEffectSuite->inputGetHandle(runtime_data->effect_instance, kOfxMeshMainOutput, &output, NULL);
  
  // Get parameters
  runtime_get_parameters_from_rna(runtime_data, fxmd);

  // Set input mesh
  propertySuite->propSetPointer(&input->mesh, kOfxMeshPropInternalData, 0, (void*)mesh);

  OfxPlugin *plugin = runtime_data->registry.plugins[runtime_data->current_asset_index];
  ofxhost_cook(plugin, runtime_data->effect_instance);

  // Get output mesh and take ownership
  Mesh *result = NULL;
  propertySuite->propGetPointer(&output->mesh, kOfxMeshPropInternalData, 0, (void**)&result);
  propertySuite->propSetPointer(&output->mesh, kOfxMeshPropInternalData, 0, NULL);

  printf("==/ mfx_Modifier_do\n");
  return result;
}
