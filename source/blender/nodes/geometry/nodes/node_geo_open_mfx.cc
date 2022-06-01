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
 */

#include "BLI_disjoint_set.hh"
#include "BLI_task.hh"
#include "BLI_vector_set.hh"

#include "DNA_mesh_types.h"
#include "DNA_meshdata_types.h"

#include "BKE_attribute_math.hh"
#include "BKE_mesh.h"
#include "BKE_mesh_runtime.h"

#include "UI_interface.h"
#include "UI_resources.h"

#include "NOD_socket.h"

#include "MFX_node_runtime.h"

#include "node_geometry_util.hh"

namespace blender::nodes::node_geo_open_mfx_cc {

NODE_STORAGE_FUNCS(NodeGeometryOpenMfx)

static void node_declare(NodeDeclarationBuilder &b)
{
  const bNode *node = b.node();
  if (node == nullptr || node->storage == nullptr)
    return;
  const NodeGeometryOpenMfx &storage = node_storage(*node);
  printf("TEST: loaded_effect_index = %d\n", storage.runtime->loaded_effect_index);
  if (BLI_str_endswith(storage.plugin_path, ".ofx")) {
    b.add_input<decl::Float>(N_("Radius"))
        .default_value(1.0f)
        .min(0.0f)
        .subtype(PROP_DISTANCE)
        .description(N_("Size of the pizza"));
    b.add_output<decl::Geometry>("Mesh");
    b.add_output<decl::Bool>(N_("Base")).field_source();
    b.add_output<decl::Bool>(N_("Olives")).field_source();
  }
  else {
    b.add_input<decl::Float>(N_("Nothing"));
  }
}

static void node_layout(uiLayout *layout, bContext *UNUSED(C), PointerRNA *ptr)
{
  uiLayoutSetPropSep(layout, true);
  uiLayoutSetPropDecorate(layout, false);
  uiItemR(layout, ptr, "plugin_path", 0, "", ICON_NONE);
  uiItemR(layout, ptr, "effect_index", 0, "", ICON_NONE);
  uiItemR(layout, ptr, "olive_count", 0, "", ICON_NONE);
}

static void node_init(bNodeTree *UNUSED(tree), bNode *node)
{
  NodeGeometryOpenMfx *data = MEM_cnew<NodeGeometryOpenMfx>(__func__);
  data->olive_count = 5;
  data->runtime = MEM_new<RuntimeData>(__func__);
  data->runtime->loaded_effect_index = 42;
  node->storage = data;
}

/* We use custom free and copy function because of manually allocated runtime data */
static void node_free_storage(struct bNode *node)
{
  NodeGeometryOpenMfx &storage = node_storage(*node);
  if (storage.runtime != nullptr) {
    MEM_delete<RuntimeData>(storage.runtime);
    storage.runtime = nullptr;
  }
  node_free_standard_storage(node);
}

static void node_copy_storage(struct bNodeTree *dest_ntree,
                       struct bNode *dest_node,
                       const struct bNode *src_node)
{
  node_copy_standard_storage(dest_ntree, dest_node, src_node);
  NodeGeometryOpenMfx &dest_storage = node_storage(*dest_node);
  const NodeGeometryOpenMfx &src_storage = node_storage(*src_node);
  dest_storage.runtime = MEM_new<RuntimeData>(__func__);
  *dest_storage.runtime = *src_storage.runtime;
}

static void force_redeclare(bNodeTree *ntree, bNode *node)
{
  BLI_assert(node->typeinfo->declaration_is_dynamic);
  if (node->declaration != nullptr)
  {
    delete node->declaration;
    node->declaration = nullptr;
  }
  node_verify_sockets(ntree, node, true);
}

static void node_update(bNodeTree *ntree, bNode *node)
{
  if (node == nullptr || node->storage == nullptr)
    return;
  const NodeGeometryOpenMfx &storage = node_storage(*node);

  bool changed = false;
  changed = storage.runtime->setPluginPath(storage.plugin_path) || changed;
  changed = storage.runtime->setEffectIndex(storage.effect_index) || changed;

  if (changed) {
    force_redeclare(ntree, node);
  }
  
  if (node->outputs.first == nullptr)
    return;

  bNodeSocket *out_socket_geometry = (bNodeSocket *)node->outputs.first;
  bNodeSocket *out_socket_base = out_socket_geometry->next;
  bNodeSocket *out_socket_olives = out_socket_base->next;

  // Stupid feature for the sake of the example: When there are too many
  // olives, we no longer output the fields!
  nodeSetSocketAvailability(ntree, out_socket_base, storage.olive_count < 25);
  nodeSetSocketAvailability(ntree, out_socket_olives, storage.olive_count < 25);
}

static Mesh *create_pizza_mesh(const int olive_count,
                               const float radius,
                               const int base_div,
                               IndexRange &base_polys,
                               IndexRange &olives_polys)
{
  // (i) compute element counts
  int vert_count = base_div + olive_count * 4;
  int edge_count = base_div + olive_count * 4;
  int corner_count = base_div + olive_count * 4;
  int face_count = 1 + olive_count;

  // (ii) allocate memory
  Mesh *mesh = BKE_mesh_new_nomain(vert_count, edge_count, 0, corner_count, face_count);

  // (iii) fill in element buffers
  MutableSpan<MVert> verts{mesh->mvert, mesh->totvert};
  MutableSpan<MLoop> loops{mesh->mloop, mesh->totloop};
  MutableSpan<MEdge> edges{mesh->medge, mesh->totedge};
  MutableSpan<MPoly> polys{mesh->mpoly, mesh->totpoly};
  base_polys = IndexRange{0, 1};
  olives_polys = IndexRange{1, olive_count};

  // Base
  const float angle_delta = 2.0f * (M_PI / static_cast<float>(base_div));
  for (const int i : IndexRange(base_div)) {
    // Vertex coordinates
    const float angle = i * angle_delta;
    copy_v3_v3(verts[i].co, float3(std::cos(angle) * radius, std::sin(angle) * radius, 0.0f));

    // Edge
    MEdge &edge = edges[i];
    edge.v1 = i;
    edge.v2 = (i + 1) % base_div;
    edge.flag = ME_EDGEDRAW | ME_EDGERENDER;

    // Corner
    MLoop &loop = loops[i];
    loop.e = i;
    loop.v = i;
  }
  // Face
  MPoly &poly = polys[0];
  poly.loopstart = 0;
  poly.totloop = base_div;

  // Olives
  const float angle_delta_olive = 2.0f * (M_PI / static_cast<float>(olive_count - 1));
  for (const int i : IndexRange(olive_count)) {
    const int offset = base_div + 4 * i;

    // Vertex coordinates
    float cx = 0, cy = 0;
    if (i > 0) { // (the olive #0 is at the center)
      const float angle = (i - 1) * angle_delta_olive;
      cx = std::cos(angle) * radius / 2;
      cy = std::sin(angle) * radius / 2;
    }
    copy_v3_v3(verts[offset + 0].co, float3(cx + 0.05f, cy + 0.05f, 0.01f));
    copy_v3_v3(verts[offset + 1].co, float3(cx - 0.05f, cy + 0.05f, 0.01f));
    copy_v3_v3(verts[offset + 2].co, float3(cx - 0.05f, cy - 0.05f, 0.01f));
    copy_v3_v3(verts[offset + 3].co, float3(cx + 0.05f, cy - 0.05f, 0.01f));

    for (const int k : IndexRange(4)) {
      // Edge
      MEdge &edge = edges[offset + k];
      edge.v1 = offset + k;
      edge.v2 = offset + (k + 1) % 4;
      edge.flag = ME_EDGEDRAW | ME_EDGERENDER;

      // Corner
      MLoop &loop = loops[offset + k];
      loop.e = offset + k;
      loop.v = offset + k;
    }

    // Face
    MPoly &poly = polys[1 + i];
    poly.loopstart = offset;
    poly.totloop = 4;
  }

  BLI_assert(BKE_mesh_is_valid(mesh));
  return mesh;
}

static void set_bool_face_field_output(GeoNodeExecParams &params, const char* attribute_name, const IndexRange &poly_range, Mesh *mesh)
{
  MeshComponent component;
  component.replace(mesh, GeometryOwnershipType::Editable);

  StrongAnonymousAttributeID id(attribute_name);
  OutputAttribute_Typed<bool> attribute = component.attribute_try_get_for_output_only<bool>(
      id.get(), ATTR_DOMAIN_FACE);
  attribute.as_span().slice(poly_range).fill(true);
  attribute.save();

  params.set_output(
      attribute_name,
      AnonymousAttributeFieldInput::Create<bool>(std::move(id), params.attribute_producer_name()));
}

static void node_geo_exec(GeoNodeExecParams params)
{
  if (params.node().outputs.first == nullptr)
    return;
  // We first retrieve the property (olive count) and the input socket (radius)
  const NodeGeometryOpenMfx &storage = node_storage(params.node());
  const int olive_count = storage.olive_count;
  const float radius = params.extract_input<float>("Radius");

  // Then we create the mesh (let's put it in a separate function)
  IndexRange base_polys, olives_polys;
  Mesh *mesh = create_pizza_mesh(olive_count, radius, 32, base_polys, olives_polys);

  if (params.output_is_required("Base")) {
    set_bool_face_field_output(params, "Base", base_polys, mesh);
  }

  if (params.output_is_required("Olives")) {
    set_bool_face_field_output(params, "Olives", base_polys, mesh);
  }

  // We build a geometry set to wrap the mesh and set it as the output value
  params.set_output("Mesh", GeometrySet::create_with_mesh(mesh));
}

}  // namespace blender::nodes::node_geo_open_mfx_cc

// ----------------------------------------------------------------------------
// Registration (main entry point)

void register_node_type_geo_open_mfx()
{
  namespace file_ns = blender::nodes::node_geo_open_mfx_cc;

  static bNodeType ntype;
  geo_node_type_base(&ntype, GEO_NODE_OPEN_MFX, "OpenMfx Plugin", NODE_CLASS_GEOMETRY);
  ntype.declare = file_ns::node_declare;
  ntype.declaration_is_dynamic = true;
  node_type_init(&ntype, file_ns::node_init);
  node_type_update(&ntype, file_ns::node_update);
  ntype.geometry_node_execute = file_ns::node_geo_exec;
  node_type_storage(&ntype,
                    "NodeGeometryOpenMfx",
                    file_ns::node_free_storage,
                    file_ns::node_copy_storage);
  ntype.draw_buttons = file_ns::node_layout;
  nodeRegisterType(&ntype);
}
