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

// RUNTIME

typedef struct OpenMeshEffectRuntime {
  char current_plugin_path[1024];
  PluginRegistry registry;
  int current_asset_index;
  int asset_count, num_parameters;
} OpenMeshEffectRuntime;

static void runtime_init(OpenMeshEffectRuntime *rd) {
  rd->current_plugin_path[0] = '\0';
}

static void runtime_free(OpenMeshEffectRuntime *rd) {
  printf("Unloading OFX plugin %s\n", rd->current_plugin_path);
  free_registry(&rd->registry);
}

static bool runtime_set_plugin_path(OpenMeshEffectRuntime *rd, const char *plugin_path) {
  if (0 != strcmp(rd->current_plugin_path, "")) {
    printf("Unloading OFX plugin %s\n", rd->current_plugin_path);
    free_registry(&rd->registry);
  }
  strncpy(rd->current_plugin_path, plugin_path, sizeof(rd->current_plugin_path));
  printf("Loading OFX plugin %s\n", rd->current_plugin_path);
  if (false == load_registry(&rd->registry, plugin_path)) {
    free_registry(&rd->registry);
    rd->current_plugin_path[0] = '\0';
    return false;
  }
  return true;
}

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

// PUBLIC API

static OpenMeshEffectRuntime *mfx_Modifier_runtime_ensure(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_runtime_ensure on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = (OpenMeshEffectRuntime*)fxmd->modifier.runtime;

  // Init
  if (NULL == runtime_data) {
    fxmd->modifier.runtime = MEM_callocN(sizeof(OpenMeshEffectRuntime), "mfx runtime");
    runtime_data = (OpenMeshEffectRuntime*)fxmd->modifier.runtime;
    runtime_init(runtime_data);
  }

  // Update
  bool plugin_changed = 0 != strcmp(runtime_data->current_plugin_path, fxmd->plugin_path);
  
  if (plugin_changed) {
    if (false == runtime_set_plugin_path(runtime_data, fxmd->plugin_path)) {
      fxmd->plugin_path[0] = '\0';
    }
  }

  return (OpenMeshEffectRuntime *)fxmd->modifier.runtime;
}

void mfx_Modifier_on_plugin_changed(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_on_plugin_changed on data %p\n", fxmd);
  OpenMeshEffectRuntime *runtime_data = mfx_Modifier_runtime_ensure(fxmd);

  // Free previous info
  if (NULL != fxmd->asset_info) {
    MEM_freeN(fxmd->asset_info);
    fxmd->asset_info = NULL;
  }

  fxmd->num_assets = runtime_data->asset_count;
  fxmd->asset_info = MEM_calloc_arrayN(sizeof(OpenMeshEffectAssetInfo), fxmd->num_assets, "mfx asset info");

  char name[MOD_OPENMESHEFFECT_MAX_ASSET_NAME];
  for (int i = 0 ; i < fxmd->num_assets ; ++i) {
    // Get asset name
    strncpy(fxmd->asset_info[i].name, name, sizeof(fxmd->asset_info[i].name));
  }

  // move to on_asset_changed
  if (0 == strcmp(runtime_data->current_plugin_path, "")) {
    return;
  }

  if (NULL != fxmd->parameter_info) {
    MEM_freeN(fxmd->parameter_info);
    fxmd->parameter_info = NULL;
  }

  OfxHost *ofxHost = getGlobalHost();
  OfxMeshEffectHandle effect_desc;
  OfxPlugin *plugin = runtime_data->registry.plugins[0];
  ofxhost_load_plugin(ofxHost, plugin);
  ofxhost_get_descriptor(ofxHost, plugin, &effect_desc);

  OfxParamSetHandle parameters = &effect_desc->parameters;

  fxmd->num_parameters = parameters->num_parameters;
  fxmd->parameter_info = MEM_calloc_arrayN(sizeof(OpenMeshEffectParameterInfo), fxmd->num_parameters, "openmesheffect parameter info");

  for (int i = 0 ; i < fxmd->num_parameters ; ++i) {
    strncpy(fxmd->parameter_info[i].name, parameters->parameters[i]->name, sizeof(fxmd->parameter_info[i].name));
    strncpy(fxmd->parameter_info[i].label, parameters->parameters[i]->name, sizeof(fxmd->parameter_info[i].label));
    fxmd->parameter_info[i].type = parameters->parameters[i]->type;
  }

  ofxhost_release_descriptor(effect_desc);
  ofxhost_unload_plugin(plugin);
  releaseGlobalHost();
}

void mfx_Modifier_on_library_changed(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_on_library_changed on data %p\n", fxmd);
}

void mfx_Modifier_on_asset_changed(OpenMeshEffectModifierData *fxmd) {
  printf("== mfx_Modifier_on_asset_changed on data %p\n", fxmd);
}

void mfx_Modifier_free_runtime_data(void *runtime_data)
{
  printf("== mfx_Modifier_free_runtime_data\n");
  if (runtime_data == NULL) {
    return;
  }
  runtime_free((OpenMeshEffectRuntime*)runtime_data);
}

Mesh * mfx_Modifier_do(OpenMeshEffectModifierData *fxmd, Mesh *mesh)
{
  printf("== mfx_Modifier_do on data %p\n", fxmd);
  Mesh *result = NULL;
  OpenMeshEffectRuntime *runtime_data = mfx_Modifier_runtime_ensure(fxmd);

  if (0 == strcmp(runtime_data->current_plugin_path, "")) {
    return NULL;
  }

  OfxHost *ofxHost = getGlobalHost();
  OfxPropertySuiteV1 *propertySuite = (OfxPropertySuiteV1*)ofxHost->fetchSuite(ofxHost->host, kOfxPropertySuite, 1);
  OfxMeshEffectSuiteV1 *meshEffectSuite = (OfxMeshEffectSuiteV1*)ofxHost->fetchSuite(ofxHost->host, kOfxMeshEffectSuite, 1);

  OfxMeshEffectHandle effect_desc, effect_instance;
  OfxMeshInputHandle input, output;

  // Set custom callbacks
  propertySuite->propSetPointer(ofxHost->host, kOfxHostPropBeforeMeshGetCb, 0, (void*)before_mesh_get);
  propertySuite->propSetPointer(ofxHost->host, kOfxHostPropBeforeMeshReleaseCb, 0, (void*)before_mesh_release);

  OfxPlugin *plugin = runtime_data->registry.plugins[0];
  ofxhost_load_plugin(ofxHost, plugin);
  ofxhost_get_descriptor(ofxHost, plugin, &effect_desc);

  ofxhost_create_instance(plugin, effect_desc, &effect_instance);

  // Set params
  OfxParamHandle *parameters = effect_instance->parameters.parameters;
  for (int i = 0 ; i < fxmd->num_parameters ; ++i) {
    switch (parameters[i]->type) {
    case PARAM_TYPE_DOUBLE:
      parameters[i]->value[0].as_double = (double)fxmd->parameter_info[i].float_value;
      break;
    case PARAM_TYPE_INT:
      parameters[i]->value[0].as_int = fxmd->parameter_info[i].int_value;
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

  // Set input mesh
  meshEffectSuite->inputGetHandle(effect_instance, kOfxMeshMainInput, &input, NULL);
  propertySuite->propSetPointer(&input->mesh, kOfxMeshPropInternalData, 0, (void*)mesh);

  ofxhost_cook(plugin, effect_instance);

  // Get output mesh
  // do not use inputGetMesh here to avoid allocating useless data, use output->mesh instead
  meshEffectSuite->inputGetHandle(effect_instance, kOfxMeshMainOutput, &output, NULL);
  propertySuite->propGetPointer(&output->mesh, kOfxMeshPropInternalData, 0, (void**)&result);

  // FIXME: Memory leak: the input data is converted back into a new blender mesh that is never freed

  ofxhost_destroy_instance(plugin, effect_instance);

  // TODO: move to free runtime or somewhere like that
  //ofxhost_release_descriptor(effect_desc);
  //ofxhost_unload_plugin(plugin);
  //releaseGlobalHost();

  return result;
}
