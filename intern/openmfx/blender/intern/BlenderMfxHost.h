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

#pragma once

#include "BKE_geometry_set.hh"

#include <mfxHost/MfxHost>

struct Mesh;
struct Object;

class BlenderMfxHost : public OpenMfx::MfxHost {
 public:
  static BlenderMfxHost &GetInstance();

 public:
  enum class CallbackContext {
    Modifier,
    Node,
  };

  /**
   * Data shared as a blind handle from Blender GPL code to host code
   */
  struct MeshInternalData {
    // Data is used either for an input or for an output
    bool is_input;
    // Type of internal data, because callbacks are different for
    // the OpenMfx Modifier and the OpenMfx Geometry Node.
    CallbackContext type;
  };

  struct MeshInternalDataModifier {
    MeshInternalData header;

    // For an input mesh, only blender_mesh is used
    // For an output mesh, blender_mesh is set to NULL and source_mesh is set to the source mesh
    // from which copying some flags and stuff.
    Mesh *blender_mesh;
    Mesh *source_mesh;
    Object *object;
  };

  struct MeshInternalDataNode {
    MeshInternalData header;
    GeometrySet geo;
  };

 protected:
  /**
   * @brief Convert Blender mesh to Open Mesh Effect mesh if necessary
   *
   * This function prepares mesh for the effect, reusing Blender buffers as much as possible.
   * Due to different representation of edges in Blender and Open Mesh Effect, there are two
   * main code paths:
   *
   * 1) Input mesh has no loose edges. This is the fast path, which allows us to reuse all
   *    Blender buffers. Typical polygonal meshes and point clouds take this path.
   * 2) Input mesh has some loose edges. For Open Mesh Effect, we convert these into 2-corner
   * faces, which means that we no longer have the same number of corners/faces as in Blender mesh,
   *    and we have to copy all buffers except point coordinates.
   *
   * For 2), the 2-corner faces are added to the end, after any proper faces. There is an
   * optimization for the case when there are no proper faces, just loose edges (ie. edge
   * wireframe) - in this case, we use kOfxMeshPropConstantFaceCount instead of face count buffer.
   *
   * This function will also convert any corner color and UV attributes.
   * Each callback has a different version depending of the context (modifier or node)
   */
  OfxStatus BeforeMeshGet(OfxMeshHandle ofxMesh) override;

 private:
  OfxStatus BeforeMeshGetModifier(OfxMeshHandle ofxMesh,
                                  MeshInternalDataModifier &internalData);
  OfxStatus BeforeMeshGetNode(OfxMeshHandle ofxMesh,
                              MeshInternalDataNode &internalData);

 protected:
  /**
   * @brief Convert Open Mesh Effect mesh to Blender mesh
   *
   * This function receives output mesh from the effect, converting it into new Blender mesh.
   * We have to filter out any 2-corner faces and turn them into Blender loose edges.
   *
   * This function will also convert UV attributes called uv0, uv1, uv2, uv3.
   */
  OfxStatus BeforeMeshRelease(OfxMeshHandle ofxMesh) override;

 private:
  OfxStatus BeforeMeshReleaseModifier(OfxMeshHandle ofxMesh,
                                      MeshInternalDataModifier &internalData);
  OfxStatus BeforeMeshReleaseNode(OfxMeshHandle ofxMesh,
                                  MeshInternalDataNode &internalData);

 protected:
  /**
   * @brief Convert Open Mesh Effect mesh to Blender mesh
   *
   * This function is called before allocating owned attributes, it can be used
   * to allocate differently (in which case turn the owned flag to false).
   */
  OfxStatus BeforeMeshAllocate(OfxMeshHandle ofxMesh) override;

 private:
  OfxStatus BeforeMeshAllocateModifier(OfxMeshHandle ofxMesh,
                                       MeshInternalDataModifier &internalData);
  OfxStatus BeforeMeshAllocateNode(OfxMeshHandle ofxMesh,
                                   MeshInternalDataNode &internalData);

 private:
  static bool hasNoLooseEdge(int face_count, const char *face_data, int face_stride);

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
  static void countMeshElements(Mesh *blender_mesh,
                                int &ofx_point_count,
                                int &ofx_corner_count,
                                int &ofx_face_count,
                                int &ofx_no_loose_edge,
                                int &ofx_constant_face_size,
                                int &blender_loop_count,
                                int &blender_loose_edge_count);
};
