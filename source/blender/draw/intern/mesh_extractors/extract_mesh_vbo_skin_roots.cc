/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 *
 * The Original Code is Copyright (C) 2021 by Blender Foundation.
 * All rights reserved.
 */

/** \file
 * \ingroup draw
 */

#include "extract_mesh.h"

namespace blender::draw {

/* ---------------------------------------------------------------------- */
/** \name Extract Skin Modifier Roots
 * \{ */

struct SkinRootData {
  float size;
  float local_pos[3];
};

static void extract_skin_roots_init(const MeshRenderData *mr,
                                    struct MeshBatchCache *UNUSED(cache),
                                    void *buf,
                                    void *UNUSED(tls_data))
{
  GPUVertBuf *vbo = static_cast<GPUVertBuf *>(buf);
  /* Exclusively for edit mode. */
  BLI_assert(mr->bm);

  static GPUVertFormat format = {0};
  if (format.attr_len == 0) {
    GPU_vertformat_attr_add(&format, "size", GPU_COMP_F32, 1, GPU_FETCH_FLOAT);
    GPU_vertformat_attr_add(&format, "local_pos", GPU_COMP_F32, 3, GPU_FETCH_FLOAT);
  }

  GPU_vertbuf_init_with_format(vbo, &format);
  GPU_vertbuf_data_alloc(vbo, mr->bm->totvert);

  SkinRootData *vbo_data = (SkinRootData *)GPU_vertbuf_get_data(vbo);

  int root_len = 0;
  int cd_ofs = CustomData_get_offset(&mr->bm->vdata, CD_MVERT_SKIN);

  BMIter iter;
  BMVert *eve;
  BM_ITER_MESH (eve, &iter, mr->bm, BM_VERTS_OF_MESH) {
    const MVertSkin *vs = (const MVertSkin *)BM_ELEM_CD_GET_VOID_P(eve, cd_ofs);
    if (vs->flag & MVERT_SKIN_ROOT) {
      vbo_data->size = (vs->radius[0] + vs->radius[1]) * 0.5f;
      copy_v3_v3(vbo_data->local_pos, bm_vert_co_get(mr, eve));
      vbo_data++;
      root_len++;
    }
  }

  /* It's really unlikely that all verts will be roots. Resize to avoid losing VRAM. */
  GPU_vertbuf_data_len_set(vbo, root_len);
}

constexpr MeshExtract create_extractor_skin_roots()
{
  MeshExtract extractor = {nullptr};
  extractor.init = extract_skin_roots_init;
  extractor.data_type = MR_DATA_NONE;
  extractor.data_size = 0;
  extractor.use_threading = false;
  extractor.mesh_buffer_offset = offsetof(MeshBufferList, vbo.skin_roots);
  return extractor;
}

/** \} */

}  // namespace blender::draw

extern "C" {
const MeshExtract extract_skin_roots = blender::draw::create_extractor_skin_roots();
}
