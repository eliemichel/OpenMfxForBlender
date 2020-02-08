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
#include "mfxModifier.h"
#include "mfxHost.h"

#include "DNA_mesh_types.h" // Mesh
#include "DNA_meshdata_types.h" // MVert

#include "BKE_mesh.h" // BKE_mesh_new_nomain
#include "BKE_main.h" // BKE_main_blendfile_path_from_global

#include "BLI_math_vector.h"
#include "BLI_string.h"
#include "BLI_path_util.h"

#ifdef _WIN32
#else
 // UTILS
 // TODO: isn't it already defined somewhere?
inline int max(int a, int b) {
  return (a > b) ? a : b;
}

inline int min(int a, int b) {
  return (a < b) ? a : b;
}
#endif

OfxStatus before_mesh_get(OfxHost *host, OfxMeshHandle ofx_mesh) {
  OfxPropertySuiteV1 *ps;
  OfxMeshEffectSuiteV1 *mes;
  Mesh *blender_mesh;
  int point_count, vertex_count, face_count;
  float *point_data;
  int *vertex_data, *face_data;

  ps = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);
  mes = (OfxMeshEffectSuiteV1*)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);

  ps->propGetPointer(&ofx_mesh->properties, kOfxMeshPropInternalData, 0, (void**)&blender_mesh);

  if (NULL == blender_mesh) {
    printf("NOT converting blender mesh into ofx mesh (no blender mesh)...\n");
    return kOfxStatOK;
  }

  printf("Converting blender mesh into ofx mesh...\n");

  // Set original mesh to null to prevent multiple conversions
  ps->propSetPointer(&ofx_mesh->properties, kOfxMeshPropInternalData, 0, NULL);

  point_count = blender_mesh->totvert;
  vertex_count = 0;
  for (int i = 0 ; i < blender_mesh->totpoly ; ++i) {
    int after_last_loop = blender_mesh->mpoly[i].loopstart + blender_mesh->mpoly[i].totloop;
    vertex_count = max(vertex_count, after_last_loop);
  }
  face_count = blender_mesh->totpoly;

  ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropPointCount, 0, point_count);
  ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropVertexCount, 0, vertex_count);
  ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropFaceCount, 0, face_count);

  mes->meshAlloc(ofx_mesh);

  OfxPropertySetHandle pos_attrib, vertpoint_attrib, facecounts_attrib;
  mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib);
  ps->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void**)&point_data);
  mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribVertex, kOfxMeshAttribVertexPoint, &vertpoint_attrib);
  ps->propGetPointer(vertpoint_attrib, kOfxMeshAttribPropData, 0, (void**)&vertex_data);
  mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribFace, kOfxMeshAttribFaceCounts, &facecounts_attrib);
  ps->propGetPointer(facecounts_attrib, kOfxMeshAttribPropData, 0, (void**)&face_data);

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

  // Free mesh on Blender side
  BKE_mesh_free(blender_mesh);

  return kOfxStatOK;
}

OfxStatus before_mesh_release(OfxHost *host, OfxMeshHandle ofx_mesh) {
  OfxPropertySuiteV1 *ps;
  OfxMeshEffectSuiteV1 *mes;
  Mesh *blender_mesh;
  int point_count, vertex_count, face_count;
  float *point_data;
  int *vertex_data, *face_data;

  ps = (OfxPropertySuiteV1*)host->fetchSuite(host->host, kOfxPropertySuite, 1);
  mes = (OfxMeshEffectSuiteV1*)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);

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

  ps->propSetPointer(&ofx_mesh->properties, kOfxMeshPropInternalData, 0, (void*)blender_mesh);

  return kOfxStatOK;
}
