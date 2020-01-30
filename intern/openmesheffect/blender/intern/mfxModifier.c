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
  runtime_set_plugin_path(runtime_data, fxmd->plugin_path);
  runtime_set_effect_index(runtime_data, fxmd->effect_index);

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
  if (NULL != fxmd->effect_info) {
    MEM_freeN(fxmd->effect_info);
    fxmd->effect_info = NULL;
    fxmd->num_effects = 0;
  }

  if (false == runtime_data->is_plugin_valid) {
    printf("==/ mfx_Modifier_on_plugin_changed\n");
    mfx_Modifier_on_effect_changed(fxmd);
    return;
  }

  fxmd->num_effects = runtime_data->registry.num_plugins;
  fxmd->effect_info = MEM_calloc_arrayN(sizeof(OpenMeshEffectEffectInfo), fxmd->num_effects, "mfx effect info");

  for (int i = 0 ; i < fxmd->num_effects; ++i) {
    // Get asset name
    const char *name = runtime_data->registry.plugins[i]->pluginIdentifier;
    printf("Loading %s to RNA\n", name);
    strncpy(fxmd->effect_info[i].name, name, sizeof(fxmd->effect_info[i].name));
  }

  mfx_Modifier_on_effect_changed(fxmd);

  printf("==/ mfx_Modifier_on_plugin_changed\n");
}

void mfx_Modifier_on_effect_changed(OpenMeshEffectModifierData *fxmd) {
  printf("==. mfx_Modifier_on_asset_changed on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = mfx_Modifier_runtime_ensure(fxmd);

  // Reset parameter DNA
  if (NULL != fxmd->parameter_info) {
    MEM_freeN(fxmd->parameter_info);
    fxmd->parameter_info = NULL;
    fxmd->num_parameters = 0;
  }

  if (NULL == runtime_data->effect_desc) {
    printf("==/ mfx_Modifier_on_asset_changed\n");
    return;
  }

  OfxParamSetHandle parameters = &runtime_data->effect_desc->parameters;

  fxmd->num_parameters = parameters->num_parameters;
  fxmd->parameter_info = MEM_calloc_arrayN(sizeof(OpenMeshEffectParameterInfo), fxmd->num_parameters, "openmesheffect parameter info");

  for (int i = 0 ; i < fxmd->num_parameters ; ++i) {
    OfxPropertySetStruct props = parameters->parameters[i]->properties;
    int prop_idx = find_property(&props, kOfxParamPropScriptName);
    const char * system_name =
      prop_idx != -1
      ? props.properties[prop_idx]->value->as_const_char
      : parameters->parameters[i]->name;
    strncpy(fxmd->parameter_info[i].name, system_name, sizeof(fxmd->parameter_info[i].name));
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

  OfxPlugin *plugin = runtime_data->registry.plugins[runtime_data->effect_index];
  ofxhost_cook(plugin, runtime_data->effect_instance);

  // Get output mesh and take ownership
  Mesh *result = NULL;
  propertySuite->propGetPointer(&output->mesh, kOfxMeshPropInternalData, 0, (void**)&result);
  propertySuite->propSetPointer(&output->mesh, kOfxMeshPropInternalData, 0, NULL);

  printf("==/ mfx_Modifier_do\n");
  return result;
}
