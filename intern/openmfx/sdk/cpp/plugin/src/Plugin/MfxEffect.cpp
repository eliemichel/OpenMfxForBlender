#include "MfxEffect.h"
#include "MfxSuiteException.h"
#include "macros.h"

#include <iostream>
#include <cstring>

void MfxEffect::SetHost(OfxHost* host)
{
    if (NULL != host) {
        m_host.propertySuite = static_cast<const OfxPropertySuiteV1*>(host->fetchSuite(host->host, kOfxPropertySuite, 1));
        m_host.parameterSuite = static_cast<const OfxParameterSuiteV1*>(host->fetchSuite(host->host, kOfxParameterSuite, 1));
        m_host.meshEffectSuite = static_cast<const OfxMeshEffectSuiteV1*>(host->fetchSuite(host->host, kOfxMeshEffectSuite, 1));
        m_host.messageSuite = static_cast<const OfxMessageSuiteV2*>(host->fetchSuite(host->host, kOfxMessageSuite, 2));
        // aliases for more convenience
        propertySuite = m_host.propertySuite;
        parameterSuite = m_host.parameterSuite;
        meshEffectSuite = m_host.meshEffectSuite;
        messageSuite = m_host.messageSuite;
    }
}

OfxStatus MfxEffect::MainEntry(const char *action,
                               const void *handle,
                               OfxPropertySetHandle inArgs,
                               OfxPropertySetHandle outArgs)
{
    try {
        if (0 == strcmp(action, kOfxActionLoad)) {
            return Load();
        }
        if (0 == strcmp(action, kOfxActionUnload)) {
            return Unload();
        }
        if (0 == strcmp(action, kOfxActionDescribe)) {
            if (!CheckSuites()) {
                return kOfxStatErrMissingHostFeature;
            }
            SetupDescribe((OfxMeshEffectHandle)handle);
            return Describe((OfxMeshEffectHandle)handle);
        }
        if (0 == strcmp(action, kOfxActionCreateInstance)) {
            return CreateInstance((OfxMeshEffectHandle)handle);
        }
        if (0 == strcmp(action, kOfxActionDestroyInstance)) {
            return DestroyInstance((OfxMeshEffectHandle)handle);
        }
        if (0 == strcmp(action, kOfxMeshEffectActionCook)) {
            SetupCook((OfxMeshEffectHandle)handle);
            return Cook((OfxMeshEffectHandle)handle);
        }
        if (0 == strcmp(action, kOfxMeshEffectActionIsIdentity)) {
            SetupIsIdentity((OfxMeshEffectHandle)handle);
            return IsIdentity((OfxMeshEffectHandle)handle);
        }
        return kOfxStatReplyDefault;
    }
    catch (MfxSuiteException &e)
    {
        std::cerr << e.what() << std::endl;
        return e.GetStatus();
    }
}

//-----------------------------------------------------------------------------

MfxInputDef MfxEffect::AddInput(const char *name)
{
    OfxPropertySetHandle inputProps;
    OfxMeshInputHandle input;
    MFX_ENSURE(meshEffectSuite->inputDefine(m_descriptor, name, &input, &inputProps));
    return MfxInputDef(host(), input, inputProps);
}

MfxParamDef<int> MfxEffect::AddParam(const char *name, int defaultValue)
{
    OfxPropertySetHandle paramProps;
    MFX_ENSURE(parameterSuite->paramDefine(m_parameters, kOfxParamTypeInteger, name, &paramProps));
    MFX_ENSURE(propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 0, defaultValue));
    return MfxParamDef<int>(host(), paramProps);
}

MfxParamDef<int2> MfxEffect::AddParam(const char *name, const int2 & defaultValue)
{
    OfxPropertySetHandle paramProps;
    MFX_ENSURE(parameterSuite->paramDefine(m_parameters, kOfxParamTypeInteger2D, name, &paramProps));
    MFX_ENSURE(propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 0, defaultValue[0]));
    MFX_ENSURE(propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 1, defaultValue[1]));
    return MfxParamDef<int2>(host(), paramProps);
}

MfxParamDef<int3> MfxEffect::AddParam(const char *name, const int3 & defaultValue)
{
    OfxPropertySetHandle paramProps;
    MFX_ENSURE(parameterSuite->paramDefine(m_parameters, kOfxParamTypeInteger3D, name, &paramProps));
    MFX_ENSURE(propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 0, defaultValue[0]));
    MFX_ENSURE(propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 1, defaultValue[1]));
    MFX_ENSURE(propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 2, defaultValue[2]));
    return MfxParamDef<int3>(host(), paramProps);
}

MfxParamDef<double> MfxEffect::AddParam(const char *name, double defaultValue)
{
    OfxPropertySetHandle paramProps;
    MFX_ENSURE(parameterSuite->paramDefine(m_parameters, kOfxParamTypeDouble, name, &paramProps));
    MFX_ENSURE(propertySuite->propSetDouble(paramProps, kOfxParamPropDefault, 0, defaultValue));
    return MfxParamDef<double>(host(), paramProps);
}

MfxParamDef<double2> MfxEffect::AddParam(const char *name, const double2 & defaultValue)
{
    OfxPropertySetHandle paramProps;
    MFX_ENSURE(parameterSuite->paramDefine(m_parameters, kOfxParamTypeDouble2D, name, &paramProps));
    MFX_ENSURE(propertySuite->propSetDouble(paramProps, kOfxParamPropDefault, 0, defaultValue[0]));
    MFX_ENSURE(propertySuite->propSetDouble(paramProps, kOfxParamPropDefault, 1, defaultValue[1]));
    return MfxParamDef<double2>(host(), paramProps);
}

MfxParamDef<double3> MfxEffect::AddParam(const char *name, const double3 & defaultValue)
{
    OfxPropertySetHandle paramProps;
    MFX_ENSURE(parameterSuite->paramDefine(m_parameters, kOfxParamTypeDouble3D, name, &paramProps));
    MFX_ENSURE(propertySuite->propSetDouble(paramProps, kOfxParamPropDefault, 0, defaultValue[0]));
    MFX_ENSURE(propertySuite->propSetDouble(paramProps, kOfxParamPropDefault, 1, defaultValue[1]));
    MFX_ENSURE(propertySuite->propSetDouble(paramProps, kOfxParamPropDefault, 2, defaultValue[2]));
    return MfxParamDef<double3>(host(), paramProps);
}

MfxParamDef<bool> MfxEffect::AddParam(const char* name, bool defaultValue)
{
    OfxPropertySetHandle paramProps;
    MFX_ENSURE(parameterSuite->paramDefine(m_parameters, kOfxParamTypeBoolean, name, &paramProps));
    MFX_ENSURE(propertySuite->propSetInt(paramProps, kOfxParamPropDefault, 0, defaultValue));
    return MfxParamDef<bool>(host(), paramProps);
}

//-----------------------------------------------------------------------------

MfxInput MfxEffect::GetInput(const char* name)
{
    OfxMeshInputHandle input;
    MFX_ENSURE(meshEffectSuite->inputGetHandle(m_instance, name, &input, NULL));
    return MfxInput(host(), input);
}

template <typename T>
MfxParam<T> MfxEffect::GetParam(const char* name)
{
    OfxParamHandle param;
    MFX_ENSURE(parameterSuite->paramGetHandle(m_parameters, name, &param, NULL));
    return MfxParam<T>(host(), param);
}

//-----------------------------------------------------------------------------

bool MfxEffect::CheckSuites()
{
    return (
        NULL != meshEffectSuite &&
        NULL != propertySuite &&
        NULL != parameterSuite
    );
}

void MfxEffect::SetupDescribe(OfxMeshEffectHandle descriptor)
{
    m_descriptor = descriptor;
    MFX_ENSURE(meshEffectSuite->getParamSet(descriptor, &m_parameters));
}

void MfxEffect::SetupCook(OfxMeshEffectHandle instance)
{
    m_instance = instance;
    MFX_ENSURE(meshEffectSuite->getParamSet(instance, &m_parameters));
}

void MfxEffect::SetupIsIdentity(OfxMeshEffectHandle instance)
{
    m_instance = instance;
    MFX_ENSURE(meshEffectSuite->getParamSet(instance, &m_parameters));
}

//-----------------------------------------------------------------------------

// Explicit instantiations
template MfxParam<int> MfxEffect::GetParam<int>(const char* name);
template MfxParam<int2> MfxEffect::GetParam<int2>(const char* name);
template MfxParam<int3> MfxEffect::GetParam<int3>(const char* name);
template MfxParam<double> MfxEffect::GetParam<double>(const char* name);
template MfxParam<double2> MfxEffect::GetParam<double2>(const char* name);
template MfxParam<double3> MfxEffect::GetParam<double3>(const char* name);
template MfxParam<bool> MfxEffect::GetParam<bool>(const char* name);
