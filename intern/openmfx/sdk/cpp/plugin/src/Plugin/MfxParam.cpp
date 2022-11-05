#include "MfxParam.h"
#include "macros.h"

template <typename T>
MfxParam<T>::MfxParam(const MfxHost& host, OfxParamHandle param)
	: MfxBase(host)
	, m_param(param)
{}

//-----------------------------------------------------------------------------

template<>
int MfxParam<int>::GetValue()
{
	int value = 0;
	MFX_ENSURE(parameterSuite->paramGetValue(m_param, &value));
	return value;
}

template<>
int2 MfxParam<int2>::GetValue()
{
	int2 value = { 0, 0 };
	MFX_ENSURE(parameterSuite->paramGetValue(m_param, &value[0], &value[1]));
	return value;
}

template<>
int3 MfxParam<int3>::GetValue()
{
	int3 value = { 0, 0, 0 };
	MFX_ENSURE(parameterSuite->paramGetValue(m_param, &value[0], &value[1], &value[2]));
	return value;
}

template<>
double MfxParam<double>::GetValue()
{
	double value = 0;
	MFX_ENSURE(parameterSuite->paramGetValue(m_param, &value));
	return value;
}

template<>
double2 MfxParam<double2>::GetValue()
{
	double2 value = { 0, 0 };
	MFX_ENSURE(parameterSuite->paramGetValue(m_param, &value[0], &value[1]));
	return value;
}

template<>
double3 MfxParam<double3>::GetValue()
{
	double3 value = { 0, 0, 0 };
	MFX_ENSURE(parameterSuite->paramGetValue(m_param, &value[0], &value[1], &value[2]));
	return value;
}

template<>
bool MfxParam<bool>::GetValue()
{
	bool value = false;
	MFX_ENSURE(parameterSuite->paramGetValue(m_param, &value));
	return value;
}

//-----------------------------------------------------------------------------

// Explicit instantiations
template class MfxParam<int>;
template class MfxParam<int2>;
template class MfxParam<int3>;
template class MfxParam<double>;
template class MfxParam<double2>;
template class MfxParam<double3>;
template class MfxParam<bool>;
