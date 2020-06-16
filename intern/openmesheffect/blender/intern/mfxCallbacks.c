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

#include <assert.h>

#include "mfxCallbacks.h"
#include "mfxModifier.h"
#include "mfxHost.h"
#include "util/memory_util.h"

#include "DNA_mesh_types.h" // Mesh
#include "DNA_meshdata_types.h" // MVert

#include "BKE_mesh.h" // BKE_mesh_new_nomain
#include "BKE_main.h" // BKE_main_blendfile_path_from_global

#include "BLI_math_vector.h"
#include "BLI_string.h"
#include "BLI_path_util.h"

#define MFX_CHECK(call) { OfxStatus status = call; assert(kOfxStatOK == status); }

OfxStatus before_mesh_get(OfxHost *host, OfxMeshHandle ofx_mesh) {
  OfxPropertySuiteV1 *ps;
  OfxMeshEffectSuiteV1 *mes;
  Mesh *blender_mesh;
  int point_count, vertex_count, face_count;
  MeshInternalData *internal_data;

  ps = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);
  mes = (OfxMeshEffectSuiteV1*)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);

  MFX_CHECK(ps->propGetPointer(&ofx_mesh->properties, kOfxMeshPropInternalData, 0, (void**)&internal_data));

  if (NULL == internal_data) {
    printf("No internal data found\n");
    return kOfxStatErrBadHandle;
  }
  blender_mesh = internal_data->blender_mesh;

  if (false == internal_data->is_input) {
    printf("Output: NOT converting blender mesh\n");
    return kOfxStatOK;
  }

  if (NULL == blender_mesh) {
    printf("NOT converting blender mesh into ofx mesh (no blender mesh, already converted)...\n");
    return kOfxStatOK;
  }

  printf("Converting blender mesh into ofx mesh...\n");

  point_count = blender_mesh->totvert;
  vertex_count = 0;
  for (int i = 0 ; i < blender_mesh->totpoly ; ++i) {
    int after_last_loop = blender_mesh->mpoly[i].loopstart + blender_mesh->mpoly[i].totloop;
    vertex_count = max(vertex_count, after_last_loop);
  }
  face_count = blender_mesh->totpoly;

  MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropPointCount, 0, point_count));
  MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropVertexCount, 0, vertex_count));
  MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropFaceCount, 0, face_count));

  // Define vertex colors attributes
  int vcolor_layers = CustomData_number_of_layers(&blender_mesh->ldata, CD_MLOOPCOL);
  char name[32];
  OfxPropertySetHandle vcolor_attrib;
  for (int k = 0; k < vcolor_layers; ++k) {
    sprintf(name, "color%d", k);
    MFX_CHECK(mes->attributeDefine(ofx_mesh, kOfxMeshAttribVertex, name, 3, kOfxMeshAttribTypeUByte, &vcolor_attrib));
    MFX_CHECK(ps->propSetInt(vcolor_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MLoopCol *vcolor_data = (MLoopCol*)CustomData_get(&blender_mesh->ldata, k, CD_MLOOPCOL);
    MFX_CHECK(ps->propSetPointer(vcolor_attrib, kOfxMeshAttribPropData, 0, (void*)&vcolor_data[0].r));
    MFX_CHECK(ps->propSetInt(vcolor_attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoopCol)));
  }

  // Define vertex UV attributes
  int uv_layers = CustomData_number_of_layers(&blender_mesh->ldata, CD_MLOOPUV);
  name[32];
  OfxPropertySetHandle uv_attrib;
  for (int k = 0; k < uv_layers; ++k) {
    sprintf(name, "uv%d", k);
    MFX_CHECK(mes->attributeDefine(ofx_mesh, kOfxMeshAttribVertex, name, 3, kOfxMeshAttribTypeFloat, &uv_attrib));
    MFX_CHECK(ps->propSetInt(uv_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MLoopUV *uv_data = (MLoopUV*)CustomData_get(&blender_mesh->ldata, k, CD_MLOOPUV);
    MFX_CHECK(ps->propSetPointer(uv_attrib, kOfxMeshAttribPropData, 0, (void*)&uv_data[0].uv[0]));
    MFX_CHECK(ps->propSetInt(uv_attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoopCol)));
  }

  // Point position is non-owned
  OfxPropertySetHandle pos_attrib;
  MFX_CHECK(mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib));
  MFX_CHECK(mes->attributeDefine(ofx_mesh, kOfxMeshAttribVertex, name, 3, kOfxMeshAttribTypeFloat, &vcolor_attrib));
  MFX_CHECK(ps->propSetInt(pos_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
  MFX_CHECK(ps->propSetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void*)&blender_mesh->mvert[0].co[0]));
  MFX_CHECK(ps->propSetInt(pos_attrib, kOfxMeshAttribPropStride, 0, sizeof(MVert)));

  // Vertex point is non-owned

  OfxPropertySetHandle vertpoint_attrib;
  MFX_CHECK(mes->meshGetAttribute(
      ofx_mesh, kOfxMeshAttribVertex, kOfxMeshAttribVertexPoint, &vertpoint_attrib));
  
  MFX_CHECK(ps->propSetInt(vertpoint_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
  MFX_CHECK(ps->propSetPointer(
      vertpoint_attrib, kOfxMeshAttribPropData, 0, (void *)&blender_mesh->mloop[0].v));
  MFX_CHECK(ps->propSetInt(vertpoint_attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoop)));

  // Face count is non-owned
  OfxPropertySetHandle facecounts_attrib;
  MFX_CHECK(mes->meshGetAttribute(
      ofx_mesh, kOfxMeshAttribFace, kOfxMeshAttribFaceCounts, &facecounts_attrib));

  MFX_CHECK(ps->propSetInt(facecounts_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
  MFX_CHECK(ps->propSetPointer(
      facecounts_attrib, kOfxMeshAttribPropData, 0, (void *)&blender_mesh->mpoly[0].totloop));
  MFX_CHECK(ps->propSetInt(facecounts_attrib, kOfxMeshAttribPropStride, 0, sizeof(MPoly)));

  // Eventually this should be avoided: we should be able to have the ofx_mesh be only a proxy to
  // original blender mesh, without any allocation/copy of data.
  MFX_CHECK(mes->meshAlloc(ofx_mesh));

  

  return kOfxStatOK;
}

OfxStatus before_mesh_release(OfxHost *host, OfxMeshHandle ofx_mesh) {
  OfxPropertySuiteV1 *ps;
  OfxMeshEffectSuiteV1 *mes;
  Mesh *source_mesh;
  Mesh *blender_mesh;
  int point_count, vertex_count, face_count;
  float *point_data;
  int *vertex_data, *face_data;
  OfxStatus status;
  MeshInternalData *internal_data;

  ps = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);
  mes = (OfxMeshEffectSuiteV1*)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);

  ps->propGetPointer(&ofx_mesh->properties, kOfxMeshPropInternalData, 0, (void**)&internal_data);

  if (NULL == internal_data) {
    printf("No internal data found\n");
    return kOfxStatErrBadHandle;
  }
  source_mesh = internal_data->source_mesh;

  if (true == internal_data->is_input) {
    printf("Input: NOT converting ofx mesh\n");
    return kOfxStatOK;
  }

  ps->propGetInt(&ofx_mesh->properties, kOfxMeshPropPointCount, 0, &point_count);
  ps->propGetInt(&ofx_mesh->properties, kOfxMeshPropVertexCount, 0, &vertex_count);
  ps->propGetInt(&ofx_mesh->properties, kOfxMeshPropFaceCount, 0, &face_count);

  OfxPropertySetHandle pos_attrib, vertpoint_attrib, facecounts_attrib;
  mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib);
  ps->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void**)&point_data);
  mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribVertex, kOfxMeshAttribVertexPoint, &vertpoint_attrib);
  ps->propGetPointer(vertpoint_attrib, kOfxMeshAttribPropData, 0, (void**)&vertex_data);
  mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribFace, kOfxMeshAttribFaceCounts, &facecounts_attrib);
  ps->propGetPointer(facecounts_attrib, kOfxMeshAttribPropData, 0, (void**)&face_data);

  ps->propSetPointer(&ofx_mesh->properties, kOfxMeshPropInternalData, 0, NULL);

  if (NULL == point_data || NULL == vertex_data || NULL == face_data) {
    printf("WARNING: Null data pointers\n");
    return kOfxStatErrBadHandle;
  }

  if (source_mesh) {
    blender_mesh = BKE_mesh_new_nomain_from_template(source_mesh, point_count, 0, 0, vertex_count, face_count);
  } else {
    printf("Warning: No source mesh\n");
    blender_mesh = BKE_mesh_new_nomain(point_count, 0, 0, vertex_count, face_count);
  }
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

  // Get vertex UVs if UVs are present in the mesh
  int uv_layers = 4;
  char name[32];
  float *ofx_uv_data;
  for (int k = 0; k < uv_layers; ++k) {
    OfxPropertySetHandle uv_attrib;
    sprintf(name, "uv%d", k);
    printf("Look for attribute '%s'\n", name);
    status = mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribVertex, name, &uv_attrib);
    if (kOfxStatOK == status) {
      printf("Found!\n");
      ps->propGetPointer(uv_attrib, kOfxMeshAttribPropData, 0, (void**)&ofx_uv_data);

      // Get UV data pointer in mesh.
      // elie: The next line does not work idk why, hence the next three lines.
      //MLoopUV *uv_data = (MLoopUV*)CustomData_add_layer_named(&blender_mesh->ldata, CD_MLOOPUV, CD_CALLOC, NULL, vertex_count, name);
      char uvname[MAX_CUSTOMDATA_LAYER_NAME];
      CustomData_validate_layer_name(&blender_mesh->ldata, CD_MLOOPUV, name, uvname);
      MLoopUV *uv_data = CustomData_duplicate_referenced_layer_named(&blender_mesh->ldata, CD_MLOOPUV, uvname, vertex_count);

      for (int i = 0; i < vertex_count; ++i) {
        uv_data[i].uv[0] = ofx_uv_data[2 * i + 0];
        uv_data[i].uv[1] = ofx_uv_data[2 * i + 1];
      }
      blender_mesh->runtime.cd_dirty_loop |= CD_MASK_MLOOPUV;
      blender_mesh->runtime.cd_dirty_poly |= CD_MASK_MTFACE;
    }
  }

  BKE_mesh_calc_edges(blender_mesh, true, false);

  internal_data->blender_mesh = blender_mesh;

  return kOfxStatOK;
}
