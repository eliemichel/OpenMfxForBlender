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
#include "DNA_object_types.h"
#include "DNA_camera_types.h"

#include <iostream>
#include <limits>

#define INDEX_COL(x,y) ((y * getWidth() + x) * COM_NUM_CHANNELS_COLOR)
#define INDEX_VAL(x,y) ((y * getWidth() + x) * COM_NUM_CHANNELS_VALUE)

extern "C" {
    void camera_stereo3d_model_matrix(Object *camera, const bool is_left, float r_modelmat[4][4]);
}

OtherEyeOperation::OtherEyeOperation() : NodeOperation()
{
	addInputSocket(COM_DT_COLOR);
	addInputSocket(COM_DT_VALUE); // ZBUF
    addOutputSocket(COM_DT_COLOR); // Other
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
        float A = 1.0f;
        float B = 1.0f;

        Object *camera = (Object*) m_camera;
        if (camera) {
			CameraParams params;
			BKE_camera_params_init(&params);
			BKE_camera_params_from_object(&params, camera);

			// Still need to set up left_to_world and world_to_right
			// c == 0: Left eye to world
			// c == 1: World to right eye
            for (int c = 0; c < 2; ++c) {
                CameraParams params;
                BKE_camera_params_init(&params);
                BKE_camera_params_from_object(&params, camera);

                float viewmat[4][4];
                float viewinv[4][4];
                camera_stereo3d_model_matrix(camera, c == 0, viewmat);
                
                // Cycles inverts z axis
                viewmat[2][0] *= -1.0F;
                viewmat[2][1] *= -1.0F;
                viewmat[2][2] *= -1.0F;
                viewmat[2][3] *= -1.0F;

                invert_m4_m4(viewinv, viewmat);
                
                float cameratondc[4][4];
                float ndctoraster[4][4];
                float cameratoraster[4][4];
                float worldtoraster[4][4];

                // focallength_to_fov
                float fov = 2.0f * atanf((params.sensor_y / 2.0f) / params.lens);

                computePerspective( cameratondc, ndctoraster,
                                    fov, params.zoom, params.clipsta, params.clipend,
                                    params.shiftx, params.shifty, params.offsetx, params.offsety);
                mul_m4_m4m4(cameratoraster, ndctoraster, cameratondc);
                mul_m4_m4m4(worldtoraster, cameratoraster, viewinv);

                if (c == 0) {
                    invert_m4_m4(left_to_world, worldtoraster);
                    A = cameratondc[2][2];
                    B = cameratondc[3][2];
                    
                    // mat[2][2] = A = -(farClip + nearClip) / (farClip - nearClip);
                    // mat[3][2] = B = (-2.0f * nearClip * farClip) / (farClip - nearClip);
                } else {
                    copy_m4_m4(world_to_right, worldtoraster);
                }
                
            }

        } else {
            unit_m4(left_to_world);
            unit_m4(world_to_right);
        }

        generateReprojection(color, depth, data, left_to_world, world_to_right, A, B);

		m_cachedInstance = data;
	}
	unlockMutex();
	return m_cachedInstance;
}

void OtherEyeOperation::computePerspective( float cameratondc[4][4], float ndctoraster[4][4],
                                            float fov, float zoom, float near, float far,
                                            float shift_x, float shift_y, float offset_x, float offset_y)
{
    float width = (float)getWidth();
    float height = (float)getHeight();

	// full viewport to camera border in the viewport.
	float fulltoborder[4][4];
	float bordertofull[4][4];
 
	transformFromViewplane(fulltoborder, 0.0f, 1.0f, 0.0f, 1.0f);
	invert_m4_m4(bordertofull, fulltoborder);

	// ndc to raster
    zero_m4(ndctoraster);
    ndctoraster[0][0] = width;
    ndctoraster[1][1] = height;
    ndctoraster[2][2] = 1.0f;
    ndctoraster[3][3] = 1.0f;
    
    mul_m4_m4m4(ndctoraster, ndctoraster, bordertofull);

    // Viewplane
    float aspect = width/height;
    float xaspect = aspect;
    float yaspect = 1.0f;

    float viewplane_left = -xaspect * zoom;
    float viewplane_right = xaspect * zoom;
    float viewplane_bottom = -yaspect * zoom;
    float viewplane_top = yaspect * zoom;

    /* modify viewplane with camera shift and 3d camera view offset */
    float dx = 2.0f * (aspect * shift_x + offset_x * xaspect * 2.0f);
    float dy = 2.0f * (aspect * shift_y + offset_y * yaspect * 2.0f);

    viewplane_left += dx;
    viewplane_right += dx;
    viewplane_bottom += dy;
    viewplane_top += dy;

    // screen to ndc
    float screentondc[4][4];
    float viewplane[4][4];
    transformFromViewplane(viewplane, viewplane_left, viewplane_right, viewplane_bottom, viewplane_top);
    mul_m4_m4m4(screentondc, fulltoborder, viewplane);

    // screen to camera
    //float fov = M_PI_4_F;
    float scale[4][4];
    float persp[4][4];
    
    //mat[3][0] x
    //mat[3][1] y
    //mat[3][2] z

    zero_m4(persp);
    persp[0][0] = persp[1][1] = persp[2][3] = 1.0f;
    persp[2][2] = far / (far - near);
    persp[3][2] = -far * near / (far - near);
    //invert_m4(persp);   // TODO: ???
    
    zero_m4(scale);
    float inv_angle = 1.0f / tanf(0.5f * fov);
    scale[0][0] = inv_angle;
    scale[1][1] = inv_angle;
    scale[2][2] = 1.0f;
    scale[3][3] = 1.0f;

    float cameratoscreen[4][4];
    mul_m4_m4m4(cameratoscreen, scale, persp);
    
	// camera to ndc
	mul_m4_m4m4(cameratondc, screentondc, cameratoscreen);
}

void OtherEyeOperation::transformFromViewplane(float transformation[4][4], float left, float right, float bottom, float top)
{
	// scale matrix
	float scale[4][4];
	zero_m4(scale);
	scale[0][0] = 1.0f / (right - left);
	scale[1][1] = 1.0f / (top - bottom);
	scale[2][2] = 1.0f;
	scale[3][3] = 1.0f;

	// translate matrix
	float translate[4][4];
	unit_m4(translate);
	translate[3][0] = -left;
	translate[3][1] = -bottom;
	translate[3][2] = 0.0f;

	mul_m4_m4m4(transformation, scale, translate);
}

void OtherEyeOperation::drawTriangle(float *data, float *depth_buffer,
                                     float vt1[2], float c1[4], float d1,
                                     float vt2[2], float c2[4], float d2,
                                     float vt3[2], float c3[4], float d3)
{
    /* spanning vectors of edge (v1,v2) and (v1,v3) */
    float vs1[2] = {vt2[0] - vt1[0], vt2[1] - vt1[1]};
    float vs2[2] = {vt3[0] - vt1[0], vt3[1] - vt1[1]};
    
    float den = 1.0f / cross_v2v2(vs1, vs2);
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

void OtherEyeOperation::reprojectLeftToRight(float r[3], float l[3], float left_to_world[4][4], float world_to_right[4][4], float A, float B)
{

    // Correct z
    auto world_depth = l[2];
    
    // i.e. near_clip = -1.0, far clip = 1.0
    float normalized_depth = (-A * world_depth + B) / world_depth + 2.0f;
    
    // Build 4 component vector
    float l4[4] = {l[0], l[1], normalized_depth, 1.0f};
    float r4[4];

    // Left camera to world transformation
    float w[4];
    mul_v4_m4v4(w, left_to_world, l4);
//    w[0] /= w[3];
//    w[1] /= w[3];
//    w[2] /= w[3];
//    w[3] = 1.0f;

    // w should be in world space now

    // World to right eye
    mul_v4_m4v4(r4, world_to_right, w);
    
    // Round to fix any round off error
    r[0] = roundf(r4[0]/r4[3]);
    r[1] = roundf(r4[1]/r4[3]);
    r[2] = roundf(r4[2]/r4[3]);
}

void OtherEyeOperation::generateReprojection(MemoryBuffer *color, MemoryBuffer *depth, float *data, float left_to_world[4][4], float world_to_right[4][4], float A, float B)
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
        
            reprojectLeftToRight(r_00, l_00, left_to_world, world_to_right, A, B);
            reprojectLeftToRight(r_10, l_10, left_to_world, world_to_right, A, B);
            reprojectLeftToRight(r_01, l_01, left_to_world, world_to_right, A, B);
            reprojectLeftToRight(r_11, l_11, left_to_world, world_to_right, A, B);

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




