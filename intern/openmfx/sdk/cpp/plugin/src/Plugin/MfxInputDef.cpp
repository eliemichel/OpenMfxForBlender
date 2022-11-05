#include "MfxInputDef.h"
#include "MfxAttribute.h"
#include "macros.h"

MfxInputDef::MfxInputDef(const MfxHost& host, OfxMeshInputHandle input, OfxPropertySetHandle properties)
	: MfxBase(host)
	, m_input(input)
	, m_properties(properties)
{}

//-----------------------------------------------------------------------------

MfxInputDef & MfxInputDef::Label(const char *label)
{
	MFX_ENSURE(propertySuite->propSetString(m_properties, kOfxPropLabel, 0, label));
	return *this;
}

MfxInputDef& MfxInputDef::RequestAttribute(MfxAttributeAttachment attachment, const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory)
{
	const char* mfxAttachment = MfxAttribute::attributeAttachmentAsString(attachment);
	const char* mfxType = MfxAttribute::attributeTypeAsString(type);
	const char* mfxSemantic = MfxAttribute::attributeSemanticAsString(semantic);
	MFX_ENSURE(meshEffectSuite->inputRequestAttribute(m_input, mfxAttachment, name, componentCount, mfxType, mfxSemantic, mandatory));
	return *this;
}

MfxInputDef& MfxInputDef::RequestPointAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory)
{
	return RequestAttribute(MfxAttributeAttachment::Point, name, componentCount, type, semantic, mandatory);
}

MfxInputDef& MfxInputDef::RequestCornerAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory)
{
	return RequestAttribute(MfxAttributeAttachment::Corner, name, componentCount, type, semantic, mandatory);
}

MfxInputDef& MfxInputDef::RequestFaceAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory)
{
	return RequestAttribute(MfxAttributeAttachment::Face, name, componentCount, type, semantic, mandatory);
}

MfxInputDef& MfxInputDef::RequestMeshAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory)
{
	return RequestAttribute(MfxAttributeAttachment::Mesh, name, componentCount, type, semantic, mandatory);
}

MfxInputDef& MfxInputDef::RequestIOMap(bool request)
{
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxInputPropRequestIOMap, 0, request ? 1 : 0));
	return *this;
}

MfxInputDef& MfxInputDef::RequestGeometry(bool request)
{
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxInputPropRequestGeometry, 0, request ? 1 : 0));
	return *this;
}

MfxInputDef& MfxInputDef::RequestTransform(bool request)
{
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxInputPropRequestTransform, 0, request ? 1 : 0));
	return *this;
}

