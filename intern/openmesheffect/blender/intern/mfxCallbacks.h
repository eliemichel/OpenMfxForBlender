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
 * This file contains callbacks to provide to the core mfxHost, that is agnostic of Blender
 * structures (and must remain, because not GPL).
 */

#include "ofxCore.h"
#include "ofxMeshEffect.h"

#include "DNA_mesh_types.h"
#include "DNA_object_types.h"

/**
 * Data shared as a blind handle from Blender GPL code to host code
 */
typedef struct MeshInternalData {
  // Data is used either for an input or for an output
  bool is_input;
  // For an input mesh, only blender_mesh is used
  // For an output mesh, blender_mesh is set to NULL and source_mesh is set to the source mesh
  // from which copying some flags and stuff.
  Mesh *blender_mesh;
  Mesh *source_mesh;
  Object *object;
} MeshInternalData;

/**
 * Convert blender mesh from internal pointer into ofx mesh.
 * /pre no ofx mesh has been allocated or internal pointer is null
 */
OfxStatus before_mesh_get(OfxHost *host, OfxMeshHandle ofx_mesh);

/**
 * Convert ofx mesh into blender mesh and store it in internal pointer
 */
OfxStatus before_mesh_release(OfxHost *host, OfxMeshHandle ofx_mesh);
