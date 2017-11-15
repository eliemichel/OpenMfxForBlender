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
* The Original Code is Copyright (C) 2014 Blender Foundation.
* All rights reserved.
*
* The Original Code is: all of this file.
*
* Contributor(s): Tod Baudais
*
* ***** END GPL LICENSE BLOCK *****
*/

/** \file blender/nodes/composite/nodes/node_composite_othereye.c
*  \ingroup cmpnodes
*/

#include "node_composite_util.h"
#include "BKE_context.h"

static bNodeSocketTemplate inputs[] = {
    { SOCK_RGBA, 1, N_("Image"), 1.0f, 1.0f, 1.0f, 1.0f },
    { SOCK_FLOAT, 1, N_("Z"), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, PROP_NONE },
	{ -1, 0, "" }
};
static bNodeSocketTemplate outputs[] = {
    { SOCK_RGBA, 0, N_("Orig Image") },
    { SOCK_RGBA, 0, N_("Other Image") },
    { SOCK_RGBA, 0, N_("Render Mask") },
	{ -1, 0, "" }
};

static void init(const bContext *C, PointerRNA *ptr)
{
	bNode *node = ptr->data;
	Scene *scene = CTX_data_scene(C);
	NodeOtherEye *user = MEM_callocN(sizeof(NodeOtherEye), "other eye");

	node->id = (ID *)scene->clip;
	node->storage = user;
//    user->fat_mode = false;
//    user->amount = 1.0f;
//    user->multisample = 1;
//    user->fill_alpha_holes = true;
}

void register_node_type_cmp_othereye(void)
{
	static bNodeType ntype;

	cmp_node_type_base(&ntype, CMP_NODE_OTHEREYE, "Other Eye", NODE_CLASS_OP_FILTER, 0);
	node_type_socket_templates(&ntype, inputs, outputs);
	ntype.initfunc_api = init;
	node_type_storage(&ntype, "NodeOtherEye", node_free_standard_storage, node_copy_standard_storage);
	nodeRegisterType(&ntype);
}
