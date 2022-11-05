#pragma once

#include "MfxMesh.h"
#include "MfxBase.h"

#include "ofxCore.h"
#include "ofxMeshEffect.h"

/**
 * The input as used during the \ref MfxEffect::Cook action.
 * It is mostly used to retrieve the mesh data.
 * Let us recall the "input" may also mean output.
 */
class MfxInput : public MfxBase
{
private:
	friend class MfxEffect;
	MfxInput(const MfxHost& host, OfxMeshInputHandle input);

public:
	/**
	 * Get the mesh data flowing through this input.
	 */
	MfxMesh GetMesh();

private:
	OfxMeshInputHandle m_input;
};

