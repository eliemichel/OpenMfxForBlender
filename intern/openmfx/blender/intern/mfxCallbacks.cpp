/**
 * OpenMfx modifier for Blender
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

#include <cassert>

#include "mfxCallbacks.h"
#include "mfxModifier.h"
#include "ofxExtras.h"
#include "mfxHost.h"
#include <mfxHost/mesh>
#include "util/memory_util.h"

#include "DNA_mesh_types.h" // Mesh
#include "DNA_meshdata_types.h" // MVert

#include "BKE_mesh.h" // BKE_mesh_new_nomain
#include "BKE_main.h" // BKE_main_blendfile_path_from_global

#include "BLI_math_vector.h"
#include "BLI_string.h"
#include "BLI_path_util.h"

#define MFX_CHECK(call) { \
  OfxStatus status = call; \
  assert(kOfxStatOK == status); \
  if (kOfxStatOK != status) printf("ERROR: Mfx suite call '" #call "' failed with status %d!\n", status); \
}

#ifndef max
#define max(a,b) (((a) > (b)) ? (a) : (b))
#endif

constexpr int MAX_CORNER_ATTRIB_NAME = 32;

// TODO: this is provided already in some utility header, avoid replicating it here
template <typename T>
T* attributeAt(char *buffer, int byteStride, int index) {
  return reinterpret_cast<T*>(buffer + index * byteStride);
}

/**
 * Class holding suite variables, exposing them to the scope more conveniently
 * than if we would have to pass them around.
 */
class Converter {
 public:
  Converter(OfxHost *host);
  Converter(const Converter &) = delete;
  Converter &operator=(const Converter &) = delete;

  /**
   * @brief Convert Blender mesh to Open Mesh Effect mesh if necessary
   *
   * This function prepares mesh for the effect, reusing Blender buffers as much as possible.
   * Due to different representation of edges in Blender and Open Mesh Effect, there are two
   * main code paths:
   *
   * 1) Input mesh has no loose edges. This is the fast path, which allows us to reuse all
   *    Blender buffers. Typical polygonal meshes and point clouds take this path.
   * 2) Input mesh has some loose edges. For Open Mesh Effect, we convert these into 2-corner faces,
   *    which means that we no longer have the same number of corners/faces as in Blender mesh,
   *    and we have to copy all buffers except point coordinates.
   *
   * For 2), the 2-corner faces are added to the end, after any proper faces. There is an optimization
   * for the case when there are no proper faces, just loose edges (ie. edge wireframe) - in this case,
   * we use kOfxMeshPropConstantFaceCount instead of face count buffer.
   *
   * This function will also convert any corner color and UV attributes.
   */
  OfxStatus blenderToMfx(OfxMeshHandle ofx_mesh) const;

  /**
   * @brief Convert Open Mesh Effect mesh to Blender mesh
   *
   * This function receives output mesh from the effect, converting it into new Blender mesh.
   * We have to filter out any 2-corner faces and turn them into Blender loose edges.
   *
   * This function will also convert UV attributes called uv0, uv1, uv2, uv3.
   */
  OfxStatus mfxToBlender(OfxMeshHandle ofx_mesh) const;

private:
  static bool check_no_loose_edges_in_ofx_mesh(int face_count,
                                               const int *face_data,
                                               int face_stride);

private:
  /**
   * Copy the model to world matrix from the blender object to target mesh properties.
   * This allocate new data that must be eventually freed using propFreeTransformMatrix()
   */
  void propSetTransformMatrix(OfxPropertySetHandle properties, const Object *object) const;

  /**
   * Free data that had been allocated for transform matrix
   */
  void propFreeTransformMatrix(OfxPropertySetHandle properties) const;

  /**
   * Count the number of point/corner/face in Blender mesh, as well as loose edge
   * and constant face size flags.
   * (it's static because it does not use the suites)
   */
  static void countMeshElements(Mesh * blender_mesh,
                               int & ofx_point_count,
                               int & ofx_corner_count,
                               int & ofx_face_count,
                               int & ofx_no_loose_edge,
                               int & ofx_constant_face_size,
                               int & blender_loop_count,
                               int & blender_loose_edge_count);


private:
  OfxPropertySuiteV1 *ps;
  OfxMeshEffectSuiteV1 *mes;
};

Converter::Converter(OfxHost *host)
{
  ps = (OfxPropertySuiteV1 *)host->fetchSuite(host->host, kOfxPropertySuite, 1);
  mes = (OfxMeshEffectSuiteV1 *)host->fetchSuite(host->host, kOfxMeshEffectSuite, 1);
}

// ----------------------------------------------------------------------------

OfxStatus Converter::blenderToMfx(OfxMeshHandle ofx_mesh) const
{
  Mesh *blender_mesh;
  int ofx_point_count, ofx_corner_count, ofx_face_count, ofx_no_loose_edge,
      ofx_constant_face_size;
  int blender_loop_count, blender_loose_edge_count;
  MeshInternalData *internal_data;

  MFX_CHECK(ps->propGetPointer(
      &ofx_mesh->properties, kOfxMeshPropInternalData, 0, (void **)&internal_data));

  if (NULL == internal_data) {
    printf("No internal data found\n");
    return kOfxStatErrBadHandle;
  }
  blender_mesh = internal_data->blender_mesh;

  if (NULL == internal_data->object) {
    // This is the way to tell the plugin that there is no object connected to the input,
    // maybe we should find something clearer.
    return kOfxStatErrBadHandle;
  }

  propSetTransformMatrix(&ofx_mesh->properties, internal_data->object);

  if (false == internal_data->is_input) {
    // Initialize counters to zero
    MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropPointCount, 0, 0));
    MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropCornerCount, 0, 0));
    MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropFaceCount, 0, 0));

    MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropNoLooseEdge, 0, 1));
    MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropConstantFaceSize, 0, -1));

    printf("Output: NOT converting blender mesh\n");
    return kOfxStatOK;
  }

  if (NULL == blender_mesh) {
    printf("NOT converting blender mesh into ofx mesh (no blender mesh, already converted)...\n");
    return kOfxStatOK;
  }

  printf("Converting blender mesh into ofx mesh...\n");

  countMeshElements(blender_mesh,
                    ofx_point_count,
                    ofx_corner_count,
                    ofx_face_count,
                    ofx_no_loose_edge,
                    ofx_constant_face_size,
                    blender_loop_count,
                    blender_loose_edge_count);

  MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropPointCount, 0, ofx_point_count));
  MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropCornerCount, 0, ofx_corner_count));
  MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropFaceCount, 0, ofx_face_count));
  MFX_CHECK(ps->propSetInt(&ofx_mesh->properties, kOfxMeshPropNoLooseEdge, 0, ofx_no_loose_edge));
  MFX_CHECK(ps->propSetInt(
      &ofx_mesh->properties, kOfxMeshPropConstantFaceSize, 0, ofx_constant_face_size));

  // Define corner colors attributes
  int vcolor_layers = CustomData_number_of_layers(&blender_mesh->ldata, CD_MLOOPCOL);
  char name[MAX_CORNER_ATTRIB_NAME];
  OfxPropertySetHandle vcolor_attrib;
  for (int k = 0; k < vcolor_layers; ++k) {
    sprintf(name, "color%d", k);

    // Note: CustomData_get() is not the correct function to call here, since that returns
    // individual values from an "active" layer of given type. We want CustomData_get_layer_n().
    MLoopCol *vcolor_data = (MLoopCol *)CustomData_get_layer_n(
        &blender_mesh->ldata, CD_MLOOPCOL, k);
    if (NULL == vcolor_data) {
      printf("WARNING: missing color attribute!\n");
      continue;
    }

    if (ofx_no_loose_edge) {
      // reuse host buffer, kOfxMeshPropNoLooseEdge optimization
      MFX_CHECK(mes->attributeDefine(ofx_mesh,
                                     kOfxMeshAttribCorner,
                                     name,
                                     3,
                                     kOfxMeshAttribTypeUByte,
                                     kOfxMeshAttribSemanticColor,
                                     &vcolor_attrib));
      MFX_CHECK(ps->propSetInt(vcolor_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
      MFX_CHECK(
          ps->propSetPointer(vcolor_attrib, kOfxMeshAttribPropData, 0, (void *)&vcolor_data[0].r));
      MFX_CHECK(ps->propSetInt(vcolor_attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoopCol)));
    }
    else if (blender_loop_count > 0) {
      // request new buffer to copy data from existing polys, fill default values for edges
      MFX_CHECK(mes->attributeDefine(ofx_mesh,
                                     kOfxMeshAttribCorner,
                                     name,
                                     3,
                                     kOfxMeshAttribTypeUByte,
                                     kOfxMeshAttribSemanticColor,
                                     &vcolor_attrib));
      MFX_CHECK(ps->propSetInt(vcolor_attrib, kOfxMeshAttribPropIsOwner, 0, 1));
    }
    else {
      // we have just loose edges, no data to copy
      printf("WARNING: I want to copy corner colors but there are no corners\n");
    }
  }

  // Define corner UV attributes
  int uv_layers = CustomData_number_of_layers(&blender_mesh->ldata, CD_MLOOPUV);
  OfxPropertySetHandle uv_attrib;
  for (int k = 0; k < uv_layers; ++k) {
    sprintf(name, "uv%d", k);
    MLoopUV *uv_data = (MLoopUV *)CustomData_get_layer_n(&blender_mesh->ldata, CD_MLOOPUV, k);
    if (NULL == uv_data) {
      printf("WARNING: missing UV attribute!\n");
      continue;
    }

    if (ofx_no_loose_edge) {
      // reuse host buffer, kOfxMeshPropNoLooseEdge optimization
      MFX_CHECK(mes->attributeDefine(ofx_mesh,
                                     kOfxMeshAttribCorner,
                                     name,
                                     2,
                                     kOfxMeshAttribTypeFloat,
                                     kOfxMeshAttribSemanticTextureCoordinate,
                                     &uv_attrib));
      MFX_CHECK(ps->propSetInt(uv_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
      MFX_CHECK(ps->propSetPointer(uv_attrib, kOfxMeshAttribPropData, 0, (void *)&uv_data[0].uv[0]));
      MFX_CHECK(ps->propSetInt(uv_attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoopUV)));
    }
    else if (blender_loop_count > 0) {
      // request new buffer to copy data from existing polys, fill default values for edges
      MFX_CHECK(mes->attributeDefine(ofx_mesh,
                                     kOfxMeshAttribCorner,
                                     name,
                                     2,
                                     kOfxMeshAttribTypeFloat,
                                     kOfxMeshAttribSemanticTextureCoordinate,
                                     &uv_attrib));
      MFX_CHECK(ps->propSetInt(uv_attrib, kOfxMeshAttribPropIsOwner, 0, 1));
    }
    else {
      // we have just loose edges, no data to copy
      printf("WARNING: I want to copy UV but there are no corners\n");
    }
  }

  // Point position
  OfxPropertySetHandle pos_attrib;
  MFX_CHECK(mes->meshGetAttribute(
      ofx_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib));
  MFX_CHECK(ps->propSetInt(pos_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
  MFX_CHECK(ps->propSetPointer(
      pos_attrib, kOfxMeshAttribPropData, 0, (void *)&blender_mesh->mvert[0].co[0]));
  MFX_CHECK(ps->propSetInt(pos_attrib, kOfxMeshAttribPropStride, 0, sizeof(MVert)));

  // Corner point
  OfxPropertySetHandle cornerpoint_attrib;
  MFX_CHECK(mes->meshGetAttribute(
      ofx_mesh, kOfxMeshAttribCorner, kOfxMeshAttribCornerPoint, &cornerpoint_attrib));

  if (ofx_no_loose_edge) {
    // use host buffers, kOfxMeshPropNoLooseEdge optimization
    MFX_CHECK(ps->propSetInt(cornerpoint_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(ps->propSetPointer(
        cornerpoint_attrib, kOfxMeshAttribPropData, 0, (void *)&blender_mesh->mloop[0].v));
    MFX_CHECK(ps->propSetInt(cornerpoint_attrib, kOfxMeshAttribPropStride, 0, sizeof(MLoop)));
  }
  else {
    // request new buffer, we need to append new corners for loose edges
    MFX_CHECK(ps->propSetInt(cornerpoint_attrib, kOfxMeshAttribPropIsOwner, 0, 1));
  }

  // Face count
  OfxPropertySetHandle facesize_attrib;
  MFX_CHECK(mes->meshGetAttribute(
      ofx_mesh, kOfxMeshAttribFace, kOfxMeshAttribFaceSize, &facesize_attrib));

  if (-1 != ofx_constant_face_size) {
    // no buffer, kOfxMeshPropConstantFaceCount optimization
    MFX_CHECK(ps->propSetInt(facesize_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(ps->propSetPointer(facesize_attrib, kOfxMeshAttribPropData, 0, NULL));
    MFX_CHECK(ps->propSetInt(facesize_attrib, kOfxMeshAttribPropStride, 0, 0));
  }
  else if (ofx_no_loose_edge) {
    // use host buffers, kOfxMeshPropNoLooseEdge optimization
    MFX_CHECK(ps->propSetInt(facesize_attrib, kOfxMeshAttribPropIsOwner, 0, 0));
    MFX_CHECK(ps->propSetPointer(
        facesize_attrib, kOfxMeshAttribPropData, 0, (void *)&blender_mesh->mpoly[0].totloop));
    MFX_CHECK(ps->propSetInt(facesize_attrib, kOfxMeshAttribPropStride, 0, sizeof(MPoly)));
  }
  else {
    // request new buffer, we need to append new faces for loose edges
    MFX_CHECK(ps->propSetInt(facesize_attrib, kOfxMeshAttribPropIsOwner, 0, 1));
  }

  // finished adding attributes, allocate any requested buffers
  MFX_CHECK(mes->meshAlloc(ofx_mesh));

  // loose edge cleanup
  // There were loose edge, so we have to copy memory rather than pointing to existing buffers
  if (!ofx_no_loose_edge) {
    // check that meshAlloc() gave us contiguous buffers
    int stride;
    MFX_CHECK(ps->propGetInt(cornerpoint_attrib, kOfxMeshAttribPropStride, 0, &stride));
    assert(stride == sizeof(int));
    MFX_CHECK(ps->propGetInt(facesize_attrib, kOfxMeshAttribPropStride, 0, &stride));
    assert(stride == sizeof(int));

    // Corner point
    int i;
    int *ofx_corner_buffer;
    MFX_CHECK(ps->propGetPointer(
        cornerpoint_attrib, kOfxMeshAttribPropData, 0, (void **)&ofx_corner_buffer));
    for (i = 0; i < blender_mesh->totloop; ++i) {
      ofx_corner_buffer[i] = blender_mesh->mloop[i].v;
    }
    for (int j = 0; j < blender_mesh->totedge; ++j) {
      if (blender_mesh->medge[j].flag & ME_LOOSEEDGE) {
        ofx_corner_buffer[i] = blender_mesh->medge[j].v1;
        ofx_corner_buffer[i + 1] = blender_mesh->medge[j].v2;
        i += 2;
      }
    }

    // Face size
    if (-1 == ofx_constant_face_size) {
      int *ofx_face_buffer;
      MFX_CHECK(ps->propGetPointer(
          facesize_attrib, kOfxMeshAttribPropData, 0, (void **)&ofx_face_buffer));
      for (i = 0; i < blender_mesh->totpoly; ++i) {
        ofx_face_buffer[i] = blender_mesh->mpoly[i].totloop;
      }
      for (int j = 0; j < blender_loose_edge_count; ++j) {
        ofx_face_buffer[i] = 2;
        ++i;
      }
    }

    // Corner colors attributes
    for (int k = 0; k < vcolor_layers; ++k) {
      sprintf(name, "color%d", k);
      MLoopCol *vcolor_data = (MLoopCol *)CustomData_get_layer_n(
          &blender_mesh->ldata, CD_MLOOPCOL, k);

      if (NULL != vcolor_data && blender_loop_count > 0) {
        unsigned char *ofx_vcolor_buffer;
        MFX_CHECK(ps->propGetPointer(
            vcolor_attrib, kOfxMeshAttribPropData, 0, (void **)&ofx_vcolor_buffer));
        MFX_CHECK(ps->propGetInt(vcolor_attrib, kOfxMeshAttribPropStride, 0, &stride));
        assert(stride == 3 * sizeof(unsigned char));

        for (i = 0; i < ofx_corner_count; i++) {
          if (i < blender_loop_count) {
            ofx_vcolor_buffer[3 * i + 0] = vcolor_data[i].r;
            ofx_vcolor_buffer[3 * i + 1] = vcolor_data[i].g;
            ofx_vcolor_buffer[3 * i + 2] = vcolor_data[i].b;
          }
          else {
            ofx_vcolor_buffer[3 * i + 0] = 0;
            ofx_vcolor_buffer[3 * i + 1] = 0;
            ofx_vcolor_buffer[3 * i + 2] = 0;
          }
        }
      }
    }

    // Define corner UV attributes
    for (int k = 0; k < uv_layers; ++k) {
      sprintf(name, "uv%d", k);
      MLoopUV *uv_data = (MLoopUV *)CustomData_get_layer_n(&blender_mesh->ldata, CD_MLOOPUV, k);

      if (NULL != uv_data && blender_loop_count > 0) {
        float *ofx_uv_buffer;
        MFX_CHECK(
            ps->propGetPointer(uv_attrib, kOfxMeshAttribPropData, 0, (void **)&ofx_uv_buffer));
        MFX_CHECK(ps->propGetInt(uv_attrib, kOfxMeshAttribPropStride, 0, &stride));
        assert(stride == 2 * sizeof(float));

        for (i = 0; i < ofx_corner_count; i++) {
          if (i < blender_loop_count) {
            ofx_uv_buffer[2 * i] = uv_data[i].uv[0];
            ofx_uv_buffer[2 * i + 1] = uv_data[i].uv[1];
          }
          else {
            ofx_uv_buffer[2 * i] = 0;
            ofx_uv_buffer[2 * i + 1] = 0;
          }
        }
      }
    }
  }  // end loose edge cleanup

  return kOfxStatOK;
}

// ----------------------------------------------------------------------------

OfxStatus Converter::mfxToBlender(OfxMeshHandle ofx_mesh) const
{
  Mesh *source_mesh;
  Mesh *blender_mesh;
  int ofx_point_count, ofx_corner_count, ofx_face_count, ofx_no_loose_edge,
      ofx_constant_face_size;
  int blender_poly_count, loose_edge_count, blender_loop_count;
  int point_stride, corner_stride, face_stride;
  char *point_data, *corner_data, *face_data;
  OfxStatus status;
  MeshInternalData *internal_data;

  propFreeTransformMatrix(&ofx_mesh->properties);
  ps->propGetPointer(&ofx_mesh->properties, kOfxMeshPropInternalData, 0, (void **)&internal_data);

  if (NULL == internal_data) {
    printf("No internal data found\n");
    return kOfxStatErrBadHandle;
  }
  source_mesh = internal_data->source_mesh;

  if (true == internal_data->is_input) {
    printf("Input: NOT converting ofx mesh\n");
    return kOfxStatOK;
  }

  ps->propGetInt(&ofx_mesh->properties, kOfxMeshPropPointCount, 0, &ofx_point_count);
  ps->propGetInt(&ofx_mesh->properties, kOfxMeshPropCornerCount, 0, &ofx_corner_count);
  ps->propGetInt(&ofx_mesh->properties, kOfxMeshPropFaceCount, 0, &ofx_face_count);
  ps->propGetInt(&ofx_mesh->properties, kOfxMeshPropNoLooseEdge, 0, &ofx_no_loose_edge);
  ps->propGetInt(
      &ofx_mesh->properties, kOfxMeshPropConstantFaceSize, 0, &ofx_constant_face_size);

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
  mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribPoint, kOfxMeshAttribPointPosition, &pos_attrib);
  ps->propGetPointer(pos_attrib, kOfxMeshAttribPropData, 0, (void **)&point_data);
  ps->propGetInt(pos_attrib, kOfxMeshAttribPropStride, 0, &point_stride);
  mes->meshGetAttribute(
      ofx_mesh, kOfxMeshAttribCorner, kOfxMeshAttribCornerPoint, &cornerpoint_attrib);
  ps->propGetPointer(cornerpoint_attrib, kOfxMeshAttribPropData, 0, (void **)&corner_data);
  ps->propGetInt(cornerpoint_attrib, kOfxMeshAttribPropStride, 0, &corner_stride);
  mes->meshGetAttribute(
      ofx_mesh, kOfxMeshAttribFace, kOfxMeshAttribFaceSize, &facesize_attrib);
  ps->propGetPointer(facesize_attrib, kOfxMeshAttribPropData, 0, (void **)&face_data);
  ps->propGetInt(facesize_attrib, kOfxMeshAttribPropStride, 0, &face_stride);

  ps->propSetPointer(&ofx_mesh->properties, kOfxMeshPropInternalData, 0, NULL);

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
      assert(check_no_loose_edges_in_ofx_mesh(ofx_face_count, face_data, face_stride));
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
    blender_mesh = BKE_mesh_new_nomain_from_template(
        source_mesh, ofx_point_count, loose_edge_count, 0, blender_loop_count, blender_poly_count);
  }
  else {
    printf("Warning: No source mesh\n");
    blender_mesh = BKE_mesh_new_nomain(
        ofx_point_count, loose_edge_count, 0, ofx_corner_count, blender_poly_count);
  }
  if (NULL == blender_mesh) {
    printf("WARNING: Could not allocate Blender Mesh data\n");
    return kOfxStatErrMemory;
  }

  printf("Converting ofx mesh into blender mesh...\n");

  // copy OFX points (= Blender's vertex)
  for (int i = 0; i < ofx_point_count; ++i) {
    float *p = attributeAt<float>(point_data, point_stride, i);
    copy_v3_v3(blender_mesh->mvert[i].co, p);
  }

  // copy OFX corners (= Blender's loops) + OFX faces (= Blender's faces and edges)
  if (loose_edge_count == 0) {
    // Corners
    for (int i = 0; i < ofx_corner_count; ++i) {
      blender_mesh->mloop[i].v = *attributeAt<int>(corner_data, corner_stride, i);
    }

    // Faces
    int size = ofx_constant_face_size;
    int current_loop = 0;
    for (int i = 0; i < ofx_face_count; ++i) {
      if (-1 == ofx_constant_face_size) {
        size = *attributeAt<int>(face_data, face_stride, i);
      }
      blender_mesh->mpoly[i].loopstart = current_loop;
      blender_mesh->mpoly[i].totloop = size;
      current_loop += size;
    }
  }
  else {
    int size = ofx_constant_face_size;
    int current_poly = 0, current_edge = 0, current_corner_ofx = 0,
        current_loop_blender = 0;

    for (int i = 0; i < ofx_face_count; ++i) {
      if (-1 == ofx_constant_face_size) {
        size = *attributeAt<int>(face_data, face_stride, i);
      }
      if (2 == size) {
        // make Blender edge, no loops
        blender_mesh->medge[current_edge].v1 = corner_data[current_corner_ofx];
        blender_mesh->medge[current_edge].v2 = corner_data[current_corner_ofx + 1];
        blender_mesh->medge[current_edge].flag |= ME_LOOSEEDGE |
                                                  ME_EDGEDRAW;  // see BKE_mesh_calc_edges_loose()

        ++current_edge;
        current_corner_ofx += 2;
      }
      else {
        // make Blender poly and loops
        blender_mesh->mpoly[current_poly].loopstart = current_loop_blender;
        blender_mesh->mpoly[current_poly].totloop = size;

        for (int j = 0; j < size; ++j) {
          blender_mesh->mloop[current_loop_blender + j].v = corner_data[current_corner_ofx + j];
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
  char name[MAX_CORNER_ATTRIB_NAME];
  char *ofx_uv_data;
  int ofx_uv_stride;
  for (int k = 0; k < uv_layers; ++k) {
    OfxPropertySetHandle uv_attrib;
    sprintf(name, "uv%d", k);
    printf("Look for attribute '%s'\n", name);
    status = mes->meshGetAttribute(ofx_mesh, kOfxMeshAttribCorner, name, &uv_attrib);
    if (kOfxStatOK == status) {
      printf("Found!\n");
      ps->propGetPointer(uv_attrib, kOfxMeshAttribPropData, 0, (void **)&ofx_uv_data);
      ps->propGetInt(uv_attrib, kOfxMeshAttribPropStride, 0, &ofx_uv_stride);

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
      // MLoopUV *uv_data = (MLoopUV*)CustomData_add_layer_named(&blender_mesh->ldata, CD_MLOOPUV,
      // CD_CALLOC, NULL, ofx_corner_count, name);
      char uvname[MAX_CUSTOMDATA_LAYER_NAME];
      CustomData_validate_layer_name(&blender_mesh->ldata, CD_MLOOPUV, name, uvname);
      MLoopUV *uv_data = (MLoopUV *)CustomData_duplicate_referenced_layer_named(
          &blender_mesh->ldata, CD_MLOOPUV, uvname, ofx_corner_count);

      for (int i = 0; i < ofx_corner_count; ++i) {
        float *uv = attributeAt<float>(ofx_uv_data, ofx_uv_stride, i);
        uv_data[i].uv[0] = uv[0];
        uv_data[i].uv[1] = uv[1];
      }
      blender_mesh->runtime.cd_dirty_loop |= CD_MASK_MLOOPUV;
      blender_mesh->runtime.cd_dirty_poly |= CD_MASK_MTFACE;
    }
  }

  if (blender_poly_count > 0) {
    // if we're here, this dominates before_mesh_get()/before_mesh_release() total running time!
    BKE_mesh_calc_edges(blender_mesh, (loose_edge_count > 0), false);
  }

  internal_data->blender_mesh = blender_mesh;

  return kOfxStatOK;
}

// ----------------------------------------------------------------------------

bool Converter::check_no_loose_edges_in_ofx_mesh(int face_count,
                                                 const int *face_data,
                                                 int face_stride)
{
  for (int i = 0; i < face_count; ++i) {
    int corner_count = *(int *)((char *)face_data + i * face_stride);
    if (2 == corner_count) {
      return false;
    }
  }
  return true;
}

// ----------------------------------------------------------------------------

void Converter::propSetTransformMatrix(OfxPropertySetHandle properties, const Object *object) const
{
  assert(NULL != object);
  double *matrix = new double[16];

// #pragma omp parallel for
  for (int i = 0; i < 16; ++i) {
    // convert to OpenMeshEffect's row-major order from Blender's column-major
    matrix[i] = static_cast<double>(object->obmat[i % 4][i / 4]);
  }

  MFX_CHECK(ps->propSetPointer(properties, kOfxMeshPropTransformMatrix, 0, (void *)matrix));
}

void Converter::propFreeTransformMatrix(OfxPropertySetHandle properties) const
{
  double *matrix = NULL;
  MFX_CHECK(ps->propGetPointer(properties, kOfxMeshPropTransformMatrix, 0, (void **)&matrix));
  MFX_CHECK(ps->propSetPointer(properties, kOfxMeshPropTransformMatrix, 0, NULL));
  assert(NULL != matrix);
  delete[] matrix;
}

void Converter::countMeshElements(Mesh * blender_mesh,
                                 int & ofx_point_count,
                                 int & ofx_corner_count,
                                 int & ofx_face_count,
                                 int & ofx_no_loose_edge,
                                 int & ofx_constant_face_size,
                                 int & blender_loop_count,
                                 int & blender_loose_edge_count)
{
  int blender_poly_count, blender_point_count;

  // count input geometry on blender side
  blender_point_count = blender_mesh->totvert;
  blender_loop_count = 0;
  for (int i = 0; i < blender_mesh->totpoly; ++i) {
    int after_last_loop = blender_mesh->mpoly[i].loopstart + blender_mesh->mpoly[i].totloop;
    blender_loop_count = max(blender_loop_count, after_last_loop);
  }
  blender_poly_count = blender_mesh->totpoly;
  blender_loose_edge_count = 0;
  for (int i = 0; i < blender_mesh->totedge; ++i) {
    if (blender_mesh->medge[i].flag & ME_LOOSEEDGE)
      ++blender_loose_edge_count;
  }

  // figure out input geometry size on OFX side
  ofx_point_count = blender_point_count;
  ofx_corner_count = blender_loop_count;
  ofx_face_count = blender_poly_count;
  ofx_no_loose_edge = (blender_loose_edge_count > 0) ? 0 : 1;
  ofx_constant_face_size = (blender_loose_edge_count > 0 && blender_poly_count == 0) ? 2 : -1;

  if (blender_loose_edge_count > 0) {
    // turn blender loose edges into 2-corner faces
    ofx_corner_count += 2 * blender_loose_edge_count;
    ofx_face_count += blender_loose_edge_count;
    printf("Blender mesh has %d loose edges\n", blender_loose_edge_count);
  }
}

// ----------------------------------------------------------------------------
// Public callbacks
// ----------------------------------------------------------------------------

OfxStatus before_mesh_get(OfxHost *host, OfxMeshHandle ofx_mesh) {
  Converter converter(host);
  return converter.blenderToMfx(ofx_mesh);
}

OfxStatus before_mesh_release(OfxHost *host, OfxMeshHandle ofx_mesh) {
  Converter converter(host);
  return converter.mfxToBlender(ofx_mesh);
}

