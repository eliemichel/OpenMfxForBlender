/*
 * Copyright 2016, Blender Foundation.
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
 * Contributor:
 *              Lukas Stockner
 */

#include "COM_CryptomatteNode.h"
#include "COM_CryptomatteOperation.h"

CryptomatteNode::CryptomatteNode(bNode *editorNode) : Node(editorNode)
{
	/* pass */
}

void CryptomatteNode::convertToOperations(NodeConverter &converter, const CompositorContext &/*context*/) const
{
	NodeOutput *outputSocket = this->getOutputSocket(0);

	CryptomatteOperation *operation = new CryptomatteOperation();
	operation->setObjectIndex(this->getbNode()->custom1);
	converter.addOperation(operation);

	for(int i = 0; i < 6; i++)
		converter.mapInputSocket(this->getInputSocket(i), operation->getInputSocket(i));
	converter.mapOutputSocket(outputSocket, operation->getOutputSocket(0));
}
