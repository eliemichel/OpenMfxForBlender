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

#include "DNA_mesh_types.h" // Mesh
#include "DNA_meshdata_types.h" // MVert

#include "BKE_mesh.h" // BKE_mesh_new_nomain
#include "BKE_main.h" // BKE_main_blendfile_path_from_global

#include "BLI_math_vector.h"
#include "BLI_string.h"
#include "BLI_path_util.h"

 // Private

void runtime_free_effect_instance(OpenMeshEffectRuntime *rd) {
  if (rd->is_plugin_valid) {
    OfxPlugin *plugin = rd->registry.plugins[rd->effect_index];
    OfxPluginStatus status = rd->registry.status[rd->effect_index];

    if (NULL != rd->effect_instance) {
      ofxhost_destroy_instance(plugin, rd->effect_instance);
      rd->effect_instance = NULL;
    }
    if (NULL != rd->effect_desc) {
      ofxhost_release_descriptor(rd->effect_desc);
      rd->effect_desc = NULL;
    }
    if (OfxPluginStatOK == status) {
      // TODO: loop over all plugins?
      ofxhost_unload_plugin(plugin);
    }
  }
}

void runtime_ensure_host(OpenMeshEffectRuntime *rd) {
  if (NULL == rd->ofx_host) {
    rd->ofx_host = getGlobalHost();

    // Configure host
    OfxPropertySuiteV1 *propertySuite = (OfxPropertySuiteV1*)rd->ofx_host->fetchSuite(rd->ofx_host->host, kOfxPropertySuite, 1);
    // Set custom callbacks
    propertySuite->propSetPointer(rd->ofx_host->host, kOfxHostPropBeforeMeshGetCb, 0, (void*)before_mesh_get);
    propertySuite->propSetPointer(rd->ofx_host->host, kOfxHostPropBeforeMeshReleaseCb, 0, (void*)before_mesh_release);
  }
}

bool runtime_ensure_effect_instance(OpenMeshEffectRuntime *rd) {

  if (false == rd->is_plugin_valid) {
    return false;
  }

  if (-1 == rd->effect_index) {
    printf("No selected plug-in effect\n");
    return false;
  }

  runtime_ensure_host(rd);

  OfxPlugin *plugin = rd->registry.plugins[rd->effect_index];

  if (NULL == rd->effect_desc) {
    // Load plugin if not already loaded
    OfxPluginStatus *pStatus = &rd->registry.status[rd->effect_index];
    if (OfxPluginStatNotLoaded == *pStatus) {
      if (ofxhost_load_plugin(rd->ofx_host, plugin)) {
        *pStatus = OfxPluginStatOK;
      } else {
        *pStatus = OfxPluginStatError;
        return false;
      }
    }

    ofxhost_get_descriptor(rd->ofx_host, plugin, &rd->effect_desc);
  }

  if (NULL == rd->effect_instance) {
    ofxhost_create_instance(plugin, rd->effect_desc, &rd->effect_instance);
  }

  return true;
}

void runtime_reset_plugin_path(OpenMeshEffectRuntime *rd) {
  if (rd->is_plugin_valid) {
    if (-1 != rd->effect_index) {
      runtime_free_effect_instance(rd);
      rd->effect_index = -1;
    }

    printf("Unloading OFX plugin %s\n", rd->plugin_path);
    runtime_free_effect_instance(rd);
    free_registry(&rd->registry);
    rd->is_plugin_valid = false;
  }
  rd->plugin_path[0] = '\0';
  rd->effect_index = -1;
}

// Public

void runtime_init(OpenMeshEffectRuntime *rd) {
  rd->plugin_path[0] = '\0';
  rd->is_plugin_valid = false;
  rd->effect_index = 0;
  rd->ofx_host = NULL;
  rd->effect_desc = NULL;
  rd->effect_instance = NULL;
}

void runtime_free(OpenMeshEffectRuntime *rd) {
  runtime_reset_plugin_path(rd);

  if (NULL != rd->ofx_host) {
    releaseGlobalHost();
    rd->ofx_host = NULL;
  }
}

void runtime_set_plugin_path(OpenMeshEffectRuntime *rd, const char *plugin_path) {
  if (0 == strcmp(rd->plugin_path, plugin_path)) {
    return;
  }

  runtime_reset_plugin_path(rd);

  strncpy(rd->plugin_path, plugin_path, sizeof(rd->plugin_path));

  if (0 == strcmp(rd->plugin_path, "")) {
    return;
  }

  printf("Loading OFX plugin %s\n", rd->plugin_path);
  // Get absolute path (ui file browser returns relative path for saved files)
  char abs_path[FILE_MAX];
  BLI_strncpy(abs_path, rd->plugin_path, FILE_MAX);
  const char *base_path = BKE_main_blendfile_path_from_global(); // TODO: How to get a bMain object here to avoid "from_global()"?
  if (NULL != base_path) {
    BLI_path_abs(abs_path, base_path);
  }
  if (false == load_registry(&rd->registry, abs_path)) {
    printf("Failed to load registry.\n");
    free_registry(&rd->registry);
    return;
  }
  rd->is_plugin_valid = true;
}

void runtime_set_effect_index(OpenMeshEffectRuntime *rd, int effect_index) {
  if (rd->effect_index == effect_index) {
    return;
  }
  
  if (-1 != rd->effect_index) {
    runtime_free_effect_instance(rd);
  }

  if (rd->is_plugin_valid) {
    rd->effect_index = min(max(-1, effect_index), rd->registry.num_plugins - 1);
  } else {
    rd->effect_index = -1;
  }

  if (-1 != rd->effect_index) {
    runtime_ensure_effect_instance(rd);
  }
}

void runtime_get_parameters_from_rna(OpenMeshEffectRuntime *rd, OpenMeshEffectModifierData *fxmd) {
  OfxParamHandle *parameters = rd->effect_instance->parameters.parameters;
  for (int i = 0 ; i < fxmd->num_parameters ; ++i) {
    switch (parameters[i]->type) {
    case PARAM_TYPE_INTEGER_3D:
      parameters[i]->value[2].as_int = fxmd->parameter_info[i].integer_vec_value[2];
    case PARAM_TYPE_INTEGER_2D:
      parameters[i]->value[1].as_int = fxmd->parameter_info[i].integer_vec_value[1];
    case PARAM_TYPE_INTEGER:
      parameters[i]->value[0].as_int = fxmd->parameter_info[i].integer_vec_value[0];
      break;

    case PARAM_TYPE_SHORT_3D:
      parameters[i]->value[2].as_short = fxmd->parameter_info[i].short_vec_value[2];
    case PARAM_TYPE_SHORT_2D:
      parameters[i]->value[1].as_short = fxmd->parameter_info[i].short_vec_value[1];
    case PARAM_TYPE_SHORT:
      parameters[i]->value[0].as_short = fxmd->parameter_info[i].short_vec_value[0];
      break;
    
    case PARAM_TYPE_RGBA:
      parameters[i]->value[3].as_double = (double)fxmd->parameter_info[i].float_vec_value[3];
    case PARAM_TYPE_DOUBLE_3D:
    case PARAM_TYPE_RGB:
      parameters[i]->value[2].as_double = (double)fxmd->parameter_info[i].float_vec_value[2];
    case PARAM_TYPE_DOUBLE_2D:
      parameters[i]->value[1].as_double = (double)fxmd->parameter_info[i].float_vec_value[1];
    case PARAM_TYPE_DOUBLE:
      parameters[i]->value[0].as_double = (double)fxmd->parameter_info[i].float_vec_value[0];
      break;

    case PARAM_TYPE_BOOLEAN:
      parameters[i]->value[0].as_bool = fxmd->parameter_info[i].integer_vec_value[0];
      break;

    case PARAM_TYPE_STRING:
      parameter_realloc_string(parameters[i], MOD_OPENMESHEFFECT_MAX_STRING_VALUE);
      strncpy(parameters[i]->value[0].as_char, fxmd->parameter_info[i].string_value, MOD_OPENMESHEFFECT_MAX_STRING_VALUE);
      break;

    default:
      printf("-- Skipping parameter %s (unsupported type: %d)\n", parameters[i]->name, parameters[i]->type);
      break;
    }
  }
}
