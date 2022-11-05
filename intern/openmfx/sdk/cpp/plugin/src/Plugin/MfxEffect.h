#pragma once

#include "MfxInputDef.h"
#include "MfxParamDef.h"
#include "MfxInput.h"
#include "MfxParam.h"
#include "MfxHost.h"
#include "macros.h"

#include "ofxCore.h"
#include "ofxMeshEffect.h"
#include "ofxMessage.h"

#include <array>

/**
 * Defining a new effect is done by subclassing MfxEffect and implementing
 * some of its virtual methods. These methods correspond to
 * [actions](https://openmesheffect.org/Reference/ofxMeshEffectActions.html)
 * and some of them like \ref Describe and \ref Cook are mandatory.
 */
class MfxEffect
{
public:
	/**
	 * Override this in subclasses to give the effect a name
	 */
	virtual const char* GetName()
	{ return "MfxEffect"; }

	/**
	 * Equivalent of the \ref setHost entry point of an OpenFX plugin.
	 */
	void SetHost(OfxHost* host);

	/**
	 * Equivalent of the \ref mainEntry entry point of an OpenFX plugin,
	 * calling one of the action methods bellow.
	 */
	OfxStatus MainEntry(const char *action,
                        const void *handle,
                        OfxPropertySetHandle inArgs,
                        OfxPropertySetHandle outArgs);

protected:
	// Actions, to be defined in subclasses

	/// Equivalent of the \ref kOfxActionLoad
	virtual OfxStatus Load()
	{ return kOfxStatOK; }

	/// Equivalent of the \ref kOfxActionUnload
	virtual OfxStatus Unload()
	{ return kOfxStatOK; }

	/// Equivalent of the \ref kOfxActionDescribe
	virtual OfxStatus Describe(OfxMeshEffectHandle descriptor)
	{ (void)descriptor; return kOfxStatOK; }

	/// Equivalent of the \ref kOfxActionCreateInstance
	virtual OfxStatus CreateInstance(OfxMeshEffectHandle instance)
	{ (void)instance; return kOfxStatOK; }

	/// Equivalent of the \ref kOfxActionDestroyInstance
	virtual OfxStatus DestroyInstance(OfxMeshEffectHandle instance)
	{ (void)instance; return kOfxStatOK; }

	/// Equivalent of the \ref kOfxMeshEffectActionCook
	virtual OfxStatus Cook(OfxMeshEffectHandle instance)
	{ (void)instance; return kOfxStatOK; }

	/// Equivalent of the \ref kOfxMeshEffectActionIsIdentity
    virtual OfxStatus IsIdentity(OfxMeshEffectHandle instance)
    { (void)instance; return kOfxStatReplyDefault; }

protected:
	// Utility methods to be used during the Describe() action only:

	/**
	 * Define a new input/output slot of the effect. The name may be one of the
	 * standard \ref kOfxMeshMainInput or \ref kOfxMeshMainOutput or anything else
	 * for additionnal slots.
	 * Can **only** be used during the \ref Describe action.
	 */
	MfxInputDef AddInput(const char *name);

	/**
	 * Define a new parameter of the effect. The second argument (the default value)
	 * defines the type of the parameter.
	 * Can **only** be used during the \ref Describe action.
	 */
	MfxParamDef<int> AddParam(const char *name, int defaultValue);
	MfxParamDef<int2> AddParam(const char *name, const int2 & defaultValue);
	MfxParamDef<int3> AddParam(const char *name, const int3 & defaultValue);
	MfxParamDef<double> AddParam(const char *name, double defaultValue);
	MfxParamDef<double2> AddParam(const char *name, const double2 & defaultValue);
	MfxParamDef<double3> AddParam(const char *name, const double3 & defaultValue);
	MfxParamDef<bool> AddParam(const char* name, bool defaultValue);

protected:
	// Utility methods to be used during the Cook() action only:

	/**
	 * Get a given input instance by its name (e.g. \ref kOfxMeshMainInput)
	 * This is used to retrieve (for inputs) or define (for outputs) the
	 * mesh data flowing through it.
	 * Can **only** be used during the \ref Cook action.
	 */
	MfxInput GetInput(const char* name);

	/**
	 * Get a given parameter instance by its name as previously defined during the
	 * \ref Describe action. The resulting \ref MfxParam can be used to retrieve the
	 * current value of the parameter. The type of the parameter must be given as a
	 * template argument, e.g. `GetParam<int2>("size")`.
	 * Can **only** be used during the \ref Cook action.
	 */
	template <typename T>
	MfxParam<T> GetParam(const char* name);

private:
	/**
	 * Check that common suites are available
	 */
	bool CheckSuites();

	/**
	 * Cache the current descriptor and its parameter set to avoid providing
	 * it to each call to AddInput and AddParameter while describing the effect
	 * in the Describe() action.
	 */
	void SetupDescribe(OfxMeshEffectHandle descriptor);

	/**
	 * Cache the current instance and its parameter set to avoid providing
	 * it to each call to GetInput and GetParameter while cooking the effect
	 * in the Cook() action.
	 */
	void SetupCook(OfxMeshEffectHandle instance);

    /**
     * Cache the current instance and its parameter set to avoid providing
     * it to each call to GetInput and GetParameter while evaluating whether
     * the effect should be cooked in the IsIdentity() action.
     */
    void SetupIsIdentity(OfxMeshEffectHandle instance);

protected:
	/**
	 * The host() implicitely when using the \ref MFX_CHECK and \ref MFX_ENSURE
	 * macros.
	 */
	const MfxHost & host() const { return m_host; }

	/**
	 * Access to raw Open Mesh Effect suites, might be used when implementing actions
	 * (these member variables are not prefixed by 'm_' for more convenience)
	 * FIXME: They are useless if one uses the \ref MFX_CHECK macro correctly. Advanced
	 * uses can still call e.g. `host()->meshEffectSuite`.
	 */
	const OfxMeshEffectSuiteV1 *meshEffectSuite = nullptr;
    const OfxPropertySuiteV1 *propertySuite = nullptr;
    const OfxParameterSuiteV1 *parameterSuite = nullptr;
	const OfxMessageSuiteV2* messageSuite = nullptr;

private:
	MfxHost m_host;

	// Valid only during Describe()
	OfxMeshEffectHandle m_descriptor = nullptr;
	// Valid only during Cook()
	OfxMeshEffectHandle m_instance = nullptr;
	// Valid only during Describe() and Cook()
	OfxParamSetHandle m_parameters = nullptr;
};
