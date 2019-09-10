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

#include "mfxModifier.h"
#include "mfxHost.h"

#include "ofxCore.h"

#include "DNA_mesh_types.h" // Mesh
#include "DNA_meshdata_types.h" // MVert

#include "BKE_mesh.h" // BKE_mesh_new_nomain

#include "BLI_math_vector.h"

// UTILS
// TODO: isn't it already defined somewhere?

#ifdef _WIN32
#else
inline int max(int a, int b) {
  return (a > b) ? a : b;
}

inline int min(int a, int b) {
  return (a < b) ? a : b;
}
#endif

// INTERNALS

/**
 * Convert blender mesh from internal pointer into ofx mesh.
 * /pre no ofx mesh has been allocated or internal pointer is null
 */
static OfxStatus before_mesh_get(OfxHost *host, OfxPropertySetHandle ofx_mesh) {
  OfxPropertySuiteV1 *ps;
  OfxMeshEffectSuiteV1 *mes;
  Mesh *blender_mesh;
  int point_count, vertex_count, face_count;
  float *point_data;
  int *vertex_data, *face_data;

  ps = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);
  mes = (OfxMeshEffectSuiteV1*)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);

  ps->propGetPointer(ofx_mesh, kOfxMeshPropInternalData, 0, (void**)&blender_mesh);

  if (NULL == blender_mesh) {
    printf("NOT converting blender mesh into ofx mesh (no blender mesh)...\n");
    return kOfxStatOK;
  }

  printf("Converting blender mesh into ofx mesh...\n");

  // Set original mesh to null to prevent multiple conversions
  ps->propSetPointer(ofx_mesh, kOfxMeshPropInternalData, 0, NULL);

  point_count = blender_mesh->totvert;
  vertex_count = 0;
  for (int i = 0 ; i < blender_mesh->totpoly ; ++i) {
    int after_last_loop = blender_mesh->mpoly[i].loopstart + blender_mesh->mpoly[i].totloop;
    vertex_count = max(vertex_count, after_last_loop);
  }
  face_count = blender_mesh->totpoly;

  mes->meshAlloc(ofx_mesh, point_count, vertex_count, face_count);

  ps->propGetPointer(ofx_mesh, kOfxMeshPropPointData, 0, (void**)&point_data);
  ps->propGetPointer(ofx_mesh, kOfxMeshPropVertexData, 0, (void**)&vertex_data);
  ps->propGetPointer(ofx_mesh, kOfxMeshPropFaceData, 0, (void**)&face_data);

  // Points (= Blender's vertex)
  for (int i = 0 ; i < point_count ; ++i) {
    copy_v3_v3(point_data + (i * 3), blender_mesh->mvert[i].co);
  }

  // Faces and vertices (~= Blender's loops)
  int current_vertex = 0;
  for (int i = 0 ; i < face_count ; ++i) {
    face_data[i] = blender_mesh->mpoly[i].totloop;
    int l = blender_mesh->mpoly[i].loopstart;
    int end = current_vertex + face_data[i];
    for (; current_vertex < end ; ++current_vertex, ++l) {
      vertex_data[current_vertex] = blender_mesh->mloop[l].v;
    }
  }

  return kOfxStatOK;
}

/**
 * Convert ofx mesh into blender mesh and store it in internal pointer
 */
static OfxStatus before_mesh_release(OfxHost *host, OfxPropertySetHandle ofx_mesh) {
  OfxPropertySuiteV1 *ps;
  Mesh *blender_mesh;
  int point_count, vertex_count, face_count;
  float *point_data;
  int *vertex_data, *face_data;

  ps = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);

  ps->propGetInt(ofx_mesh, kOfxMeshPropPointCount, 0, &point_count);
  ps->propGetInt(ofx_mesh, kOfxMeshPropVertexCount, 0, &vertex_count);
  ps->propGetInt(ofx_mesh, kOfxMeshPropFaceCount, 0, &face_count);
  ps->propGetPointer(ofx_mesh, kOfxMeshPropPointData, 0, (void**)&point_data);
  ps->propGetPointer(ofx_mesh, kOfxMeshPropVertexData, 0, (void**)&vertex_data);
  ps->propGetPointer(ofx_mesh, kOfxMeshPropFaceData, 0, (void**)&face_data);

  ps->propSetPointer(ofx_mesh, kOfxMeshPropInternalData, 0, NULL);

  if (NULL == point_data || NULL == vertex_data || NULL == face_data) {
    printf("WARNING: Null data pointers\n");
    return kOfxStatErrBadHandle;
  }

  blender_mesh = BKE_mesh_new_nomain(point_count, 0, 0, vertex_count, face_count);
  if (NULL == blender_mesh) {
    printf("WARNING: Could not allocate Blender Mesh data\n");
    return kOfxStatErrMemory;
  }

  printf("Converting ofx mesh into blender mesh...\n");

  // Points (= Blender's vertex)
  for (int i = 0 ; i < point_count ; ++i) {
    copy_v3_v3(blender_mesh->mvert[i].co, point_data + (i * 3));
  }

  // Vertices (= Blender's loops)
  for (int i = 0 ; i < vertex_count ; ++i) {
    blender_mesh->mloop[i].v = vertex_data[i];
  }

  // Faces
  int count, current_loop = 0;
  for (int i = 0 ; i < face_count ; ++i) {
    count = face_data[i];
    blender_mesh->mpoly[i].loopstart = current_loop;
    blender_mesh->mpoly[i].totloop = count;
    current_loop += count;
  }

  BKE_mesh_calc_edges(blender_mesh, true, false);

  ps->propSetPointer(ofx_mesh, kOfxMeshPropInternalData, 0, (void*)blender_mesh);

  return kOfxStatOK;
}

// RUNTIME

typedef struct OpenMeshEffectRuntime {
  char current_plugin_path[1024];
  PluginRegistry registry;
  int current_asset_index;
  int num_parameters;

  OfxHost *ofx_host;
  OfxMeshEffectHandle effect_desc;
  OfxMeshEffectHandle effect_instance;
} OpenMeshEffectRuntime;

static void runtime_init(OpenMeshEffectRuntime *rd) {
  rd->current_plugin_path[0] = '\0';
  rd->current_asset_index = 0;
  rd->ofx_host = NULL;
  rd->effect_desc = NULL;
  rd->effect_instance = NULL;
}

void runtime_free_effect_instance(OpenMeshEffectRuntime *rd) {
  if (0 != strcmp(rd->current_plugin_path, "")) {
    OfxPlugin *plugin = rd->registry.plugins[rd->current_asset_index];
    OfxPluginStatus status = rd->registry.status[rd->current_asset_index];

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

bool runtime_ensure_effect_instance(OpenMeshEffectRuntime *rd) {

  if (0 == strcmp(rd->current_plugin_path, "")) {
    printf("current_plugin_path is empty\n");
    return false;
  }

  if (-1 == rd->current_asset_index) {
    printf("No selected plug-in asset\n");
    return false;
  }

  if (NULL == rd->ofx_host) {
    rd->ofx_host = getGlobalHost();

    // Configure host
    OfxPropertySuiteV1 *propertySuite = (OfxPropertySuiteV1*)rd->ofx_host->fetchSuite(rd->ofx_host->host, kOfxPropertySuite, 1);
    // Set custom callbacks
    propertySuite->propSetPointer(rd->ofx_host->host, kOfxHostPropBeforeMeshGetCb, 0, (void*)before_mesh_get);
    propertySuite->propSetPointer(rd->ofx_host->host, kOfxHostPropBeforeMeshReleaseCb, 0, (void*)before_mesh_release);
  }

  OfxPlugin *plugin = rd->registry.plugins[rd->current_asset_index];

  if (NULL == rd->effect_desc) {
    // Load plugin if not already loaded
    OfxPluginStatus *pStatus = &rd->registry.status[rd->current_asset_index];
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

static void runtime_free(OpenMeshEffectRuntime *rd) {
  printf("Unloading OFX plugin %s\n", rd->current_plugin_path);

  runtime_free_effect_instance(rd);

  if (NULL != rd->ofx_host) {
    releaseGlobalHost();
    rd->ofx_host = NULL;
  }

  free_registry(&rd->registry);
}

static bool runtime_set_plugin_path(OpenMeshEffectRuntime *rd, const char *plugin_path) {
  if (0 != strcmp(rd->current_plugin_path, "")) {
    printf("Unloading OFX plugin %s\n", rd->current_plugin_path);
    runtime_free_effect_instance(rd);
    free_registry(&rd->registry);
  }

  strncpy(rd->current_plugin_path, plugin_path, sizeof(rd->current_plugin_path));

  if (0 != strcmp(rd->current_plugin_path, "")) {
    printf("Loading OFX plugin %s\n", rd->current_plugin_path);
    if (false == load_registry(&rd->registry, rd->current_plugin_path)) {
      printf("Failed to load registry.\n");
      free_registry(&rd->registry);
      rd->current_plugin_path[0] = '\0';
      return false;
    }
  }
  return true;
}

void runtime_set_asset_index(OpenMeshEffectRuntime *rd, int asset_index) {
  if (-1 != rd->current_asset_index) {
    runtime_free_effect_instance(rd);
  }

  rd->current_asset_index = asset_index;

  if (-1 != rd->current_asset_index) {
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
    char *name = runtime_data->registry.plugins[i]->pluginIdentifier;
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

void mfx_Modifier_free_runtime_data(void *runtime_data)
{
  printf("== mfx_Modifier_free_runtime_data\n");
  if (runtime_data == NULL) {
    printf("runtime data is null\n");
    printf("==/ mfx_Modifier_free_runtime_data\n");
    return;
  }
  runtime_free((OpenMeshEffectRuntime*)runtime_data);
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
