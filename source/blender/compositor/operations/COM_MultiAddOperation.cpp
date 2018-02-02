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
*		Cristian Kovacs (Only the Multi Add Node)
*/

#include "COM_MultiAddOperation.h"

extern "C" {
#  include "BLI_math.h"
}

/* ******** Multi Add Operation ******** */
MultiAddOperation::MultiAddOperation(size_t num_inputs) : NodeOperation()
{

	this->addInputSocket(COM_DT_VALUE);
	for (size_t i = 1; i < num_inputs; i++) {
		this->addInputSocket(COM_DT_COLOR);
	}
	inputs.resize(num_inputs);
	this->addOutputSocket(COM_DT_COLOR);
	this->m_inputValueOperation = NULL;
	//this->m_inputColor1Operation = NULL;
	//this->m_inputColor2Operation = NULL;
	this->setUseValueAlphaMultiply(false);
	this->setUseClamp(false);
}

void MultiAddOperation::initExecution()
{
	this->m_inputValueOperation = this->getInputSocketReader(0);
	for (size_t i = 1; i < inputs.size(); i++) {
		inputs[i] = this->getInputSocketReader(i);
	}

	//this->m_inputColor1Operation = this->getInputSocketReader(1);
	//this->m_inputColor2Operation = this->getInputSocketReader(2);
}

void MultiAddOperation::executePixelSampled(float output[4], float x, float y, PixelSampler sampler)
{
	float inputColor1[4];
	float inputColor2[4];
	float inputValue[4];
	float (*inputColors)[4];

	this->m_inputValueOperation->readSampled(inputValue, x, y, sampler);
	for (size_t i = 0; i < inputs.size(); i++) {
		inputs[i]->readSampled(inputColors[i], x, y, sampler);
	}


	//this->m_inputColor1Operation->readSampled(inputColor1, x, y, sampler);
	//this->m_inputColor2Operation->readSampled(inputColor2, x, y, sampler);

	float value = inputValue[0];
	if (this->useValueAlphaMultiply()) {
		for (size_t i = 1; i < inputs.size(); i++) {
			value *= inputColors[i][3];
		}
	}

	output[0] = inputColors[0][0];
	output[1] = inputColors[0][1];
	output[2] = inputColors[0][2];
	output[3] = inputColors[0][3];

	for (size_t i = 1; i < inputs.size(); i++) {
		output[0] += value * inputColors[i][0];
		output[1] += value * inputColors[i][1];
		output[2] += value * inputColors[i][2];
	}

	//output[0] = inputColor1[0] + value * inputColor2[0];
	//output[1] = inputColor1[1] + value * inputColor2[1];
	//output[2] = inputColor1[2] + value * inputColor2[2];
	//output[3] = inputColor1[3];

	clampIfNeeded(output);
}

void MultiAddOperation::determineResolution(unsigned int resolution[2], unsigned int preferredResolution[2])
{
	NodeOperationInput *socket;
	unsigned int tempPreferredResolution[2] = { 0, 0 };
	unsigned int tempResolution[2];

	bool input_set = false;
	for (size_t i = 1; i < inputs.size(); i++) {
		socket = this->getInputSocket(i);
		socket->determineResolution(tempResolution, tempPreferredResolution);
		if ((tempResolution[0] != 0) && (tempResolution[1] != 0)) {
			this->setResolutionInputSocketIndex(i);
			input_set = true;
		}
	}
	if (!input_set)
		this->setResolutionInputSocketIndex(0);
	NodeOperation::determineResolution(resolution, preferredResolution);

	//socket = this->getInputSocket(1);
	//socket->determineResolution(tempResolution, tempPreferredResolution);
	//if ((tempResolution[0] != 0) && (tempResolution[1] != 0)) {
	//	this->setResolutionInputSocketIndex(1);
	//}
	//else {
	//	socket = this->getInputSocket(2);
	//	socket->determineResolution(tempResolution, tempPreferredResolution);
	//	if ((tempResolution[0] != 0) && (tempResolution[1] != 0)) {
	//		this->setResolutionInputSocketIndex(2);
	//	}
	//	else {
	//		this->setResolutionInputSocketIndex(0);
	//	}
	//}
	//NodeOperation::determineResolution(resolution, preferredResolution);



}

void MultiAddOperation::deinitExecution()
{
	this->m_inputValueOperation = NULL;
	for (size_t i = 0; i < inputs.size(); i++) {
		inputs[i] = NULL;
	}
	//this->m_inputColor1Operation = NULL;
	//this->m_inputColor2Operation = NULL;
}