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

#ifndef _COM_OTHEREYEOPERATION_h
#define _COM_OTHEREYEOPERATION_h
#include "COM_NodeOperation.h"
#include "DNA_node_types.h"
#include "DNA_camera_types.h"
#include "BKE_camera.h"


class OtherEyeOperation : public NodeOperation {
private:
	/**
	* Cached reference to the inputProgram
	*/
	SocketReader *m_inputImageProgram;
	SocketReader *m_inputDepthProgram;

	struct ID *m_camera;
	float *m_cachedInstance;
	
    void drawTriangle(float *data, float *depth_buffer,
                      float vt1[2], float c1[4], float d1,
                      float vt2[2], float c2[4], float d2,
                      float vt3[2], float c3[4], float d3);

    void reprojectLeftToRight(float r[3], float l[3], float left_to_world[4][4], float world_to_right[4][4], float A, float B);
    void generateReprojection(MemoryBuffer *color, MemoryBuffer *depth, float *data, float left_to_world[4][4], float world_to_right[4][4], float A, float B);

public:
	OtherEyeOperation();

	/**
	* the inner loop of this program
	*/
	void executePixel(float output[4], int x, int y, void *data);
	bool determineDependingAreaOfInterest(rcti *input, ReadBufferOperation *readOperation, rcti *output);
	void *initializeTileData(rcti *rect);
	float camera_stereo_shift(Object *camera);
	void ComputeCameraParamsViewplane(CameraParams *params, int width, int height);

	void setCamera(struct ID *camera) { m_camera = camera; }

	/**
	* Initialize the execution
	*/
	void initExecution();

	/**
	* Deinitialize the execution
	*/
	void deinitExecution();

};
#endif
