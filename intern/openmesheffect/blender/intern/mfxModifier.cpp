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

#include "BLI_math_vector.h"
#include "BLI_path_util.h"
#include "BLI_string.h"


/**
 * Ensure that fxmd->modifier.runtime points to a valid OpenMeshEffectRuntime and return
 * this poitner, correctly casted.
 * (idempotent)
 */
static OpenMeshEffectRuntime *mfx_Modifier_runtime_ensure(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_runtime_ensure on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime = (OpenMeshEffectRuntime *)fxmd->modifier.runtime;
  printf("RUNTIME DATA @%p\n", runtime);

  // Init
  if (NULL == runtime) {
    runtime = new OpenMeshEffectRuntime();
    fxmd->modifier.runtime = runtime; 
    printf("NEW RUNTIME DATA @%p\n", runtime);
  }

  // Update
  runtime->set_plugin_path(fxmd->plugin_path);
  runtime->set_effect_index(fxmd->effect_index);

  if (false == runtime->is_plugin_valid()) {
    modifier_setError(&fxmd->modifier, "Could not load ofx plugins!");
  }

  printf("==/ mfx_Modifier_runtime_ensure\n");
  return (OpenMeshEffectRuntime *)fxmd->modifier.runtime;
}

void mfx_Modifier_reload_effect_info(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_reload_effect_info on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = mfx_Modifier_runtime_ensure(fxmd);

  // Free previous info
  if (NULL != fxmd->effect_info) {
    MEM_freeN(fxmd->effect_info);
    fxmd->effect_info = NULL;
    fxmd->num_effects = 0;
  }

  if (false == runtime_data->is_plugin_valid()) {
    printf("==/ mfx_Modifier_reload_effect_info\n");
    return;
  }

  fxmd->num_effects = runtime_data->registry->num_plugins;
  fxmd->effect_info = (OpenMeshEffectEffectInfo*)MEM_calloc_arrayN(sizeof(OpenMeshEffectEffectInfo), fxmd->num_effects, "mfx effect info");

  for (int i = 0; i < fxmd->num_effects; ++i) {
    // Get asset name
    const char *name = runtime_data->registry->plugins[i]->pluginIdentifier;
    printf("Loading %s to RNA\n", name);
    strncpy(fxmd->effect_info[i].name, name, sizeof(fxmd->effect_info[i].name));
  }

  printf("==/ mfx_Modifier_reload_effect_info\n");
}

void mfx_Modifier_on_plugin_changed(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_on_plugin_changed on data %p\n", fxmd);

  mfx_Modifier_reload_effect_info(fxmd);
  mfx_Modifier_on_effect_changed(fxmd);

  printf("==/ mfx_Modifier_on_plugin_changed\n");
}

void mfx_Modifier_on_effect_changed(OpenMeshEffectModifierData *fxmd) {
  printf("==. mfx_Modifier_on_asset_changed on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = mfx_Modifier_runtime_ensure(fxmd);

  // Reset parameter DNA
  if (NULL != fxmd->parameter_info) {
    runtime_data->save_rna_parameter_values(fxmd);
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
  fxmd->parameter_info = (OpenMeshEffectParameterInfo*)MEM_calloc_arrayN(sizeof(OpenMeshEffectParameterInfo), fxmd->num_parameters, "openmesheffect parameter info");

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

    int default_idx = find_property(&props, kOfxParamPropDefault);
    if (default_idx > -1) {
      copy_parameter_value_to_rna(&fxmd->parameter_info[i], props.properties[default_idx]);
    }
  }

  runtime_data->try_restore_rna_parameter_values(fxmd);

  printf("==/ mfx_Modifier_on_asset_changed on data %p\n", fxmd);
}

void mfx_Modifier_free_runtime_data(void * runtime_data)
{
  OpenMeshEffectRuntime * rd = (OpenMeshEffectRuntime *)runtime_data;
  printf("== mfx_Modifier_free_runtime_data\n");
  if (NULL != rd) {
    delete rd;
  }
  
  printf("==/ mfx_Modifier_free_runtime_data\n");
}

Mesh * mfx_Modifier_do(OpenMeshEffectModifierData *fxmd, Mesh *mesh)
{
  printf("== mfx_Modifier_do on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime = mfx_Modifier_runtime_ensure(fxmd);

  if (false == runtime->ensure_effect_instance()) {
    printf("failed to get effect instance\n");
    printf("==/ mfx_Modifier_do\n");
    return NULL;
  }

  OfxHost *ofxHost = runtime->ofx_host;
  OfxMeshEffectSuiteV1 *meshEffectSuite = (OfxMeshEffectSuiteV1*)ofxHost->fetchSuite(ofxHost->host, kOfxMeshEffectSuite, 1);
  OfxPropertySuiteV1 *propertySuite = (OfxPropertySuiteV1*)ofxHost->fetchSuite(ofxHost->host, kOfxPropertySuite, 1);

  OfxMeshInputHandle input, output;
  meshEffectSuite->inputGetHandle(runtime->effect_instance, kOfxMeshMainInput, &input, NULL);
  meshEffectSuite->inputGetHandle(runtime->effect_instance, kOfxMeshMainOutput, &output, NULL);
  
  // Get parameters
  runtime->get_parameters_from_rna(fxmd);

  // Test if we can skip cooking
  OfxPlugin *plugin = runtime->registry->plugins[runtime->effect_index];
  bool shouldCook = true;
  ofxhost_is_identity(plugin, runtime->effect_instance, &shouldCook);

  if (false == shouldCook) {
    printf("effect is identity, skipping cooking\n");
    printf("==/ mfx_Modifier_do\n");
    return mesh;
  }

  // Set input mesh data binding, used by before/after callbacks
  MeshInternalData input_data;
  input_data.is_input = true;
  input_data.blender_mesh = mesh;
  input_data.source_mesh = NULL;
  propertySuite->propSetPointer(&input->mesh.properties, kOfxMeshPropInternalData, 0, (void*)&input_data);

  // Set output mesh data binding, used by before/after callbacks
  MeshInternalData output_data;
  output_data.is_input = false;
  output_data.blender_mesh = NULL;
  output_data.source_mesh = mesh;
  propertySuite->propSetPointer(&output->mesh.properties, kOfxMeshPropInternalData, 0, (void*)&output_data);

  ofxhost_cook(plugin, runtime->effect_instance);

  // Free mesh on Blender side
  if (NULL != output_data.blender_mesh && output_data.blender_mesh != output_data.source_mesh) {
    BKE_mesh_free(output_data.source_mesh);
  }

  runtime->set_message_in_rna(fxmd);

  printf("==/ mfx_Modifier_do\n");
  return output_data.blender_mesh;
}

void mfx_Modifier_copyData(OpenMeshEffectModifierData *source, OpenMeshEffectModifierData *destination)
{
  if (source->parameter_info) {
    destination->parameter_info = (OpenMeshEffectParameterInfo*)MEM_dupallocN(
        source->parameter_info);
  }

  if (source->effect_info) {
    destination->effect_info = (OpenMeshEffectEffectInfo*)MEM_dupallocN(source->effect_info);
  }

  OpenMeshEffectRuntime *runtime = (OpenMeshEffectRuntime *)source->modifier.runtime;
  if (NULL != runtime) {
    runtime->set_message_in_rna(destination);
  }
}
