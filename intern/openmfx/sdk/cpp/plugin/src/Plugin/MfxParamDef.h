#pragma once

#include "MfxBase.h"

#include "ofxCore.h"
#include "ofxMeshEffect.h"

/**
 * A parameter definition is manipulated during the \ref MfxEffect::Describe
 * action to tell the host how to let the user interact with the parameter.
 * It differs from \ref MfxParam that is the parameter instance during the
 * cooking.
 */
template <typename T>
class MfxParamDef : public MfxBase
{
private:
	friend class MfxEffect;
	MfxParamDef(const MfxHost& host, OfxPropertySetHandle properties);

public:
	/**
	 * Set the human readable label to display for this parameter
	 */
	MfxParamDef & Label(const char *label);
	/**
	 * Set the range of values allowed for this parameter.
	 * Some hosts may not supporting distinct min/max for the different
	 * component of the parameter.
	 */
	MfxParamDef & Range(const T & minimum, const T & maximum);

private:
	OfxPropertySetHandle m_properties;
};

