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
#include "BKE_object.h"
#include "BKE_camera.h"
#include "DNA_object_types.h"

#include <iostream>
#include <limits>

#define INDEX_COL(x,y) ((y * getWidth() + x) * COM_NUM_CHANNELS_COLOR)
#define INDEX_VAL(x,y) ((y * getWidth() + x) * COM_NUM_CHANNELS_VALUE)

OtherEyeOperation::OtherEyeOperation() : NodeOperation()
{
	addInputSocket(COM_DT_COLOR);
	addInputSocket(COM_DT_VALUE); // ZBUF
    addOutputSocket(COM_DT_COLOR); // Orig
    addOutputSocket(COM_DT_COLOR); // Other
    addOutputSocket(COM_DT_COLOR); // Render Mask
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

        // Camera matrices
        float left_to_world[4][4];
        float world_to_right[4][4];

        Object *camera = (Object*) m_camera;
        if (camera) {

            //
            // Calculate left
            //
            
            CameraParams params;
            
            // View matrix inv
            float viewinv[4][4];
            float viewmat[4][4];

            copy_m4_m4(viewinv, camera->obmat);
            normalize_m4(viewinv);
            invert_m4_m4(viewmat, viewinv);

            // Window matrix, clipping and ortho
            BKE_camera_params_init(&params);
            BKE_camera_params_from_object(&params, camera);
            BKE_camera_params_compute_viewplane(&params, getWidth(), getHeight(), 1.0f, 1.0f);
            BKE_camera_params_compute_matrix(&params);
            
            mul_m4_m4m4(left_to_world, params.winmat, viewinv);

            //
            // Calculate right
            //
        
            // TODO
            
        } else {
            unit_m4(left_to_world);
            unit_m4(world_to_right);
        }

        generateReprojection(color, depth, data, left_to_world, world_to_right);

		m_cachedInstance = data;
	}
	unlockMutex();
	return m_cachedInstance;
}

void OtherEyeOperation::drawTriangle(float *data, float *depth_buffer,
                                     float vt1[2], float c1[4], float d1,
                                     float vt2[2], float c2[4], float d2,
                                     float vt3[2], float c3[4], float d3)
{
    /* spanning vectors of edge (v1,v2) and (v1,v3) */
    float vs1[2] = {vt2[0] - vt1[0], vt2[1] - vt1[1]};
    float vs2[2] = {vt3[0] - vt1[0], vt3[1] - vt1[1]};
    
    float den = 1.0F / cross_v2v2(vs1, vs2);
    mul_v2_fl(vs1,den);
    mul_v2_fl(vs2,den);

    float minX = std::min(std::min(vt1[0], vt2[0]), vt3[0]);
    float maxX = std::max(std::max(vt1[0], vt2[0]), vt3[0]);
    float minY = std::min(std::min(vt1[1], vt2[1]), vt3[1]);
    float maxY = std::max(std::max(vt1[1], vt2[1]), vt3[1]);
    
    if (minX < 0)   minX = 0;
    if (minY < 0)   minY = 0;
    if (maxX >= getWidth())  maxX = getWidth()-1;
    if (maxY >= getHeight()) maxY = getHeight()-1;

    for (int x = minX; x <= maxX; x++) {
        for (int y = minY; y <= maxY; y++) {
            float q[2] = {x - vt1[0], y - vt1[1]};

            float s = cross_v2v2(q, vs2);
            float t = cross_v2v2(vs1, q);

            if ( (s >= 0) && (t >= 0) && (s + t <= 1)) {
                float u = 1 - (s+t);

                float di = s*d2 + t*d3 + u*d1; // Interpolated depth
                
                int index_val = INDEX_VAL(x, y);
                float *depth_pixel = depth_buffer + index_val;
                
                if (di > *depth_pixel) {
                    float ci[4]; // Interpolated color
                    ci[0] = s*c2[0] + t*c3[0] + u*c1[0];
                    ci[1] = s*c2[1] + t*c3[1] + u*c1[1];
                    ci[2] = s*c2[2] + t*c3[2] + u*c1[2];
                    ci[3] = s*c2[3] + t*c3[3] + u*c1[3];

                    int index_col = INDEX_COL(x, y);
                    float *data_pixel = data + index_col;

                    data_pixel[0] = ci[0];
                    data_pixel[1] = ci[1];
                    data_pixel[2] = ci[2];
                    data_pixel[3] = ci[3];
                }
            }
        }
    }
}

void OtherEyeOperation::reprojectLeftToRight(float r[3], float l[3], float left_to_world[4][4], float world_to_right[4][4])
{
    float l4[4] = {l[0], l[1], l[2], 1.0F};
    float r4[4];

    // Left camera to world transformation
    float w[4];
    mul_v4_m4v4(w, left_to_world, l4);
    w[0] /= w[3];
    w[1] /= w[3];
    w[2] /= w[3];

    // w should be in world space now

    // World to right eye
    mul_v4_m4v4(r4, world_to_right, w);
    
    r[0] = r4[0];
    r[1] = r4[1];
    r[2] = r4[2];
}

void OtherEyeOperation::generateReprojection(MemoryBuffer *color, MemoryBuffer *depth, float *data, float left_to_world[4][4], float world_to_right[4][4])
{
	float *depth_buffer = (float *) MEM_callocN(MEM_allocN_len(depth->getBuffer()), "Other eye depth buffer");

    int width = getWidth();
    int height = getHeight();
    
    for (int y = 0; y < height-1; ++y) {
        for (int x = 0; x < width-1; ++x) {
        
            float *color_pixel_00 = color->getBuffer() + INDEX_COL(x, y);
            float *color_pixel_10 = color->getBuffer() + INDEX_COL(x+1, y);
            float *color_pixel_01 = color->getBuffer() + INDEX_COL(x, y+1);
            float *color_pixel_11 = color->getBuffer() + INDEX_COL(x+1, y+1);

            float *depth_pixel_00 = depth->getBuffer() + INDEX_VAL(x, y);
            float *depth_pixel_10 = depth->getBuffer() + INDEX_VAL(x+1, y);
            float *depth_pixel_01 = depth->getBuffer() + INDEX_VAL(x, y+1);
            float *depth_pixel_11 = depth->getBuffer() + INDEX_VAL(x+1, y+1);

            float l_00[3] = {x, y, *depth_pixel_00};
            float r_00[3];

            float l_10[3] = {x+1, y, *depth_pixel_10};
            float r_10[3];

            float l_01[3] = {x, y+1, *depth_pixel_01};
            float r_01[3];

            float l_11[3] = {x+1, y+1, *depth_pixel_11};
            float r_11[3];

            reprojectLeftToRight(r_00, l_00, left_to_world, world_to_right);
            reprojectLeftToRight(r_10, l_10, left_to_world, world_to_right);
            reprojectLeftToRight(r_01, l_01, left_to_world, world_to_right);
            reprojectLeftToRight(r_11, l_11, left_to_world, world_to_right);

            drawTriangle(data, depth_buffer,
                         r_00, color_pixel_00, r_00[2],
                         r_11, color_pixel_11, r_11[2],
                         r_10, color_pixel_10, r_10[2]);
            drawTriangle(data, depth_buffer,
                         r_00, color_pixel_00, r_00[2],
                         r_11, color_pixel_11, r_11[2],
                         r_01, color_pixel_01, r_01[2]);
        }
    }

    MEM_freeN(depth_buffer);
}


void OtherEyeOperation::executePixel(float output[4], int x, int y, void *data)
{
	float *buffer = (float *)data;
	copy_v4_v4(output, &buffer[INDEX_COL(x, y)]);
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




