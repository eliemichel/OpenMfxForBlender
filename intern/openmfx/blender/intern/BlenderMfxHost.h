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

#include <functional>
#include <vector>

struct MfxAttributeProps;
struct Mesh;
struct Object;
struct MLoopCol;
struct MLoopUV;
struct MIntProperty;

using CallbackList = std::vector<std::function<void()>>;

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
  struct ElementCounts {
    int ofxPointCount = 0;
    int ofxCornerCount = 0;
    int ofxFaceCount = 0;
    int ofxNoLooseEdge = 1;
    int ofxConstantFaceSize = -1;
    int blenderLoopCount = 0;
    int blenderLooseEdgeCount = 0;
    int blenderPolygonCount = 0;
  };

  static bool hasNoLooseEdge(int face_count, const MfxAttributeProps &faceSize);

  /**
   * Copy the model to world matrix from the blender object to target mesh properties.
   * This allocate new data that must be eventually freed using propFreeTransformMatrix()
   * @param properties Mesh properties
   */
  void propSetTransformMatrix(OfxPropertySetHandle properties, const Object *object) const;

  /**
   * Free data that had been allocated for transform matrix
   * @param properties Mesh properties
   */
  void propFreeTransformMatrix(OfxPropertySetHandle properties) const;

  /**
   * Count the number of point/corner/face in Blender mesh, as well as loose edge
   * and constant face size flags.
   * (it's static because it does not use the suites)
   */
  static void countMeshElements(const Mesh *blenderMesh, ElementCounts &counts);

  /**
   * Get the number of point/corner/face from an ofx mesh, as well as loose edge
   * and constant face size flags.
   */
  OfxStatus countMeshElements(OfxMeshHandle ofxMesh, ElementCounts &counts) const;

  /**
   * Given the connectivity attributes, compute the numbers of verts/loops/polys
   * for the new Blender mesh
   */
  OfxStatus computeBlenderMeshElementsCounts(const MfxAttributeProps &faceSize,
                                             ElementCounts &counts) const;

  /**
   * Initialize mesh properties for an empty mesh
   * @param properties Mesh properties
   */
  OfxStatus setupElementCounts(OfxPropertySetHandle properties, const ElementCounts &counts) const;

  /**
   * Set the data pointer and stride for point position attribute
   */
  OfxStatus setupPointPositionAttribute(OfxMeshHandle ofxMesh, const Mesh *blenderMesh) const;

  /**
   * Set the data pointer and stride for corner point attribute
   */
  OfxStatus setupCornerPointAttribute(OfxMeshHandle ofxMesh,
                                      const Mesh *blenderMesh,
                                      const ElementCounts &counts,
                                      CallbackList &afterAllocate) const;

  /**
   * Set the data pointer and stride for face size attribute
   */
  OfxStatus setupFaceSizeAttribute(OfxMeshHandle ofxMesh,
                                   const Mesh *blenderMesh,
                                   const ElementCounts &counts,
                                   CallbackList &afterAllocate) const;

  /**
   * Set the data pointer and stride for all point weight attributes
   * @param blenderData must not be null
   */
  OfxStatus setupPointWeightAttributes(OfxMeshHandle ofxMesh,
                                       const char *prefix,
                                       const Mesh *blenderMesh,
                                       const ElementCounts &counts,
                                       CallbackList &afterAllocate) const;

  /**
   * Set the data pointer and stride for all corner color attributes
   * @param blenderData must not be null
   */
  OfxStatus setupCornerColorAttributes(OfxMeshHandle ofxMesh,
                                       const char *prefix,
                                       const Mesh *blenderMesh,
                                       const ElementCounts &counts,
                                       CallbackList &afterAllocate) const;

  /**
   * Set the data pointer and stride for all corner UV attributes
   * @param blenderData must not be null
   */
  OfxStatus setupCornerUvAttributes(OfxMeshHandle ofxMesh,
                                    const char *prefix,
                                    const Mesh *blenderMesh,
                                    const ElementCounts &counts,
                                    CallbackList &afterAllocate) const;

  /**
   * Set the data pointer and stride for all face map attributes
   * @param blenderData must not be null
   */
  OfxStatus setupFaceMapAttributes(OfxMeshHandle ofxMesh,
                                   const char *prefix,
                                   const Mesh *blenderMesh,
                                   const ElementCounts &counts,
                                   CallbackList &afterAllocate) const;

  /**
   * Setup an ofx corner attribute from a blender loop attribute.
   * If the is no loose edge, directly point to blender buffers,
   * otherwise copy to a new buffer and set the value to 0 for loose edges.
   */
  OfxStatus setupCornerAttribute(OfxMeshHandle ofxMesh,
                                 const char *name,
                                 int componentCount,
                                 const char *componentType,
                                 const char *semantic,
                                 char *blenderLoopData,
                                 size_t blenderLoopStride,
                                 const ElementCounts &counts,
                                 CallbackList &afterAllocate) const;

  /**
   * Set the data pointer and stride for a face map attribute
   * @param blenderData must not be null
   */
  OfxStatus setupFaceMapAttribute(OfxMeshHandle ofxMesh,
                                  const char *name,
                                  const MIntProperty *blenderData,
                                  const ElementCounts &counts,
                                  CallbackList &afterAllocate) const;

  /**
   * Extract from ofx mesh the basic attributes (point position, corner point, face size)
   */
  OfxStatus extractBasicAttributes(const MfxAttributeProps &pointPosition,
                                   const MfxAttributeProps &cornerPoint,
                                   const MfxAttributeProps &faceSize,
                                   Mesh *blenderMesh,
                                   const ElementCounts &counts) const;

  /**
   * Extract all UV attributes from ofx mesh to blender mesh
   */
  OfxStatus extractUvAttributes(OfxMeshHandle ofxMesh,
                                Mesh *blenderMesh,
                                const ElementCounts &counts) const;

 private:
  // Avoid calling before allocate callback from within the before mesh get one
  bool m_deactivateBeforeAllocateCb = false;
};
