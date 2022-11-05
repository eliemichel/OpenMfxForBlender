#include "MfxInput.h"
#include "macros.h"

MfxInput::MfxInput(const MfxHost& host, OfxMeshInputHandle input)
	: MfxBase(host)
	, m_input(input)
{}

//-----------------------------------------------------------------------------

MfxMesh MfxInput::GetMesh()
{
	OfxTime time = 0;
	OfxMeshHandle mesh;
	OfxPropertySetHandle meshProps;
	OfxStatus status;
	status = host().meshEffectSuite->inputGetMesh(m_input, time, &mesh, &meshProps);
	if (kOfxStatOK != status) {
		mesh = NULL;
	}
	return MfxMesh(host(), mesh, meshProps);
}
