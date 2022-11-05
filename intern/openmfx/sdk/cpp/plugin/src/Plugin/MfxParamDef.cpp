#include "MfxParamDef.h"
#include "macros.h"

template <typename T>
MfxParamDef<T>::MfxParamDef(const MfxHost& host, OfxPropertySetHandle properties)
	: MfxBase(host)
	, m_properties(properties)
{}

//-----------------------------------------------------------------------------

template <typename T>
MfxParamDef<T> & MfxParamDef<T>::Label(const char *label)
{
	MFX_ENSURE(propertySuite->propSetString(m_properties, kOfxPropLabel, 0, label));
	return *this;
}

template <>
MfxParamDef<int> & MfxParamDef<int>::Range(const int & minimum, const int & maximum)
{
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMin, 0, minimum));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMax, 0, maximum));
	return *this;
}

template <>
MfxParamDef<int2> & MfxParamDef<int2>::Range(const int2 & minimum, const int2 & maximum)
{
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMin, 0, minimum[0]));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMin, 1, minimum[1]));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMax, 0, maximum[0]));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMax, 1, maximum[1]));
	return *this;
}

template <>
MfxParamDef<int3> & MfxParamDef<int3>::Range(const int3 & minimum, const int3 & maximum)
{
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMin, 0, minimum[0]));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMin, 1, minimum[1]));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMin, 2, minimum[2]));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMax, 0, maximum[0]));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMax, 1, maximum[1]));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMax, 2, maximum[2]));
	return *this;
}

template <>
MfxParamDef<double> & MfxParamDef<double>::Range(const double & minimum, const double & maximum)
{
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMin, 0, minimum));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMax, 0, maximum));
	return *this;
}

template <>
MfxParamDef<double2> & MfxParamDef<double2>::Range(const double2 & minimum, const double2 & maximum)
{
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMin, 0, minimum[0]));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMin, 1, minimum[1]));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMax, 0, maximum[0]));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMax, 1, maximum[1]));
	return *this;
}

template <>
MfxParamDef<double3> & MfxParamDef<double3>::Range(const double3 & minimum, const double3 & maximum)
{
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMin, 0, minimum[0]));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMin, 1, minimum[1]));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMin, 2, minimum[2]));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMax, 0, maximum[0]));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMax, 1, maximum[1]));
	MFX_ENSURE(propertySuite->propSetDouble(m_properties, kOfxParamPropMax, 2, maximum[2]));
	return *this;
}

template <>
MfxParamDef<bool> & MfxParamDef<bool>::Range(const bool & minimum, const bool & maximum)
{
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMin, 0, minimum));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxParamPropMax, 0, maximum));
	return *this;
}

//-----------------------------------------------------------------------------

// Explicit instantiations
template class MfxParamDef<int>;
template class MfxParamDef<int2>;
template class MfxParamDef<int3>;
template class MfxParamDef<double>;
template class MfxParamDef<double2>;
template class MfxParamDef<double3>;
template class MfxParamDef<bool>;
