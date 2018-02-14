/*
* Copyright 2011, Blender Foundation.
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
*		Jeroen Bakker
*		Monique Dewanchand
*		Cristian Kovacs (Just the multi add node)
*/

#include "COM_MultiAddOperation.h"
#include "COM_MultiAddNode.h"

#include "COM_ExecutionSystem.h"
#include "COM_SetValueOperation.h"
#include "DNA_material_types.h" // the ramp types

MultiAddNode::MultiAddNode(bNode *editorNode) : Node(editorNode)
{
	/* pass */
}

void MultiAddNode::convertToOperations(NodeConverter &converter, const CompositorContext &/*context*/) const
{
	NodeInput *valueSocket = this->getInputSocket(0);
	NodeOutput *outputSocket = this->getOutputSocket(0);
	bool useAlphaPremultiply = (this->getbNode()->custom2 & 1) != 0;
	bool useClamp = (this->getbNode()->custom2 & 2) != 0;

	MultiAddOperation *prog = new MultiAddOperation(getNumberOfInputSockets());
	//prog->setUseValueAlphaMultiply(useAlphaPremultiply);
	prog->setUseClamp(useClamp);
	converter.addOperation(prog);

	converter.mapInputSocket(valueSocket, prog->getInputSocket(0));
	for (int i = 1; i < getNumberOfInputSockets(); i++) {
		converter.mapInputSocket(this->getInputSocket(i), prog->getInputSocket(i));
	}
	converter.mapOutputSocket(outputSocket, prog->getOutputSocket(0));
	converter.addPreview(prog->getOutputSocket(0));
}