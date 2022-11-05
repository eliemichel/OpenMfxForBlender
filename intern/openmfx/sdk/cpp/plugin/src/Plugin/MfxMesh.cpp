#include "MfxMesh.h"
#include "macros.h"

MfxMesh::MfxMesh(const MfxHost& host, OfxMeshHandle mesh, OfxPropertySetHandle properties)
	: MfxBase(host)
	, m_mesh(mesh)
	, m_properties(properties)
{}

//-----------------------------------------------------------------------------

bool MfxMesh::IsValid() const
{
	return NULL != m_mesh;
}

void MfxMesh::FetchProperties(MfxMeshProps& props)
{
	int noLooseEdge;

    MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshPropPointCount, 0, &props.pointCount));
	MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshPropCornerCount, 0, &props.cornerCount));
    MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshPropFaceCount, 0, &props.faceCount));
    MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshPropNoLooseEdge, 0, &noLooseEdge));
    MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshPropConstantFaceSize, 0, &props.constantFaceSize));
    MFX_ENSURE(propertySuite->propGetInt(m_properties, kOfxMeshPropAttributeCount, 0, &props.attributeCount));

    props.noLooseEdge = (bool)noLooseEdge;
}

MfxMesh MfxMesh::AllocateAndFetchIOMap(int output_points_count, int origin_points_pool_size) {
	OfxMeshHandle map;
	OfxPropertySetHandle meshProps;
	MFX_ENSURE(propertySuite->propGetPointer(m_properties, kOfxMeshPropIOMap, 0, (void**)&map));
	MFX_ENSURE(meshEffectSuite->meshGetPropertySet(map, &meshProps));

	MFX_ENSURE(propertySuite->propSetInt(meshProps, "OfxMeshPropIsAttributeMap", 0, 1));
	MFX_ENSURE(propertySuite->propSetInt(meshProps, "OfxMeshPropOutputPointsCount", 0, output_points_count));
	MFX_ENSURE(propertySuite->propSetInt(meshProps, "OfxMeshPropOriginPointsTotalPoolSize", 0, origin_points_pool_size));
	MFX_ENSURE(meshEffectSuite->meshAlloc(map));

	return MfxMesh(host(), map, meshProps);
}

void MfxMesh::FetchTransform(double** matrix)
{
	MFX_ENSURE(propertySuite->propGetPointer(m_properties, kOfxMeshPropTransformMatrix, 0, (void**)matrix));
}

MfxAttribute MfxMesh::GetAttributeByIndex(int index)
{
	OfxPropertySetHandle attribute;
	MFX_ENSURE(meshEffectSuite->meshGetAttributeByIndex(m_mesh, index, &attribute));
	return MfxAttribute(host(), attribute);
}

MfxAttribute MfxMesh::GetAttribute(MfxAttributeAttachment attachment, const char* name)
{
	const char* mfxAttachment = MfxAttribute::attributeAttachmentAsString(attachment);
	OfxPropertySetHandle attribute;
	MFX_ENSURE(meshEffectSuite->meshGetAttribute(m_mesh, mfxAttachment, name, &attribute));
	return MfxAttribute(host(), attribute);
}

MfxAttribute MfxMesh::GetPointAttribute(const char* name)
{
	return GetAttribute(MfxAttributeAttachment::Point, name);
}

MfxAttribute MfxMesh::GetCornerAttribute(const char* name)
{
	return GetAttribute(MfxAttributeAttachment::Corner, name);
}

MfxAttribute MfxMesh::GetFaceAttribute(const char* name)
{
	return GetAttribute(MfxAttributeAttachment::Face, name);
}

MfxAttribute MfxMesh::GetMeshAttribute(const char* name)
{
	return GetAttribute(MfxAttributeAttachment::Mesh, name);
}

bool MfxMesh::HasAttribute(MfxAttributeAttachment attachment, const char* name)
{
	const char* mfxAttachment = MfxAttribute::attributeAttachmentAsString(attachment);
    OfxPropertySetHandle attribute;
    OfxStatus status = this->host().meshEffectSuite->meshGetAttribute(m_mesh, mfxAttachment, name, &attribute);

    if (kOfxStatOK == status) {
        return true;
    } else if (kOfxStatErrBadIndex == status) {
        return false;
    } else {
        throw MfxSuiteException(status, "meshEffectSuite->meshGetAttribute(m_mesh, attachment, name, &attribute)");
    }
}

bool MfxMesh::HasPointAttribute(const char* name)
{
    return HasAttribute(MfxAttributeAttachment::Point, name);
}

bool MfxMesh::HasCornerAttribute(const char* name)
{
    return HasAttribute(MfxAttributeAttachment::Corner, name);
}

bool MfxMesh::HasFaceAttribute(const char* name)
{
    return HasAttribute(MfxAttributeAttachment::Face, name);
}

bool MfxMesh::HasMeshAttribute(const char* name)
{
    return HasAttribute(MfxAttributeAttachment::Mesh, name);
}

void MfxMesh::Release()
{
	MFX_ENSURE(meshEffectSuite->inputReleaseMesh(m_mesh));
}

//-----------------------------------------------------------------------------

MfxAttribute MfxMesh::AddAttribute(MfxAttributeAttachment attachment, const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic)
{
	const char* mfxAttachment = MfxAttribute::attributeAttachmentAsString(attachment);
	const char* mfxType = MfxAttribute::attributeTypeAsString(type);
	const char* mfxSemantic = MfxAttribute::attributeSemanticAsString(semantic);
	OfxPropertySetHandle attribute;
	MFX_ENSURE(meshEffectSuite->attributeDefine(m_mesh, mfxAttachment, name, componentCount, mfxType, mfxSemantic, &attribute));
	MFX_ENSURE(propertySuite->propSetInt(attribute, kOfxMeshAttribPropIsOwner, 0, 1));
	return MfxAttribute(host(), attribute);
}

MfxAttribute MfxMesh::AddPointAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic)
{
	return AddAttribute(MfxAttributeAttachment::Point, name, componentCount, type, semantic);
}

MfxAttribute MfxMesh::AddCornerAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic)
{
	return AddAttribute(MfxAttributeAttachment::Corner, name, componentCount, type, semantic);
}

MfxAttribute MfxMesh::AddFaceAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic)
{
	return AddAttribute(MfxAttributeAttachment::Face, name, componentCount, type, semantic);
}

MfxAttribute MfxMesh::AddMeshAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic)
{
	return AddAttribute(MfxAttributeAttachment::Mesh, name, componentCount, type, semantic);
}

void MfxMesh::Allocate(int pointCount, int cornerCount, int faceCount, bool noLooseEdge, int constantFaceSize)
{
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxMeshPropPointCount, 0, pointCount));
	MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxMeshPropCornerCount, 0, cornerCount));
    MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxMeshPropFaceCount, 0, faceCount));
    MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxMeshPropNoLooseEdge, 0, (int)noLooseEdge));
    MFX_ENSURE(propertySuite->propSetInt(m_properties, kOfxMeshPropConstantFaceSize, 0, constantFaceSize));
	MFX_ENSURE(meshEffectSuite->meshAlloc(m_mesh));
}

void MfxMesh::Allocate(const MfxMeshProps& props)
{
	Allocate(
		props.pointCount,
		props.cornerCount,
		props.faceCount,
		props.noLooseEdge,
		props.constantFaceSize
	);
}
