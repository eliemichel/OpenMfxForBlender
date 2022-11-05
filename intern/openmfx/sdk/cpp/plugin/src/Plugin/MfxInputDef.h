#pragma once

#include "MfxBase.h"
#include "MfxAttributeProps.h"

#include "ofxCore.h"
#include "ofxMeshEffect.h"

/**
 * An input definition is manipulated during the \ref MfxEffect::Describe
 * action to tell the host what is expected to flow through the input at
 * cook time. It differs from \ref MfxInput that is the input instance during
 * the cooking, in particular the input definition does not contain any mesh.
 */
class MfxInputDef : public MfxBase
{
private:
	friend class MfxEffect;
	MfxInputDef(const MfxHost & host, OfxMeshInputHandle input, OfxPropertySetHandle properties);

public:
	/**
	 * Define the human readable label to display next to this input
	 * (in hosts supporting it).
	 */
	MfxInputDef & Label(const char *label);

	/**
	 * Notify the host that the effect requires (if \e mandatory is true)
	 * or would make good use of a given attribute. If the attribute is
	 * mandatory, it is ensured to be present with at least \e componentCount
	 * components of the requested type. If not mandatory, it is the
	 * responsibility of this effect to check at cook time whether the attribute
	 * exist or not and whether it has enough components and the right type.
	 * 
	 * The semantics can be used to tell the intended use of the of the attribute
	 * to help the host suggest some attributes to send to the effect to the user.
	 * Possible values are \ref kOfxMeshAttribSemanticTextureCoordinate,
	 * \ref kOfxMeshAttribSemanticNormal, \ref kOfxMeshAttribSemanticColor
	 * or \ref kOfxMeshAttribSemanticWeight. It may also just be null.
	 */
	MfxInputDef & RequestAttribute(MfxAttributeAttachment attachment, const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory);

	MfxInputDef & RequestPointAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory);
	MfxInputDef & RequestCornerAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory);
	MfxInputDef & RequestFaceAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory);
	MfxInputDef & RequestMeshAttribute(const char* name, int componentCount, MfxAttributeType type, MfxAttributeSemantic semantic, bool mandatory);

	MfxInputDef& RequestIOMap(bool request = true);

	/**
	 * Set the geometry matrix dependency flag of this input.
	 * By default, an input does depend on its geometry, but this may be turned off
	 * when the input is used solely for its transform (which must be signaled with
	 * RequestTransform).
	 */
	MfxInputDef& RequestGeometry(bool request = true);

	/**
	 * Set the transform matrix dependency flag of this input.
	 * By default, an input does not depend on the transform matrix and hence
	 * the effect will not have access to the transform matrix, and not be recooked
	 * when the transform changes.
	 */
	MfxInputDef& RequestTransform(bool request = true);

private:
	OfxMeshInputHandle m_input;
	OfxPropertySetHandle m_properties;
};

