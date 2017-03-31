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
 * Contributor: Lukas Stockner, Stefan Werner
 */

#include "COM_CryptomatteOperation.h"

CryptomatteOperation::CryptomatteOperation() : NodeOperation()
{
	for(int i = 0; i < 6; i++) {
		inputs[i] = NULL;
		this->addInputSocket(COM_DT_COLOR);
	}
	this->addOutputSocket(COM_DT_COLOR);
	this->setComplex(true);
}

void CryptomatteOperation::initExecution()
{
	for(int i = 0; i < 6; i++)
		inputs[i] = this->getInputSocketReader(i);
}

void CryptomatteOperation::addObjectIndex(float objectIndex)
{
	if(objectIndex != 0.f)
		m_objectIndex.push_back(objectIndex);
}

void CryptomatteOperation::executePixel(float output[4],
                                   int x,
                                   int y,
                                   void *data)
{
	float input[4];
	output[0] = output[1] = output[2] = output[3] = 0.0f;
	for(int i = 0; i < 6; i++) {
		inputs[i]->read(input, x, y, data);
		if (i == 0) {
			// write the frontmost object as false color for picking
			output[0] = input[0] >= 0.f ? input[0] : 0.f;
			output[1] = input[0] < 0.f ? -input[0] : 0.f;
			uint32_t m3hash;
			::memcpy(&m3hash, &input[0], 4);
			output[2] = ((float) ((m3hash << 16)) / (float) UINT32_MAX);
		}
		for(float f : m_objectIndex) {
			if (f == input[0])
				output[3] += input[1];
			if (f == input[2])
				output[3] += input[3];
		}
	}
}
