#pragma once

#include "MfxBase.h"
#include "MfxAttributeProps.h"

#include "ofxCore.h"
#include "ofxMeshEffect.h"

/**
 * An attribute is part of a mesh, it is an information attached to either each
 * point, each corner, each face or only once for the whole mesh.
 * See \ref MfxMesh::GetAttribute to get attribute objects.
 */
struct MfxAttribute : public MfxBase
{
private:
	friend class MfxMesh;
	MfxAttribute(const MfxHost& host, OfxPropertySetHandle properties);

	/**
	 * Copy attribute and try to cast. If number of component is different,
	 * copy the common components only.
	 */
	static OfxStatus copyAttributeData(MfxAttributeProps & destination, const MfxAttributeProps & source, int start, int count);

public:
	/**
	 * Populate the provided props structure with this attribute's properties
	 */
	void FetchProperties(MfxAttributeProps & props) const;

    /**
     * Set attribute properties according to provided props structure
     */
    void SetProperties(const MfxAttributeProps & props);

    /**
     * Copy attribute data, casting if necessary
     */
	void CopyFrom(const MfxAttribute& other, int start, int count);

    /**
     * Forward attribute data, pointing to existing buffers instead of copying.
     * Note that the buffer in source attribute must already be allocated.
     */
    void ForwardFrom(const MfxAttribute& other);

public:
	/**
	 * Convert a type string from MeshEffect API to its local enum counterpart
	 */
	static MfxAttributeType attributeTypeAsEnum(const char* mfxType);

	/**
	 * Convert local typestring enum to a type string from MeshEffect API
	 */
	static const char* attributeTypeAsString(MfxAttributeType type);

	/**
	 * Convert an attachment string from MeshEffect API to its local enum counterpart
	 */
	static MfxAttributeAttachment attributeAttachmentAsEnum(const char* mfxAttachment);

	/**
	 * Convert local attachment enum to an attachment string from MeshEffect API
	 */
	static const char* attributeAttachmentAsString(MfxAttributeAttachment attachment);

	/**
	 * Convert a semantic string from MeshEffect API to its local enum counterpart
	 */
	static MfxAttributeSemantic attributeSemanticAsEnum(const char* mfxSemantic);

	/**
	 * Convert local semantic enum to a semantic string from MeshEffect API
	 */
	static const char* attributeSemanticAsString(MfxAttributeSemantic semantic);


private:
	OfxPropertySetHandle m_properties;
};
