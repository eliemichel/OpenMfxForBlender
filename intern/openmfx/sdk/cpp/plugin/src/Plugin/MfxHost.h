#pragma once

#include "ofxCore.h"
#include "ofxMeshEffect.h"
#include "ofxMessage.h"

/**
 * Equivalent of the \ref OfxHost handle that keeps a reference
 * to supported C suites. One will usually use the C++ methods
 * instead but for some advanced use not supported by the C++ API
 * one can always fall back to using these.
 */
class MfxHost
{
public:
    const OfxMeshEffectSuiteV1* meshEffectSuite = nullptr;
    const OfxPropertySuiteV1* propertySuite = nullptr;
    const OfxParameterSuiteV1* parameterSuite = nullptr;
    const OfxMessageSuiteV2* messageSuite = nullptr;
};

