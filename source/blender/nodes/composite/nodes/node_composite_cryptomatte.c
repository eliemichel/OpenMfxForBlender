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
 * The Original Code is Copyright (C) 2016 Blender Foundation.
 * All rights reserved.
 *
 * The Original Code is: all of this file.
 *
 * Contributor(s): Lukas Stockner
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/nodes/composite/nodes/node_composite_cryptomatte.c
 *  \ingroup cmpnodes
 */

#include "node_composite_util.h"

static bNodeSocketTemplate inputs[] = {
	{	SOCK_RGBA, 1, N_("Pass 1"),			0.0f, 0.0f, 0.0f, 1.0f},
	{	SOCK_RGBA, 1, N_("Pass 2"),			0.0f, 0.0f, 0.0f, 1.0f},
	{	SOCK_RGBA, 1, N_("Pass 3"),			0.0f, 0.0f, 0.0f, 1.0f},
	{	SOCK_RGBA, 1, N_("Pass 4"),			0.0f, 0.0f, 0.0f, 1.0f},
	{	SOCK_RGBA, 1, N_("Pass 5"),			0.0f, 0.0f, 0.0f, 1.0f},
	{	SOCK_RGBA, 1, N_("Pass 6"),			0.0f, 0.0f, 0.0f, 1.0f},
	{	-1, 0, ""	}
};
static bNodeSocketTemplate outputs[] = {
	{	SOCK_FLOAT, 0, N_("Mask")},
	{	-1, 0, ""	}
};

static void init(bNodeTree *UNUSED(ntree), bNode *node)
{
	node->custom1 = 0;
}

void register_node_type_cmp_cryptomatte(void)
{
	static bNodeType ntype;

	cmp_node_type_base(&ntype, CMP_NODE_CRYPTOMATTE, "Cryptomatte", NODE_CLASS_CONVERTOR, 0);
	node_type_socket_templates(&ntype, inputs, outputs);
	node_type_init(&ntype, init);

	nodeRegisterType(&ntype);
}
