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

#ifndef _COM_MOTIONBLUR2DOPERATION_h
#define _COM_MOTIONBLUR2DOPERATION_h
#include "COM_NodeOperation.h"
#include "DNA_node_types.h"


class MotionBlur2DOperation : public NodeOperation {
private:
	/**
	* Cached reference to the inputProgram
	*/
	SocketReader *m_inputImageProgram;
	SocketReader *m_inputSpeedProgram;
	SocketReader *m_inputDepthProgram;
	SocketReader *m_inputObjIDProgram;

	NodeMotionBlur2D *m_settings;
	float *m_cachedInstance;

	// Sample buffer
	struct Sample {
		int x, y;
	};

	// Deep sample
	struct DeepSample {
		float obj_id;
		float color[4];
		float depth;
		float max_alpha;
		DeepSample *next_sample;
	};

	struct DeepSamplePixel {
		DeepSample *samples;
	};

	struct DeepSampleBuffer {
		DeepSampleBuffer *next_buffer;
		DeepSample buffers[1024 * 256];
	};

	DeepSample *samples;
	DeepSampleBuffer *buffers;

	DeepSample* alloc_sample(void);

public:
	MotionBlur2DOperation();

	/**
	* the inner loop of this program
	*/
	void executePixel(float output[4], int x, int y, void *data);
	bool determineDependingAreaOfInterest(rcti *input, ReadBufferOperation *readOperation, rcti *output);
	void *initializeTileData(rcti *rect);

	void line(int x0, int y0, int x1, int y1, Sample *samples, int *num_samples);

	void generateMotionBlur(float *data, MemoryBuffer *inputImage, MemoryBuffer *inputSpeed);
	void generateMotionBlurDeep(float *data, MemoryBuffer *color, MemoryBuffer *speed, MemoryBuffer *depth, MemoryBuffer *objid);

	static bool deepSamplesSortFn(DeepSample *a, DeepSample *b);

	void setMotionBlurSettings(NodeMotionBlur2D *settings) { this->m_settings = settings; }

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