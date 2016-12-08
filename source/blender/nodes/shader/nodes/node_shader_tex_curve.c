/*
 * ***** BEGIN GPL LICENSE BLOCK *****
 *
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
 * The Original Code is Copyright (C) 2005 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): none yet.
 *
 * ***** END GPL LICENSE BLOCK *****
 */

#include "../node_shader_util.h"

/* **************** OUTPUT ******************** */

static bNodeSocketTemplate sh_node_tex_curve_in[] = {
	{	SOCK_VECTOR, 1, N_("Vector"),          0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, PROP_NONE, SOCK_HIDE_VALUE},
	{	SOCK_FLOAT, 1,  N_("LineThickness"),   0.01f,0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
	{	SOCK_VECTOR, 1, N_("CurveLocation"),   0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
	{	SOCK_VECTOR, 1, N_("CurveScale"),      1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
	{ 	SOCK_RGBA, 1, 	N_("BackgroundColor"), 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f},
	{ 	SOCK_RGBA, 1, 	N_("FillColor"), 	   1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 1.0f},
	{	-1, 0, ""	}
};

static bNodeSocketTemplate sh_node_tex_curve_out[] = {
	{	SOCK_RGBA, 0, N_("Color"),		0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f},
	{	-1, 0, ""	}
};

static void node_shader_init_tex_curve(bNodeTree *UNUSED(ntree), bNode *node)
{
    // TODO: TEXCURVE

	NodeTexCurve *tex = MEM_callocN(sizeof(NodeTexCurve), "NodeTexCurve");
	BKE_texture_mapping_default(&tex->base.tex_mapping, TEXMAP_TYPE_POINT);
	BKE_texture_colormapping_default(&tex->base.color_mapping);

	node->storage = tex;
}

//static int node_shader_gpu_tex_curve(GPUMaterial *mat, bNode *node, bNodeExecData *UNUSED(execdata), GPUNodeStack *in, GPUNodeStack *out)
//{
//	if (!in[0].link)
//		in[0].link = GPU_attribute(CD_ORCO, "");
//
//	node_shader_gpu_tex_mapping(mat, node, in, out);
//
//	return GPU_stack_link(mat, "node_tex_curve", in, out);
//}

/* node type definition */
void register_node_type_sh_tex_curve(void)
{
	static bNodeType ntype;

	sh_node_type_base(&ntype, SH_NODE_TEX_CURVE, "Curve Texture", NODE_CLASS_TEXTURE, 0);
	node_type_compatibility(&ntype, NODE_NEW_SHADING);
	node_type_socket_templates(&ntype, sh_node_tex_curve_in, sh_node_tex_curve_out);
	node_type_size_preset(&ntype, NODE_SIZE_MIDDLE);
	node_type_init(&ntype, node_shader_init_tex_curve);
	node_type_storage(&ntype, "NodeTexCurve", node_free_standard_storage, node_copy_standard_storage);
//	node_type_gpu(&ntype, node_shader_gpu_tex_curve);

	nodeRegisterType(&ntype);
}
