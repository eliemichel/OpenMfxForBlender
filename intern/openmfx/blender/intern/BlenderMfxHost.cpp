/**
 * OpenMfx modifier for Blender
 * Copyright (C) 2019 - 2022 Elie Michel
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
 * \ingroup openmfx
 */

#include "BlenderMfxHost.h"

#include "util/mfx_util.h"
#include "ofxExtras.h"
#include <mfxHost/mesh>

#include "DNA_mesh_types.h" // Mesh
#include "DNA_meshdata_types.h" // MVert
#include "DNA_object_types.h" // Object

#include "BKE_mesh.h" // BKE_mesh_new_nomain
#include "BKE_main.h" // BKE_main_blendfile_path_from_global
#include "BKE_customdata.h"

#include "BLI_math_vector.h"
#include "BLI_string.h"
#include "BLI_path_util.h"

#include <cassert>

#ifndef max
#  define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

constexpr int MAX_ATTRIB_NAME = 32;

// ----------------------------------------------------------------------------

/**
 * This class is used in the beforeMeshGet callback in order to regroup what
 * happens before and after the memory allocation for a given attribute.
 * Without this, we had to organize the code as
 *   define attrib A
 *   define attrib B
 *   define attrib C
 *   allocate
 *   fill in attrib A
 *   fill in attrib B
 *   fill in attrib C
 * which was hard to maintain as a clean code. Instead, it is now:
 *   add transfer definition A
 *   add transfer definition B
 *   add transfer definition C
 *   automatically call all beforeAllocate()
 *   allocate
 *   automatically call all afterAllocate()
 */
class AttributeTransferDefinition {
  virtual OfxStatus beforeAllocate() { return kOfxStatOK; }
  virtual OfxStatus afterAllocate() { return kOfxStatOK; }
};

class PointPositionTransferDefinition : public AttributeTransferDefinition {
};

// ----------------------------------------------------------------------------

BlenderMfxHost &BlenderMfxHost::GetInstance()
{
  static BlenderMfxHost d;
  return d;
}

// ----------------------------------------------------------------------------

#pragma region [BeforeMeshGet]

OfxStatus BlenderMfxHost::BeforeMeshGet(OfxMeshHandle ofxMesh)
{
  MeshInternalData *internalData = nullptr;

  MFX_CHECK(propertySuite->propGetPointer(
      &ofxMesh->properties, kOfxMeshPropInternalData, 0, (void **)&internalData));

  if (nullptr == internalData) {
    printf("No internal data found\n");
    return kOfxStatErrBadHandle;
  }

  switch (internalData->type) {
    case CallbackContext::Modifier:
      return BeforeMeshGetModifier(ofxMesh,
                                   *reinterpret_cast<MeshInternalDataModifier *>(internalData));
    case CallbackContext::Node:
      return BeforeMeshGetNode(ofxMesh,
                               *reinterpret_cast<MeshInternalDataNode *>(internalData));
    default:
      return kOfxStatErrBadHandle;
  }
}

// ----------------------------------------------------------------------------

OfxStatus BlenderMfxHost::BeforeMeshGetModifier(OfxMeshHandle ofxMesh,
                                                MeshInternalDataModifier &internalData)
{
  Mesh *blenderMesh;
  ElementCounts counts;

  blenderMesh = internalData.blender_mesh;

  if (NULL == internalData.object) {
    // This is the way to tell the plugin that there is no object connected to the input,
    // maybe we should find something clearer.
    return kOfxStatErrBadHandle;
  }

  propSetTransformMatrix(&ofxMesh->properties, internalData.object);

  if (false == internalData.header.is_input) {
    return setupElementCounts(&ofxMesh->properties, counts);
  }

  if (NULL == blenderMesh) {
    printf("NOT converting blender mesh into ofx mesh (no blender mesh, already converted)...\n");
    return kOfxStatOK;
  }

  printf("Converting blender mesh into ofx mesh...\n");

  countMeshElements(blenderMesh, counts);
  MFX_CHECK(setupElementCounts(&ofxMesh->properties, counts));

  CallbackList afterAllocate;
  setupPointPositionAttribute(ofxMesh, blenderMesh);
  setupCornerPointAttribute(ofxMesh, blenderMesh, counts, afterAllocate);
  setupFaceSizeAttribute(ofxMesh, blenderMesh, counts, afterAllocate);
  setupCornerColorAttributes(ofxMesh, "color", blenderMesh, counts, afterAllocate);
  setupCornerUvAttributes(ofxMesh, "uv", blenderMesh, counts, afterAllocate);
  setupFaceMapAttributes(ofxMesh, "faceMap", blenderMesh, counts, afterAllocate);
  setupPointWeightAttributes(ofxMesh, "pointWeight", blenderMesh, counts, afterAllocate);

  // finished adding attributes, allocate any requested buffers
  m_deactivateBeforeAllocateCb = true;
  MFX_CHECK(meshEffectSuite->meshAlloc(ofxMesh));
  m_deactivateBeforeAllocateCb = false;

  for (auto &callback : afterAllocate) {
    callback();
  }

  return kOfxStatOK;
}

// ----------------------------------------------------------------------------

OfxStatus BlenderMfxHost::BeforeMeshGetNode(OfxMeshHandle ofxMesh,
                                            MeshInternalDataNode &internalData)
{
  ElementCounts counts;

    // If the mesh is an output, just initialize an empty mesh
  if (false == internalData.header.is_input) {
    return setupElementCounts(&ofxMesh->properties, counts);
  }

  // If the mesh is an input, copy the mesh component of the input Geometry Set
  // to the ofx Mesh.
  const Mesh *blenderMesh = internalData.geo.get_mesh_for_read();
  if (nullptr == blenderMesh) {
    return kOfxStatErrBadHandle;
  }

  countMeshElements(blenderMesh, counts);

  MFX_CHECK(setupElementCounts(&ofxMesh->properties, counts));

  CallbackList afterAllocate;
  setupPointPositionAttribute(ofxMesh, blenderMesh);
  setupCornerPointAttribute(ofxMesh, blenderMesh, counts, afterAllocate);
  setupFaceSizeAttribute(ofxMesh, blenderMesh, counts, afterAllocate);
  setupCornerColorAttributes(ofxMesh, "color", blenderMesh, counts, afterAllocate);
  setupCornerUvAttributes(ofxMesh, "uv", blenderMesh, counts, afterAllocate);
  setupFaceMapAttributes(ofxMesh, "faceMap", blenderMesh, counts, afterAllocate);
  setupPointWeightAttributes(ofxMesh, "pointWeight", blenderMesh, counts, afterAllocate);

  // finished adding attributes, allocate any requested buffers
  m_deactivateBeforeAllocateCb = true;
  MFX_CHECK(meshEffectSuite->meshAlloc(ofxMesh));
  m_deactivateBeforeAllocateCb = false;

  for (auto &callback : afterAllocate) {
    callback();
  }

  return kOfxStatReplyDefault;
}

#pragma endregion [BeforeMeshGet]

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#pragma region [BeforeMeshRelease]

OfxStatus BlenderMfxHost::BeforeMeshRelease(OfxMeshHandle ofxMesh)
{
  MeshInternalData *internalData = nullptr;

  MFX_CHECK(propertySuite->propGetPointer(
      &ofxMesh->properties, kOfxMeshPropInternalData, 0, (void **)&internalData));

  if (nullptr == internalData) {
    printf("No internal data found\n");
    return kOfxStatErrBadHandle;
  }

  switch (internalData->type) {
    case CallbackContext::Modifier:
      return BeforeMeshReleaseModifier(ofxMesh,
                                       *reinterpret_cast<MeshInternalDataModifier *>(internalData));
    case CallbackContext::Node:
      return BeforeMeshReleaseNode(ofxMesh,
                                   *reinterpret_cast<MeshInternalDataNode *>(internalData));
    default:
      return kOfxStatErrBadHandle;
  }
}

// ----------------------------------------------------------------------------

OfxStatus BlenderMfxHost::BeforeMeshReleaseModifier(OfxMeshHandle ofxMesh,
                                                    MeshInternalDataModifier& internalData)
{
  Mesh *source_mesh;
  Mesh *blenderMesh;
  int ofx_point_count, ofx_corner_count, ofx_face_count, ofx_no_loose_edge, ofx_constant_face_size;
  int blender_poly_count, loose_edge_count, blender_loop_count;
  int point_stride, corner_stride, face_stride;
  char *point_data, *corner_data, *face_data;
  OfxStatus status;

  propFreeTransformMatrix(&ofxMesh->properties);

  source_mesh = internalData.source_mesh;

  if (true == internalData.header.is_input) {
    printf("Input: NOT converting ofx mesh\n");
    return kOfxStatOK;
  }

  MFX_CHECK(propertySuite->propGetInt(&ofxMesh->properties, kOfxMeshPropPointCount, 0, &ofx_point_count));
  MFX_CHECK(propertySuite->propGetInt(&ofxMesh->properties, kOfxMeshPropCornerCount, 0, &ofx_corner_count));
  MFX_CHECK(propertySuite->propGetInt(&ofxMesh->properties, kOfxMeshPropFaceCount, 0, &ofx_face_count));
  MFX_CHECK(propertySuite->propGetInt(&ofxMesh->properties, kOfxMeshPropNoLooseEdge, 0, &ofx_no_loose_edge));
  MFX_CHECK(propertySuite->propGetInt(&ofxMesh->properties, kOfxMeshPropConstantFaceSize, 0, &ofx_constant_face_size));

  if (ofx_point_count < 0 || ofx_corner_count < 0 || ofx_face_count < 0 ||
      (ofx_no_loose_edge != 0 && ofx_no_loose_edge != 1) ||
      (ofx_no_loose_edge == 1 && ofx_constant_face_size == 2 && ofx_face_count > 0) ||
      (ofx_face_count > 0 && (ofx_constant_face_size < 2 && ofx_constant_face_size != -1))) {
    printf("WARNING: Bad mesh property values\n");
    return kOfxStatErrBadHandle;
  }

  // We need to handle stride here. While buffers from meshAlloc() are contiguous, these attributes
  // may have been forwarded from input mesh, in which case they would be strided.
  OfxPropertySetHandle pos_attrib, cornerpoint_attrib, facesize_attrib;
  MFX_CHECK(meshEffectSuite->meshGetAttribute(ofxMesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib));
  MFX_CHECK(propertySuite->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void **)&point_data));
  MFX_CHECK(propertySuite->propGetInt(pos_attrib, kOfxMeshAttribPropStride, 0, &point_stride));
  MFX_CHECK(meshEffectSuite->meshGetAttribute(ofxMesh, kOfxMeshAttribCorner, kOfxMeshAttribCornerPoint, &cornerpoint_attrib));
  MFX_CHECK(propertySuite->propGetPointer(cornerpoint_attrib, kOfxMeshAttribPropData, 0, (void **)&corner_data));
  MFX_CHECK(propertySuite->propGetInt(cornerpoint_attrib, kOfxMeshAttribPropStride, 0, &corner_stride));
  MFX_CHECK(meshEffectSuite->meshGetAttribute(ofxMesh, kOfxMeshAttribFace, kOfxMeshAttribFaceSize, &facesize_attrib));
  MFX_CHECK(propertySuite->propGetPointer(facesize_attrib, kOfxMeshAttribPropData, 0, (void **)&face_data));
  MFX_CHECK(propertySuite->propGetInt(facesize_attrib, kOfxMeshAttribPropStride, 0, &face_stride));

  MFX_CHECK(propertySuite->propSetPointer(&ofxMesh->properties, kOfxMeshPropInternalData, 0, NULL));

  if ((NULL == point_data && ofx_point_count > 0) ||
      (NULL == corner_data && ofx_corner_count > 0) ||
      (NULL == face_data && ofx_face_count > 0 && -1 == ofx_constant_face_size)) {
    printf("WARNING: Null data pointers\n");
    return kOfxStatErrBadHandle;
  }

  // Figure out geometry size on Blender side.
  // Separate true faces (polys) and 2-corner faces (loose edges), to get proper faces/edges in
  // Blender. This requires reinterpretation of OFX face and corner attributes, since we'll
  // "forget" corners associated with loose edges:
  //
  // OFX 2-corners face (ie. edge) -> Blender edge (no loops)
  // OFX n-corners face (ie. poly) -> Blender poly and loops
  if (1 == ofx_no_loose_edge) {
    loose_edge_count = 0;
    if (ofx_constant_face_size == -1) {
      assert(hasNoLooseEdge(ofx_face_count, face_data, face_stride));
    }
  }
  else if (2 == ofx_constant_face_size) {
    loose_edge_count = ofx_face_count;
  }
  else {
    loose_edge_count = 0;
    for (int i = 0; i < ofx_face_count; ++i) {
      int corner_count = *attributeAt<int>(face_data, face_stride, i);
      if (2 == corner_count) {
        ++loose_edge_count;
      }
    }
  }

  blender_poly_count = ofx_face_count - loose_edge_count;
  blender_loop_count = ofx_corner_count - 2 * loose_edge_count;

  printf("Allocating Blender mesh with %d verts %d edges %d loops %d polys\n",
         ofx_point_count,
         loose_edge_count,
         blender_loop_count,
         blender_poly_count);
  if (source_mesh) {
    blenderMesh = BKE_mesh_new_nomain_from_template(
        source_mesh, ofx_point_count, loose_edge_count, 0, blender_loop_count, blender_poly_count);
  }
  else {
    printf("Warning: No source mesh\n");
    blenderMesh = BKE_mesh_new_nomain(
        ofx_point_count, loose_edge_count, 0, ofx_corner_count, blender_poly_count);
  }
  if (NULL == blenderMesh) {
    printf("WARNING: Could not allocate Blender Mesh data\n");
    return kOfxStatErrMemory;
  }

  printf("Converting ofx mesh into blender mesh...\n");

  // copy OFX points (= Blender's vertex)
  for (int i = 0; i < ofx_point_count; ++i) {
    float *p = attributeAt<float>(point_data, point_stride, i);
    copy_v3_v3(blenderMesh->mvert[i].co, p);
  }

  // copy OFX corners (= Blender's loops) + OFX faces (= Blender's faces and edges)
  if (loose_edge_count == 0) {
    // Corners
    for (int i = 0; i < ofx_corner_count; ++i) {
      blenderMesh->mloop[i].v = *attributeAt<int>(corner_data, corner_stride, i);
    }

    // Faces
    int size = ofx_constant_face_size;
    int current_loop = 0;
    for (int i = 0; i < ofx_face_count; ++i) {
      if (-1 == ofx_constant_face_size) {
        size = *attributeAt<int>(face_data, face_stride, i);
      }
      blenderMesh->mpoly[i].loopstart = current_loop;
      blenderMesh->mpoly[i].totloop = size;
      current_loop += size;
    }
  }
  else {
    int size = ofx_constant_face_size;
    int current_poly = 0, current_edge = 0, current_corner_ofx = 0, current_loop_blender = 0;

    for (int i = 0; i < ofx_face_count; ++i) {
      if (-1 == ofx_constant_face_size) {
        size = *attributeAt<int>(face_data, face_stride, i);
      }
      if (2 == size) {
        // make Blender edge, no loops
        blenderMesh->medge[current_edge].v1 = *attributeAt<int>(corner_data, corner_stride, current_corner_ofx);
        blenderMesh->medge[current_edge].v2 = *attributeAt<int>(corner_data, corner_stride, current_corner_ofx + 1);
        blenderMesh->medge[current_edge].flag |= ME_LOOSEEDGE |
                                                  ME_EDGEDRAW;  // see BKE_mesh_calc_edges_loose()

        ++current_edge;
        current_corner_ofx += 2;
      }
      else {
        // make Blender poly and loops
        blenderMesh->mpoly[current_poly].loopstart = current_loop_blender;
        blenderMesh->mpoly[current_poly].totloop = size;

        for (int j = 0; j < size; ++j) {
          blenderMesh->mloop[current_loop_blender + j].v =*attributeAt<int> (corner_data, corner_stride, current_corner_ofx + j);
        }

        ++current_poly;
        current_loop_blender += size;
        current_corner_ofx += size;
      }
    }
  }

  // Get corner UVs if UVs are present in the mesh
  // TODO: Use semantics to get UV layers back from mfx mesh
  int uv_layers = 4;
  char name[MAX_ATTRIB_NAME];
  char *ofx_uv_data;
  int ofx_uv_stride;
  for (int k = 0; k < uv_layers; ++k) {
    OfxPropertySetHandle uv_attrib;
    sprintf(name, "uv%d", k);
    printf("Look for attribute '%s'\n", name);
    status = meshEffectSuite->meshGetAttribute(ofxMesh, kOfxMeshAttribCorner, name, &uv_attrib);
    if (kOfxStatOK == status) {
      printf("Found!\n");
      MFX_CHECK(propertySuite->propGetPointer(uv_attrib, kOfxMeshAttribPropData, 0, (void **)&ofx_uv_data));
      MFX_CHECK(propertySuite->propGetInt(uv_attrib, kOfxMeshAttribPropStride, 0, &ofx_uv_stride));

      if (loose_edge_count > 0) {
        // TODO implement OFX->Blender UV conversion for loose edge meshes
        // we would need to traverse faces too, since we need to skip loose edges
        // and they need not be at the end like in before_mesh_get()
        printf(
            "WARNING: mesh has loose edges, copying UVs is not currently implemented for this "
            "case!\n");
        continue;
      }

      // Get UV data pointer in mesh.
      // elie: The next line does not work idk why, hence the next three lines.
      // MLoopUV *uv_data = (MLoopUV*)CustomData_add_layer_named(&blenderMesh->ldata, CD_MLOOPUV,
      // CD_CALLOC, NULL, ofx_corner_count, name);
      char uvname[MAX_CUSTOMDATA_LAYER_NAME];
      CustomData_validate_layer_name(&blenderMesh->ldata, CD_MLOOPUV, name, uvname);
      MLoopUV *uv_data = (MLoopUV *)CustomData_duplicate_referenced_layer_named(
          &blenderMesh->ldata, CD_MLOOPUV, uvname, ofx_corner_count);

      for (int i = 0; i < ofx_corner_count; ++i) {
        float *uv = attributeAt<float>(ofx_uv_data, ofx_uv_stride, i);
        uv_data[i].uv[0] = uv[0];
        uv_data[i].uv[1] = uv[1];
      }
      blenderMesh->runtime.cd_dirty_loop |= CD_MASK_MLOOPUV;
      blenderMesh->runtime.cd_dirty_poly |= CD_MASK_MTFACE;
    }
  }

  if (blender_poly_count > 0) {
    // if we're here, this dominates before_mesh_get()/before_mesh_release() total running time!
    BKE_mesh_calc_edges(blenderMesh, (loose_edge_count > 0), false);
  }

  internalData.blender_mesh = blenderMesh;

  return kOfxStatOK;
}

// ----------------------------------------------------------------------------

OfxStatus BlenderMfxHost::BeforeMeshReleaseNode(OfxMeshHandle ofxMesh,
                                                MeshInternalDataNode &internalData)
{
  return kOfxStatReplyDefault;
}

#pragma endregion [BeforeMeshRelease]

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

#pragma region [BeforeMeshAllocate]

OfxStatus BlenderMfxHost::BeforeMeshAllocate(OfxMeshHandle ofxMesh)
{
  if (m_deactivateBeforeAllocateCb) {
    return kOfxStatOK;
  }

  MeshInternalData *internalData = nullptr;

  MFX_CHECK(propertySuite->propGetPointer(
      &ofxMesh->properties, kOfxMeshPropInternalData, 0, (void **)&internalData));

  if (nullptr == internalData) {
    printf("No internal data found\n");
    return kOfxStatErrBadHandle;
  }

  switch (internalData->type) {
    case CallbackContext::Modifier:
      return BeforeMeshAllocateModifier(ofxMesh,
                                        *reinterpret_cast<MeshInternalDataModifier *>(internalData));
    case CallbackContext::Node:
      return BeforeMeshAllocateNode(ofxMesh,
                                    *reinterpret_cast<MeshInternalDataNode *>(internalData));
    default:
      return kOfxStatErrBadHandle;
  }
}

// ----------------------------------------------------------------------------

OfxStatus BlenderMfxHost::BeforeMeshAllocateModifier(OfxMeshHandle ofxMesh,
                                                     MeshInternalDataModifier &internalData)
{
  return kOfxStatReplyDefault;
}

// ----------------------------------------------------------------------------

OfxStatus BlenderMfxHost::BeforeMeshAllocateNode(OfxMeshHandle ofxMesh,
                                                 MeshInternalDataNode &internalData)
{
  return kOfxStatReplyDefault;
}

#pragma endregion [BeforeMeshAllocate]

// ----------------------------------------------------------------------------
// ----------------------------------------------------------------------------

bool BlenderMfxHost::hasNoLooseEdge(int face_count,
                                    const char *face_data,
                                    int face_stride)
{
  for (int i = 0; i < face_count; ++i) {
    int corner_count = *attributeAt<int>(face_data, face_stride, i);
    if (2 == corner_count) {
      return false;
    }
  }
  return true;
}

void BlenderMfxHost::propSetTransformMatrix(OfxPropertySetHandle properties,
                                            const Object *object) const
{
  assert(NULL != object);
  double *matrix = new double[16];

  // #pragma omp parallel for
  for (int i = 0; i < 16; ++i) {
    // convert to OpenMeshEffect's row-major order from Blender's column-major
    matrix[i] = static_cast<double>(object->obmat[i % 4][i / 4]);
  }

  MFX_CHECK(propertySuite->propSetPointer(properties, kOfxMeshPropTransformMatrix, 0, (void *)matrix));
}

void BlenderMfxHost::propFreeTransformMatrix(OfxPropertySetHandle properties) const
{
  double *matrix = NULL;
  MFX_CHECK(propertySuite->propGetPointer(properties, kOfxMeshPropTransformMatrix, 0, (void **)&matrix));
  MFX_CHECK(propertySuite->propSetPointer(properties, kOfxMeshPropTransformMatrix, 0, NULL));
  assert(NULL != matrix);
  delete[] matrix;
}

void BlenderMfxHost::countMeshElements(const Mesh *blenderMesh, ElementCounts &counts)
{
  int blenderPolyCount, blenderPointCount;

  // count input geometry on blender side
  blenderPointCount = blenderMesh->totvert;
  counts.blenderLoopCount = 0;
  for (int i = 0; i < blenderMesh->totpoly; ++i) {
    int after_last_loop = blenderMesh->mpoly[i].loopstart + blenderMesh->mpoly[i].totloop;
    counts.blenderLoopCount = max(counts.blenderLoopCount, after_last_loop);
  }
  blenderPolyCount = blenderMesh->totpoly;
  counts.blenderLooseEdgeCount = 0;
  for (int i = 0; i < blenderMesh->totedge; ++i) {
    if (blenderMesh->medge[i].flag & ME_LOOSEEDGE)
      ++counts.blenderLooseEdgeCount;
  }

  // figure out input geometry size on OFX side
  counts.ofxPointCount = blenderPointCount;
  counts.ofxCornerCount = counts.blenderLoopCount;
  counts.ofxFaceCount = blenderPolyCount;
  counts.ofxNoLooseEdge = (counts.blenderLooseEdgeCount > 0) ? 0 : 1;
  counts.ofxConstantFaceSize = (counts.blenderLooseEdgeCount > 0 && blenderPolyCount == 0) ? 2 : -1;

  if (counts.blenderLooseEdgeCount > 0) {
    // turn blender loose edges into 2-corner faces
    counts.ofxCornerCount += 2 * counts.blenderLooseEdgeCount;
    counts.ofxFaceCount += counts.blenderLooseEdgeCount;
    printf("Blender mesh has %d loose edges\n", counts.blenderLooseEdgeCount);
  }
}

OfxStatus BlenderMfxHost::setupElementCounts(OfxPropertySetHandle properties,
                                             const ElementCounts &counts) const
{
  MFX_CHECK(propertySuite->propSetInt(properties, kOfxMeshPropPointCount, 0, counts.ofxPointCount));
  MFX_CHECK(propertySuite->propSetInt(properties, kOfxMeshPropCornerCount, 0, counts.ofxCornerCount));
  MFX_CHECK(propertySuite->propSetInt(properties, kOfxMeshPropFaceCount, 0, counts.ofxFaceCount));
  MFX_CHECK(propertySuite->propSetInt(properties, kOfxMeshPropNoLooseEdge, 0, counts.ofxNoLooseEdge));
  MFX_CHECK(propertySuite->propSetInt(properties, kOfxMeshPropConstantFaceSize, 0, counts.ofxConstantFaceSize));
  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupPointPositionAttribute(OfxMeshHandle ofxMesh,
                                                      const Mesh *blenderMesh) const
{
  OfxPropertySetHandle attrib;
  MFX_CHECK(meshEffectSuite->meshGetAttribute(ofxMesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &attrib));
  MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 0));
  MFX_CHECK(propertySuite->propSetPointer(attrib, kOfxMeshAttribPropData, 0, (void *)&blenderMesh->mvert[0].co[0]));
  MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropStride, 0, sizeof(MVert)));
  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupCornerPointAttribute(OfxMeshHandle ofxMesh,
                                                    const Mesh *blenderMesh,
                                                    const ElementCounts &counts,
                                                    CallbackList &afterAllocate) const
{
  OfxPropertySetHandle attrib;
  MFX_CHECK(meshEffectSuite->meshGetAttribute(ofxMesh, kOfxMeshAttribCorner, kOfxMeshAttribCornerPoint, &attrib));

  if (counts.ofxNoLooseEdge) {
    // use host buffers, kOfxMeshPropNoLooseEdge optimization
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(propertySuite->propSetPointer(attrib, kOfxMeshAttribPropData, 0, (void *)&blenderMesh->mloop[0].v));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoop)));
  }
  else {
    // request new buffer, we need to append new corners for loose edges
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 1));
    afterAllocate.push_back([&]() {
      int *data = nullptr;
      MFX_CHECK(propertySuite->propGetPointer(attrib, kOfxMeshAttribPropData, 0, (void **)&data));

#ifndef NDEBUG
      int stride;
      MFX_CHECK(propertySuite->propGetInt(attrib, kOfxMeshAttribPropStride, 0, &stride));
      assert(stride == sizeof(int));
#endif // NDEBUG

      int c = 0;
      for (int i = 0; i < blenderMesh->totloop; ++i) {
        data[c++] = blenderMesh->mloop[i].v;
      }
      for (int j = 0; j < blenderMesh->totedge; ++j) {
        if (blenderMesh->medge[j].flag & ME_LOOSEEDGE) {
          data[c + 0] = blenderMesh->medge[j].v1;
          data[c + 1] = blenderMesh->medge[j].v2;
          c += 2;
        }
      }
    });
  }

  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupFaceSizeAttribute(OfxMeshHandle ofxMesh,
                                                 const Mesh *blenderMesh,
                                                 const ElementCounts &counts,
                                                 CallbackList &afterAllocate) const
{
  OfxPropertySetHandle attrib;
  MFX_CHECK(meshEffectSuite->meshGetAttribute(ofxMesh, kOfxMeshAttribFace, kOfxMeshAttribFaceSize, &attrib));

  if (-1 != counts.ofxConstantFaceSize) {
    // no buffer, kOfxMeshPropConstantFaceCount optimization
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(propertySuite->propSetPointer(attrib, kOfxMeshAttribPropData, 0, NULL));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropStride, 0, 0));
  }
  else if (counts.ofxNoLooseEdge) {
    // use host buffers, kOfxMeshPropNoLooseEdge optimization
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(propertySuite->propSetPointer(attrib, kOfxMeshAttribPropData, 0, (void *)&blenderMesh->mpoly[0].totloop));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropStride, 0, sizeof(MPoly)));
  }
  else {
    // request new buffer, we need to append new faces for loose edges
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 1));
    afterAllocate.push_back([&]() {
      if (-1 == counts.ofxConstantFaceSize) {
        int *data = nullptr;
        MFX_CHECK(propertySuite->propGetPointer(attrib, kOfxMeshAttribPropData, 0, (void **)&data));

#ifndef NDEBUG
        int stride;
        MFX_CHECK(propertySuite->propGetInt(attrib, kOfxMeshAttribPropStride, 0, &stride));
        assert(stride == sizeof(int));
#endif  // NDEBUG

        int c = 0;
        for (int i = 0; i < blenderMesh->totpoly; ++i) {
          data[c++] = blenderMesh->mpoly[i].totloop;
        }
        for (int j = 0; j < counts.blenderLooseEdgeCount; ++j) {
          data[c++] = 2;
        }
      }
    });
  }

  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupPointWeightAttributes(OfxMeshHandle ofxMesh,
                                                     const char *prefix,
                                                     const Mesh *blenderMesh,
                                                     const ElementCounts &counts,
                                                     CallbackList &afterAllocate) const
{
  // Point weights are not stored as a contiguous memory, we have to copy memory rather than
  // pointing to existing buffers.
  int weightGroupsCount = 0;
  if (nullptr != blenderMesh->dvert) {
    for (int i = 0; i < counts.ofxPointCount; i++) {
      const MDeformVert &deformedVert = blenderMesh->dvert[i];
      for (int w = 0; w < deformedVert.totweight; w++) {
        const int& groupIndex = deformedVert.dw[w].def_nr;
        weightGroupsCount = max(weightGroupsCount, groupIndex + 1);
      }
    }
  }

  std::vector<OfxPropertySetHandle> attribs(weightGroupsCount);
  for (int k = 0; k < weightGroupsCount; k++) {
    std::string name = prefix + std::to_string(k);
    // request new buffer to copy data from existing polys, fill default values for edges
    MFX_CHECK(meshEffectSuite->attributeDefine(ofxMesh,
                                               kOfxMeshAttribPoint,
                                               name.c_str(),
                                               1,
                                               kOfxMeshAttribTypeFloat,
                                               kOfxMeshAttribSemanticWeight,
                                               &attribs[k]));
    MFX_CHECK(propertySuite->propSetInt(attribs[k], kOfxMeshAttribPropIsOwner, 0, 1));
  }

  afterAllocate.push_back([&]() {
    std::vector<float *> buffers(weightGroupsCount, nullptr);
    for (int k = 0; k < weightGroupsCount; ++k) {
      MFX_CHECK(propertySuite->propGetPointer(attribs[k], kOfxMeshAttribPropData, 0, (void **)&buffers[k]));

#ifndef NDEBUG
      int stride;
      MFX_CHECK(propertySuite->propGetInt(attribs[k], kOfxMeshAttribPropStride, 0, &stride));
      assert(stride == sizeof(float));
#endif  // NDEBUG
    }

    for (int i = 0; i < counts.ofxPointCount; i++) {
      const MDeformVert &deformedVert = blenderMesh->dvert[i];
      for (int w = 0; w < weightGroupsCount; w++) {
        buffers[w][i] = 0;
      }
      for (int w = 0; w < deformedVert.totweight; w++) {
        buffers[deformedVert.dw[w].def_nr][i] = deformedVert.dw[w].weight;
      }
    }
  });

  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupCornerColorAttributes(OfxMeshHandle ofxMesh,
                                                     const char *prefix,
                                                     const Mesh *blenderMesh,
                                                     const ElementCounts &counts,
                                                     CallbackList &afterAllocate) const
{
  int vcolor_layers = CustomData_number_of_layers(&blenderMesh->ldata, CD_MLOOPCOL);
  char name[MAX_ATTRIB_NAME];
  for (int k = 0; k < vcolor_layers; ++k) {
    sprintf(name, "%s%d", prefix, k);

    MLoopCol *vcolor_data = (MLoopCol *)CustomData_get_layer_n(&blenderMesh->ldata, CD_MLOOPCOL, k);
    if (nullptr != vcolor_data) {
      setupCornerColorAttribute(ofxMesh, name, vcolor_data, counts, afterAllocate);
    }
  }

  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupCornerUvAttributes(OfxMeshHandle ofxMesh,
                                                  const char *prefix,
                                                  const Mesh *blenderMesh,
                                                  const ElementCounts &counts,
                                                  CallbackList &afterAllocate) const
{
  int uv_layers = CustomData_number_of_layers(&blenderMesh->ldata, CD_MLOOPUV);
  char name[MAX_ATTRIB_NAME];
  for (int k = 0; k < uv_layers; ++k) {
    sprintf(name, "%s%d", prefix, k);

    MLoopUV *uv_data = (MLoopUV *)CustomData_get_layer_n(&blenderMesh->ldata, CD_MLOOPUV, k);
    if (nullptr != uv_data) {
      setupCornerUvAttribute(ofxMesh, name, uv_data, counts, afterAllocate);
    }
  }

  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupFaceMapAttributes(OfxMeshHandle ofxMesh,
                                                 const char *prefix,
                                                 const Mesh *blenderMesh,
                                                 const ElementCounts &counts,
                                                 CallbackList &afterAllocate) const
{
  int fmap_layers = CustomData_number_of_layers(&blenderMesh->pdata, CD_FACEMAP);
  char name[MAX_ATTRIB_NAME];
  for (int k = 0; k < fmap_layers; ++k) {
    sprintf(name, "%s%d", prefix, k);
    // Note: CustomData_get() is not the correct function to call here, since that returns
    // individual values from an "active" layer of given type. We want CustomData_get_layer_n().
    MIntProperty *fmap_data = (MIntProperty *)CustomData_get_layer_n(&blenderMesh->pdata, CD_FACEMAP, k);
    if (nullptr != fmap_data) {
      setupFaceMapAttribute(ofxMesh, name, fmap_data, counts, afterAllocate);
    }
  }

  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupCornerColorAttribute(OfxMeshHandle ofxMesh,
                                                    const char* name,
                                                    const MLoopCol *blenderData,
                                                    const ElementCounts &counts,
                                                    CallbackList &afterAllocate) const
{
  OfxPropertySetHandle attrib;

  if (counts.ofxNoLooseEdge) {
    // reuse host buffer, kOfxMeshPropNoLooseEdge optimization
    MFX_CHECK(meshEffectSuite->attributeDefine(ofxMesh,
                                               kOfxMeshAttribCorner,
                                               name,
                                               3,
                                               kOfxMeshAttribTypeUByte,
                                               kOfxMeshAttribSemanticColor,
                                               &attrib));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(propertySuite->propSetPointer(attrib, kOfxMeshAttribPropData, 0, (void *)&blenderData[0].r));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoopCol)));
  }
  else if (counts.blenderLoopCount > 0) {
    // request new buffer to copy data from existing polys, fill default values for edges
    MFX_CHECK(meshEffectSuite->attributeDefine(ofxMesh,
                                               kOfxMeshAttribCorner,
                                               name,
                                               3,
                                               kOfxMeshAttribTypeUByte,
                                               kOfxMeshAttribSemanticColor,
                                               &attrib));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 1));
    afterAllocate.push_back([&]() {
        unsigned char *data = nullptr;
        MFX_CHECK(propertySuite->propGetPointer(attrib, kOfxMeshAttribPropData, 0, (void **)&data));

# ifndef NDEBUG
        int stride;
        MFX_CHECK(propertySuite->propGetInt(attrib, kOfxMeshAttribPropStride, 0, &stride));
        assert(stride == 3 * sizeof(unsigned char));
# endif // NDEBUG

        for (int i = 0; i < counts.ofxCornerCount; i++) {
          if (i < counts.blenderLoopCount) {
            data[3 * i + 0] = blenderData[i].r;
            data[3 * i + 1] = blenderData[i].g;
            data[3 * i + 2] = blenderData[i].b;
          }
          else {
            data[3 * i + 0] = 0;
            data[3 * i + 1] = 0;
            data[3 * i + 2] = 0;
          }
        }
    });
  }
  else {
    // we have just loose edges, no data to copy
    printf("WARNING: I want to copy corner colors but there are no corners\n");
  }

  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupCornerUvAttribute(OfxMeshHandle ofxMesh,
                                                 const char *name,
                                                 const MLoopUV *blenderData,
                                                 const ElementCounts &counts,
                                                 CallbackList &afterAllocate) const
{
  OfxPropertySetHandle attrib;

  if (counts.ofxNoLooseEdge) {
    // reuse host buffer, kOfxMeshPropNoLooseEdge optimization
    MFX_CHECK(meshEffectSuite->attributeDefine(ofxMesh,
                                               kOfxMeshAttribCorner,
                                               name,
                                               2,
                                               kOfxMeshAttribTypeFloat,
                                               kOfxMeshAttribSemanticTextureCoordinate,
                                               &attrib));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(propertySuite->propSetPointer(attrib, kOfxMeshAttribPropData, 0, (void *)&blenderData[0].uv[0]));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoopUV)));
  }
  else if (counts.blenderLoopCount > 0) {
    // request new buffer to copy data from existing polys, fill default values for edges
    MFX_CHECK(meshEffectSuite->attributeDefine(ofxMesh,
                                               kOfxMeshAttribCorner,
                                               name,
                                               2,
                                               kOfxMeshAttribTypeFloat,
                                               kOfxMeshAttribSemanticTextureCoordinate,
                                               &attrib));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 1));
    afterAllocate.push_back([&]() {
      float *data = nullptr;
      MFX_CHECK(propertySuite->propGetPointer(attrib, kOfxMeshAttribPropData, 0, (void **)&data));

#ifndef NDEBUG
      int stride;
      MFX_CHECK(propertySuite->propGetInt(attrib, kOfxMeshAttribPropStride, 0, &stride));
      assert(stride == 2 * sizeof(float));
#endif  // NDEBUG

      for (int i = 0; i < counts.ofxCornerCount; i++) {
        if (i < counts.blenderLoopCount) {
          copy_v2_v2(&data[2 * i], blenderData[i].uv);
        }
        else {
          zero_v2(&data[2 * i]);
        }
      }
    });
  }
  else {
    // we have just loose edges, no data to copy
    printf("WARNING: I want to copy UV but there are no corners\n");
  }

  return kOfxStatOK;
}

OfxStatus BlenderMfxHost::setupFaceMapAttribute(OfxMeshHandle ofxMesh,
                                const char *name,
                                const MIntProperty *blenderData,
                                const ElementCounts &counts,
                                CallbackList &afterAllocate) const
{
  OfxPropertySetHandle attrib;

  if (counts.ofxNoLooseEdge) {
    // reuse host buffer, kOfxMeshPropNoLooseEdge optimization
    MFX_CHECK(meshEffectSuite->attributeDefine(ofxMesh,
                                               kOfxMeshAttribFace,
                                               name,
                                               1,
                                               kOfxMeshAttribTypeInt,
                                               kOfxMeshAttribSemanticWeight,
                                               &attrib));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(propertySuite->propSetPointer(attrib, kOfxMeshAttribPropData, 0, (void *)blenderData));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropStride, 0, sizeof(MIntProperty)));
  }
  else if (counts.ofxFaceCount > counts.blenderLooseEdgeCount) {
    // if there are faces other than loose edges request new buffer to copy
    // data from existing polys, fill default values for edges
    MFX_CHECK(meshEffectSuite->attributeDefine(ofxMesh,
                                               kOfxMeshAttribFace,
                                               name,
                                               1,
                                               kOfxMeshAttribTypeInt,
                                               kOfxMeshAttribSemanticWeight,
                                               &attrib));
    MFX_CHECK(propertySuite->propSetInt(attrib, kOfxMeshAttribPropIsOwner, 0, 1));
    afterAllocate.push_back([&]() {
      int *data = nullptr;
      MFX_CHECK(propertySuite->propGetPointer(attrib, kOfxMeshAttribPropData, 0, (void **)&data));

#ifndef NDEBUG
      int stride;
      MFX_CHECK(propertySuite->propGetInt(attrib, kOfxMeshAttribPropStride, 0, &stride));
      assert(stride == sizeof(int));
#endif  // NDEBUG

      int c = 0;
      for (int i = 0; i < counts.ofxFaceCount; ++i) {
        data[c++] = blenderData[i].i;
      }
      for (int j = 0; j < counts.blenderLooseEdgeCount; ++j) {
        data[c++] = -1;
      }
    });
  }
  else {
    // we have just loose edges, no data to copy
    printf("WARNING: I want to copy faceMaps but there are no faces\n");
  }

  return kOfxStatOK;
}