#pragma once

#include "MfxBase.h"

#include "ofxCore.h"
#include "ofxMeshEffect.h"

/**
 * A parameter as used during the \ref MfxEffect::Cook action.
 * It is mostly used to retrieve its value.
 */
template <typename T>
class MfxParam : public MfxBase
{
private:
	friend class MfxEffect;
	MfxParam(const MfxHost & host, OfxParamHandle param);

public:
	/**
	 * Evaluate the parameter.
	 */
	T GetValue();

private:
	OfxParamHandle m_param;
};

