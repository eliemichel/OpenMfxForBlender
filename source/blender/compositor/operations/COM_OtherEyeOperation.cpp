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
*		Tod Baudais
*/

#include "COM_OtherEyeOperation.h"
#include "MEM_guardedalloc.h"
#include "RNA_blender_cpp.h"

#include <iostream>
#include <limits>

OtherEyeOperation::OtherEyeOperation() : NodeOperation()
{
	addInputSocket(COM_DT_COLOR);
	addInputSocket(COM_DT_VALUE); // ZBUF
    addOutputSocket(COM_DT_COLOR); // Orig
    addOutputSocket(COM_DT_COLOR); // Other
    addOutputSocket(COM_DT_COLOR); // Render Mask
	m_settings = NULL;
	m_inputImageProgram = NULL;
	m_inputDepthProgram = NULL;
	m_cachedInstance = NULL;
	setComplex(true);
}
void OtherEyeOperation::initExecution()
{
	initMutex();
	m_inputImageProgram = getInputSocketReader(0);
	m_inputDepthProgram = getInputSocketReader(1);
	m_cachedInstance = NULL;
}

void OtherEyeOperation::deinitExecution()
{
	deinitMutex();
	if (m_cachedInstance) {
		MEM_freeN(m_cachedInstance);
		m_cachedInstance = NULL;
	}
}

void *OtherEyeOperation::initializeTileData(rcti *rect)
{
	if (m_cachedInstance) {
		return m_cachedInstance;
	}

	lockMutex();
	if (m_cachedInstance == NULL) {
		MemoryBuffer *color = (MemoryBuffer *)m_inputImageProgram->initializeTileData(rect);
        MemoryBuffer *depth = (MemoryBuffer *)m_inputDepthProgram->initializeTileData(rect);

        float *data = (float *)MEM_callocN(MEM_allocN_len(color->getBuffer()), "Other eye data buffer");

        // Recover ID pointer
        PointerRNA ptr;
        RNA_id_pointer_create((ID*)m_settings->camera, &ptr);
        BL::ID b_id(ptr);

        if(b_id.is_a(&RNA_Camera)) {
            BL::Camera b_camera(b_id);
            
            
        }

		m_cachedInstance = data;
	}
	unlockMutex();
	return m_cachedInstance;
}

void OtherEyeOperation::executePixel(float output[4], int x, int y, void *data)
{
	float *buffer = (float *)data;
	//copy_v4_v4(output, &buffer[INDEX_COL(x, y)]);
}

bool OtherEyeOperation::determineDependingAreaOfInterest(rcti *input, ReadBufferOperation *readOperation, rcti *output)
{
	if (m_cachedInstance == NULL) {
		rcti newInput;
		newInput.xmax = getWidth();
		newInput.xmin = 0;
		newInput.ymax = getHeight();
		newInput.ymin = 0;
		return NodeOperation::determineDependingAreaOfInterest(&newInput, readOperation, output);
	}
	else {
		return false;
	}
}




