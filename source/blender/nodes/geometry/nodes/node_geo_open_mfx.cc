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

#include "node_geometry_util.hh"

namespace blender::nodes::node_geo_open_mfx_cc {

NODE_STORAGE_FUNCS(NodeGeometryOpenMfx)

static void node_declare(NodeDeclarationBuilder &b)
{
  b.add_input<decl::Float>(N_("Radius"))
      .default_value(1.0f)
      .min(0.0f)
      .subtype(PROP_DISTANCE)
      .description(N_("Size of the pizza"));
  b.add_output<decl::Geometry>("Mesh");
  b.add_output<decl::Bool>(N_("Base")).field_source();
  b.add_output<decl::Bool>(N_("Olives")).field_source();
}

static void node_layout(uiLayout *layout, bContext *UNUSED(C), PointerRNA *ptr)
{
  uiLayoutSetPropSep(layout, true);
  uiLayoutSetPropDecorate(layout, false);
  uiItemR(layout, ptr, "olive_count", 0, "", ICON_NONE);
}

static void node_init(bNodeTree *UNUSED(tree), bNode *node)
{
  NodeGeometryOpenMfx *data = MEM_cnew<NodeGeometryOpenMfx>(__func__);
  data->olive_count = 5;
  node->storage = data;
}

static void node_update(bNodeTree *ntree, bNode *node)
{
  const NodeGeometryOpenMfx &storage = node_storage(*node);

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
                               IndexRange &base_polys,
                               IndexRange &olives_polys)
{
  // (i) compute element counts
  int vert_count = 32 + olive_count * 4;
  int edge_count = 32 + olive_count * 4;
  int corner_count = 32 + olive_count * 4;
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
  const float angle_delta = 2 * M_PI / 32;
  for (const int i : IndexRange(32)) {
    // Vertex coordinates
    const float angle = i * angle_delta;
    copy_v3_v3(verts[i].co, float3(std::cos(angle) * radius, std::sin(angle) * radius, 0.0f));

    // Edge
    MEdge &edge = edges[i];
    edge.v1 = i;
    edge.v2 = (i + 1) % 32;
    edge.flag = ME_EDGEDRAW | ME_EDGERENDER;

    // Corner
    MLoop &loop = loops[i];
    loop.e = i;
    loop.v = i;
  }
  // Face
  MPoly &poly = polys[0];
  poly.loopstart = 0;
  poly.totloop = 32;

  // Olives
  const float angle_delta_olive = 2.0f * (M_PI / static_cast<float>(olive_count - 1));
  for (const int i : IndexRange(olive_count)) {
    const int offset = 32 + 4 * i;

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

static void node_geo_exec(GeoNodeExecParams params)
{
  // We first retrieve the property (olive count) and the input socket (radius)
  const NodeGeometryOpenMfx &storage = node_storage(params.node());
  const int olive_count = storage.olive_count;
  const float radius = params.extract_input<float>("Radius");

  // Then we create the mesh (let's put it in a separate function)
  IndexRange base_polys, olives_polys;
  Mesh *mesh = create_pizza_mesh(olive_count, radius, base_polys, olives_polys);

  if (params.output_is_required("Base")) {
    MeshComponent component;
    component.replace(mesh, GeometryOwnershipType::Editable);

    StrongAnonymousAttributeID id("Base");
    OutputAttribute_Typed<bool> attribute = component.attribute_try_get_for_output_only<bool>(
        id.get(), ATTR_DOMAIN_FACE);
    attribute.as_span().slice(base_polys).fill(true);
    attribute.save();
    
    params.set_output("Base",
                      AnonymousAttributeFieldInput::Create<bool>(
                          std::move(id), params.attribute_producer_name()));
  }

  if (params.output_is_required("Olives")) {
    MeshComponent component;
    component.replace(mesh, GeometryOwnershipType::Editable);

    StrongAnonymousAttributeID id("Olives");
    OutputAttribute_Typed<bool> attribute = component.attribute_try_get_for_output_only<bool>(
        id.get(), ATTR_DOMAIN_FACE);
    attribute.as_span().slice(olives_polys).fill(true);
    attribute.save();

    params.set_output("Olives",
                      AnonymousAttributeFieldInput::Create<bool>(
                          std::move(id), params.attribute_producer_name()));
  }

  // We build a geometry set to wrap the mesh and set it as the output value
  params.set_output("Mesh", GeometrySet::create_with_mesh(mesh));
}

}  // namespace blender::nodes::node_geo_open_mfx_cc

void register_node_type_geo_open_mfx()
{
  namespace file_ns = blender::nodes::node_geo_open_mfx_cc;

  static bNodeType ntype;
  geo_node_type_base(&ntype, GEO_NODE_OPEN_MFX, "Pizza", NODE_CLASS_GEOMETRY);
  ntype.declare = file_ns::node_declare;
  node_type_init(&ntype, file_ns::node_init);
  node_type_update(&ntype, file_ns::node_update);
  ntype.geometry_node_execute = file_ns::node_geo_exec;
  node_type_storage(
      &ntype, "NodeGeometryOpenMfx", node_free_standard_storage, node_copy_standard_storage);
  ntype.draw_buttons = file_ns::node_layout;
  nodeRegisterType(&ntype);
}
