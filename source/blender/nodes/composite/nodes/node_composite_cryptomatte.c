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
 * Contributor(s): Lukas Stockner, Stefan Werner
 *
 * ***** END GPL LICENSE BLOCK *****
 */

/** \file blender/nodes/composite/nodes/node_composite_cryptomatte.c
 *  \ingroup cmpnodes
 */

#include "node_composite_util.h"

// I can't be bothered to do string operations without std::string
extern void cryptomatte_add(NodeCryptomatte* n, float f);
extern void cryptomatte_remove(NodeCryptomatte*n, float f);

static bNodeSocketTemplate outputs[] = {
	{	SOCK_RGBA,	0, N_("Image")},
	{	SOCK_FLOAT, 0, N_("Matte")},
	{	SOCK_RGBA,	0, N_("CryptoPick")},
	{	-1, 0, ""	}
};

void ntreeCompositCryptomatteSyncFromAdd(bNodeTree *UNUSED(ntree), bNode *node)
{
	NodeCryptomatte *n = node->storage;
	if(n->add[0] != 0.0f) {
		cryptomatte_add(n, n->add[0]);
		n->add[0] = 0.0f;
		n->add[1] = 0.0f;
		n->add[2] = 0.0f;
	}
}

void ntreeCompositCryptomatteSyncFromRemove(bNodeTree *UNUSED(ntree), bNode *node)
{
	NodeCryptomatte *n = node->storage;
	if(n->remove[0] != 0.0f) {
		cryptomatte_remove(n, n->remove[0]);
		n->remove[0] = 0.0f;
		n->remove[1] = 0.0f;
		n->remove[2] = 0.0f;
	}
}

bNodeSocket *ntreeCompositCryptomatteAddSocket(bNodeTree *ntree, bNode *node)
{
	NodeCryptomatte *n = node->storage;
	char sockname[32];
	n->num_inputs++;
	BLI_snprintf(sockname, sizeof(sockname), "Pass %d", n->num_inputs);
	bNodeSocket *sock = nodeAddStaticSocket(ntree, node, SOCK_IN, SOCK_RGBA, PROP_NONE, NULL, sockname);
	return sock;
}

int ntreeCompositCryptomatteRemoveSocket(bNodeTree *ntree, bNode *node)
{
	NodeCryptomatte *n = node->storage;
	if (n->num_inputs < 2) {
		return 0;
	}
	bNodeSocket *sock = node->inputs.last;
	nodeRemoveSocket(ntree, node, sock);
	n->num_inputs--;
	return 1;
}

static void init(bNodeTree *ntree, bNode *node)
{
	NodeCryptomatte *user = MEM_callocN(sizeof(NodeCryptomatte), "cryptomatte user");
	node->storage = user;


	nodeAddStaticSocket(ntree, node, SOCK_IN, SOCK_RGBA, PROP_NONE, "image", "Image");

	/* add two inputs by default */
	ntreeCompositCryptomatteAddSocket(ntree, node);
	ntreeCompositCryptomatteAddSocket(ntree, node);
}

void register_node_type_cmp_cryptomatte(void)
{
	static bNodeType ntype;

	cmp_node_type_base(&ntype, CMP_NODE_CRYPTOMATTE, "Cryptomatte", NODE_CLASS_CONVERTOR, 0);
	node_type_socket_templates(&ntype, NULL, outputs);
	node_type_init(&ntype, init);
	node_type_storage(&ntype, "NodeCryptomatte", node_free_standard_storage, node_copy_standard_storage);
	nodeRegisterType(&ntype);
}
