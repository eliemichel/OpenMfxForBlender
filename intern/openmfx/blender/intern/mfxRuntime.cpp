/**
 * Open Mesh Effect modifier for Blender
 * Copyright (C) 2019 - 2021 Elie Michel
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
#include "PluginRegistryManager.h"
#include <mfxHost/mesheffect>
#include <mfxHost/messages>
#include <mfxHost/MfxHost>
#include "ofxExtras.h"
#include "BlenderMfxHost.h"

#include "DNA_mesh_types.h" // Mesh
#include "DNA_modifier_types.h"
#include "DNA_meshdata_types.h" // MVert

#include "BKE_mesh.h" // BKE_mesh_new_nomain
#include "BKE_main.h" // BKE_main_blendfile_path_from_global
#include "BKE_modifier.h" // BKE_modifier_set_error

#include "BLI_math_vector.h"
#include "BLI_string.h"
#include "BLI_path_util.h"

#include "DEG_depsgraph.h"
#include "DEG_depsgraph_query.h"

#include "MFX_util.h"

#include <vector>
#include <cassert>

#define PluginManager OpenMfx::PluginRegistryManager::GetInstance()

// ----------------------------------------------------------------------------
// Public

OpenMfxRuntime::OpenMfxRuntime()
{
  plugin_path[0] = '\0';
  m_is_plugin_valid = false;
  effect_index = 0;
  mfx_host = nullptr;
  effect_desc = nullptr;
  effect_instance = nullptr;
  registry = nullptr;
}

OpenMfxRuntime::~OpenMfxRuntime()
{
  reset_plugin_path();
}

void OpenMfxRuntime::set_plugin_path(const char *plugin_path)
{
  if (0 == strcmp(this->plugin_path, plugin_path)) {
    return;
  }

  reset_plugin_path();

  strncpy(this->plugin_path, plugin_path, sizeof(this->plugin_path));

  if (0 == strcmp(this->plugin_path, "")) {
    return;
  }

  printf("Loading OFX plugin %s\n", this->plugin_path);
  
  char abs_path[FILE_MAX];
  MFX_normalize_plugin_path(this->plugin_path, abs_path);

  this->registry = PluginManager.getRegistry(abs_path);
  m_is_plugin_valid = this->registry != NULL;
}

void OpenMfxRuntime::set_effect_index(int effect_index)
{
  if (this->effect_index == effect_index) {
    return;
  }
  
  if (-1 != this->effect_index) {
    free_effect_instance();
  }

  if (is_plugin_valid()) {
    this->effect_index = min_ii(max_ii(-1, effect_index), this->registry->num_plugins - 1);
  } else {
    this->effect_index = -1;
  }

  if (-1 != this->effect_index) {
    ensure_effect_instance();
  }
}

void OpenMfxRuntime::get_parameters_from_rna(OpenMfxModifierData *fxmd)
{
  OfxParamSetStruct& parameters = this->effect_instance->parameters;
  assert(parameters.count() == fxmd->num_parameters);
  for (int i = 0 ; i < fxmd->num_parameters ; ++i) {
    copy_parameter_value_from_rna(&parameters[i], fxmd->parameters + i);
  }
}

void OpenMfxRuntime::set_message_in_rna(OpenMfxModifierData *fxmd)
{
  if (NULL == this->effect_instance) {
    return;
  }

  OfxMessageType type = this->effect_instance->messageType;

  if (type != OfxMessageType::Invalid) {
    BLI_strncpy(fxmd->message, this->effect_instance->message, MOD_OPENMFX_MAX_MESSAGE);
    fxmd->message[MOD_OPENMFX_MAX_MESSAGE - 1] = '\0';
  }

  if (type == OfxMessageType::Error || type == OfxMessageType::Fatal) {
    BKE_modifier_set_error(NULL, &fxmd->modifier, this->effect_instance->message);
  }
}

bool OpenMfxRuntime::ensure_effect_instance()
{

  if (false == is_plugin_valid()) {
    return false;
  }

  if (-1 == this->effect_index) {
    printf("No selected plug-in effect\n");
    return false;
  }

  ensure_host();

  OfxPlugin *plugin = this->registry->plugins[this->effect_index];

  if (nullptr == this->effect_desc) {
    // Load plugin if not already loaded
    OfxPluginStatus *pStatus = &this->registry->status[this->effect_index];
    if (OfxPluginStatNotLoaded == *pStatus) {
      if (mfx_host->LoadPlugin(plugin)) {
        *pStatus = OfxPluginStatOK;
      }
      else {
        printf("Error while loading plugin!\n");
        *pStatus = OfxPluginStatError;
        return false;
      }
    }

    if (!mfx_host->GetDescriptor(plugin, this->effect_desc)) {
      this->effect_desc = nullptr;
    }
  }

  if (NULL == this->effect_instance) {
    mfx_host->CreateInstance(this->effect_desc, this->effect_instance);
  }

  return true;
}

bool OpenMfxRuntime::is_plugin_valid() const
{
  return m_is_plugin_valid;
}

void OpenMfxRuntime::save_rna_parameter_values(OpenMfxModifierData *fxmd)
{
  m_saved_parameter_values.clear();
  for (int i = 0; i < fxmd->num_parameters; ++i) {
    OpenMfxParameter* rna = fxmd->parameters + i;
    std::string key = std::string(rna->name);
    copy_parameter_value_from_rna(&m_saved_parameter_values[key], rna);
  }
}

void OpenMfxRuntime::try_restore_rna_parameter_values(OpenMfxModifierData *fxmd)
{
  for (int i = 0; i < fxmd->num_parameters; ++i) {
    OpenMfxParameter *rna = fxmd->parameters + i;
    std::string key = std::string(rna->name);
    if (m_saved_parameter_values.count(key)) {
      copy_parameter_value_to_rna(rna, &m_saved_parameter_values[key]);
    }
  }
}

Mesh *OpenMfxRuntime::cook(OpenMfxModifierData *fxmd,
                           const Depsgraph *depsgraph,
                                  Mesh *mesh,
                                  Object *object)
{
  if (false == this->ensure_effect_instance()) {
    printf("failed to get effect instance\n");
    return NULL;
  }

  OfxMeshInputHandle input, output;
  mfx_host->meshEffectSuite->inputGetHandle(this->effect_instance, kOfxMeshMainInput, &input, NULL);
  mfx_host->meshEffectSuite->inputGetHandle(this->effect_instance, kOfxMeshMainOutput, &output, NULL);

  // Get parameters
  this->get_parameters_from_rna(fxmd);

  // Test if we can skip cooking
  bool isIdentity = true;
  char *inputToPassThrough = nullptr;
  mfx_host->IsIdentity(this->effect_instance, &isIdentity, &inputToPassThrough);

  if (isIdentity) {
    printf("effect is identity, skipping cooking\n");
    // TODO: handle cases where 'inputToPassThrough' is not 'MainInput'
    return mesh;
  }

  // Set input mesh data binding, used by before/after callbacks
  MeshInternalData input_data; // must remain in scope
  if (NULL != input) {
    input_data.is_input = true;
    input_data.blender_mesh = mesh;
    input_data.source_mesh = NULL;
    input_data.object = object;
    mfx_host->propertySuite->propSetPointer(
        &input->mesh.properties, kOfxMeshPropInternalData, 0, (void *)&input_data);
  }

  // Same for extra inputs
  // allocate here so that it last until after the call to ofxhost_cook
  std::vector<MeshInternalData> extra_input_data(fxmd->num_extra_inputs);
  for (int i = 0; i < fxmd->num_extra_inputs; ++i) {
    OfxMeshInputHandle input;
    mfx_host->meshEffectSuite->inputGetHandle(
        this->effect_instance, fxmd->extra_inputs[i].name, &input, NULL);

    Object *object = fxmd->extra_inputs[i].connected_object;

    // TODO: different behaviour when secodary mesh has a mfx_modifier attached to it
    Mesh *mesh = NULL;
    if (object != NULL && fxmd->extra_inputs[i].request_geometry) {
      Object *object_eval = DEG_get_evaluated_object(depsgraph, object);
      mesh = BKE_modifier_get_evaluated_mesh_from_evaluated_object(object_eval, false);
    }
    extra_input_data[i].is_input = true;
    extra_input_data[i].blender_mesh = mesh;
    extra_input_data[i].source_mesh = NULL;
    extra_input_data[i].object = object;

    mfx_host->propertySuite->propSetPointer(
        &input->mesh.properties, kOfxMeshPropInternalData, 0, (void *)&extra_input_data[i]);
  }

  // Set output mesh data binding, used by before/after callbacks
  MeshInternalData output_data;
  output_data.is_input = false;
  output_data.blender_mesh = NULL;
  output_data.source_mesh = mesh;
  output_data.object = object;
  mfx_host->propertySuite->propSetPointer(
      &output->mesh.properties, kOfxMeshPropInternalData, 0, (void *)&output_data);

  if (!mfx_host->Cook(this->effect_instance)) {
    return nullptr;
  }

  // NB: ModifierTypeInfo's doc says a modifier must not free its input
  // so don't free 'mesh' here

  this->set_message_in_rna(fxmd);

  return output_data.blender_mesh;
}

void OpenMfxRuntime::reload_effect_info(OpenMfxModifierData *fxmd)
{
  // Free previous info
  if (NULL != fxmd->effects) {
    MEM_freeN(fxmd->effects);
    fxmd->effects = NULL;
    fxmd->num_effects = 0;
  }

  if (false == this->is_plugin_valid()) {
    return;
  }

  fxmd->num_effects = this->registry->num_plugins;
  fxmd->effects = (OpenMfxEffect *)MEM_calloc_arrayN(
      sizeof(OpenMfxEffect), fxmd->num_effects, "mfx effect info");

  for (int i = 0; i < fxmd->num_effects; ++i) {
    // Get asset name
    const char *name = this->registry->plugins[i]->pluginIdentifier;
    printf("Loading %s to RNA\n", name);
    strncpy(fxmd->effects[i].name, name, sizeof(fxmd->effects[i].name));
  }
}

void OpenMfxRuntime::reload_parameters(OpenMfxModifierData *fxmd)
{
  // Reset parameter DNA
  if (NULL != fxmd->parameters) {
    save_rna_parameter_values(fxmd);
    MEM_freeN(fxmd->parameters);
    fxmd->parameters = NULL;
    fxmd->num_parameters = 0;
  }

  if (NULL == this->effect_desc) {
    return;
  }

  OfxParamSetStruct& parameters = this->effect_desc->parameters;

  fxmd->num_parameters = parameters.count();
  fxmd->parameters = (OpenMfxParameter *)MEM_calloc_arrayN(
      sizeof(OpenMfxParameter), fxmd->num_parameters, "openmesheffect parameter info");

  for (int i = 0; i < fxmd->num_parameters; ++i) {
    const OfxPropertySetStruct & props = parameters[i].properties;
    OpenMfxParameter &rna = fxmd->parameters[i];

    int script_name_idx = props.find(kOfxParamPropScriptName);
    int label_idx = props.find(kOfxPropLabel);

    const char *parameter_name = parameters[i].name;
    const char *system_name = (script_name_idx != -1) ?
                                  props[script_name_idx].value->as_const_char :
                                  parameter_name;
    const char *label_name = (label_idx != -1) ?
                                  props[label_idx].value->as_const_char :
                                  parameter_name;

    strncpy(rna.name, system_name, sizeof(rna.name));
    strncpy(rna.label, label_name, sizeof(rna.label));
    rna.type = static_cast<int>(parameters[i].type);

    int default_idx = props.find(kOfxParamPropDefault);
    if (default_idx > -1) {
      copy_parameter_value_to_rna(&rna, &props[default_idx]);
    }

    // Handle boundaries
    // (TODO: there must be some factorization possible)
    rna.int_min = INT_MIN;
    rna.int_softmin = INT_MIN;
    rna.int_max = INT_MAX;
    rna.int_softmax = INT_MAX;
    rna.float_min = FLT_MIN;
    rna.float_softmin = FLT_MIN;
    rna.float_max = FLT_MAX;
    rna.float_softmax = FLT_MAX;

    int min_idx = props.find(kOfxParamPropMin);
    if (min_idx > -1) {
      copy_parameter_minmax_to_rna(
          rna.type, rna.int_min, rna.float_min, &props[min_idx]);
    }

    int softmin_idx = props.find(kOfxParamPropDisplayMin);
    if (softmin_idx > -1) {
      copy_parameter_minmax_to_rna(
          rna.type, rna.int_softmin, rna.float_softmin, &props[softmin_idx]);
    }
    else if (min_idx > -1) {
      copy_parameter_minmax_to_rna(
          rna.type, rna.int_softmin, rna.float_softmin, &props[min_idx]);
    }

    int max_idx = props.find(kOfxParamPropMax);
    if (max_idx > -1) {
      copy_parameter_minmax_to_rna(
          rna.type, rna.int_max, rna.float_max, &props[max_idx]);
    }

    int softmax_idx = props.find(kOfxParamPropDisplayMax);
    if (softmax_idx > -1) {
      copy_parameter_minmax_to_rna(
          rna.type, rna.int_softmax, rna.float_softmax, &props[softmax_idx]);
    }
    else if (max_idx > -1) {
      copy_parameter_minmax_to_rna(
          rna.type, rna.int_softmax, rna.float_softmax, &props[max_idx]);
    }
  }

  try_restore_rna_parameter_values(fxmd);
}

void OpenMfxRuntime::reload_extra_inputs(OpenMfxModifierData *fxmd)
{
  // Reset parameter DNA
  if (NULL != fxmd->extra_inputs) {
    // save_rna_parameter_values(fxmd); // TODO
    MEM_freeN(fxmd->extra_inputs);
    fxmd->extra_inputs = NULL;
    fxmd->num_extra_inputs = 0;
  }

  if (NULL == this->effect_desc) {
    return;
  }

  OfxMeshInputSetStruct& inputs = this->effect_desc->inputs;

  fxmd->num_extra_inputs = 0;
  for (int i = 0; i < inputs.count(); ++i) {
    if (inputs[i].name() == kOfxMeshMainInput ||
        inputs[i].name() == kOfxMeshMainOutput) {
      continue;
    }
    ++fxmd->num_extra_inputs;
  }

  fxmd->extra_inputs = (OpenMfxInput *)MEM_calloc_arrayN(
      sizeof(OpenMfxInput), fxmd->num_extra_inputs, "openmesheffect extra input info");

  OpenMfxInput *current_input = fxmd->extra_inputs;
  for (int i = 0; i < inputs.count(); ++i) {
    if (inputs[i].name() == kOfxMeshMainInput ||
        inputs[i].name() == kOfxMeshMainOutput) {
      continue;
    }
    const OfxPropertySetStruct &props = inputs[i].properties;
    OpenMfxInput &rna = *current_input;

    int label_idx = props.find(kOfxPropLabel);

    const char *input_name = inputs[i].name().c_str();
    const char *label_name = (label_idx != -1) ?
                                 props[label_idx].value->as_const_char :
                                 input_name;

    strncpy(rna.name, input_name, sizeof(rna.name));
    strncpy(rna.label, label_name, sizeof(rna.label));

    rna.connected_object = NULL;
    rna.request_geometry = true;
    rna.request_transform = false;
    ++current_input;
  }

  // Regular blender loading mechanism should be enough, but is not working for some reason, so
  // TODO handle this manyally (based on object name? Dirty but would work it around...)
  // try_restore_rna_input_values(fxmd);
  set_input_prop_in_rna(fxmd);
}


void OpenMfxRuntime::set_input_prop_in_rna(OpenMfxModifierData *fxmd)
{
  if (NULL == this->effect_desc) {
    return;
  }
  OfxMeshInputSetStruct& inputs = this->effect_desc->inputs;

  OpenMfxInput *current_input = fxmd->extra_inputs;
  for (int i = 0; i < inputs.count(); ++i) {
    if (inputs[i].name() == kOfxMeshMainInput ||
        inputs[i].name() == kOfxMeshMainOutput) {
      continue;
    }
    const OfxPropertySetStruct &props = inputs[i].properties;
    OpenMfxInput &rna = *current_input;

    int request_geometry_idx = props.find(kOfxInputPropRequestGeometry);
    rna.request_geometry = props[request_geometry_idx].value->as_int != 0;

    int request_transform_idx = props.find(kOfxInputPropRequestTransform);
    rna.request_transform = props[request_transform_idx].value->as_int != 0;
    ++current_input;
  }
}

// ----------------------------------------------------------------------------
// Private

void OpenMfxRuntime::free_effect_instance()
{
  if (is_plugin_valid() && -1 != this->effect_index) {
    OfxPlugin *plugin = this->registry->plugins[this->effect_index];
    OfxPluginStatus status = this->registry->status[this->effect_index];

    if (NULL != this->effect_instance) {
      mfx_host->DestroyInstance(this->effect_instance);
      this->effect_instance = NULL;
    }
    if (NULL != this->effect_desc) {
      mfx_host->ReleaseDescriptor(this->effect_desc);
      this->effect_desc = NULL;
    }
    if (OfxPluginStatOK == status) {
      // TODO: loop over all plugins?
      mfx_host->UnloadPlugin(plugin);
      this->registry->status[this->effect_index] = OfxPluginStatNotLoaded;
    }

    this->effect_index = -1;
  }
}

void OpenMfxRuntime::ensure_host()
{
  if (nullptr == this->mfx_host) {
    this->mfx_host = &BlenderMfxHost::GetInstance();
  }
}

void OpenMfxRuntime::reset_plugin_path()
{
  if (is_plugin_valid()) {
    printf("Unloading OFX plugin %s\n", this->plugin_path);
    free_effect_instance();

    char abs_path[FILE_MAX];
    MFX_normalize_plugin_path(this->plugin_path, abs_path);
    PluginManager.releaseRegistry(this->registry);
    m_is_plugin_valid = false;
  }
  this->plugin_path[0] = '\0';
  this->effect_index = -1;
}
