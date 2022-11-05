#pragma once

#include "MfxBase.h"
#include "MfxAttribute.h"
#include "MfxMeshProps.h"

#include "ofxCore.h"
#include "ofxMeshEffect.h"

/**
 * Mesh data coming from an input or being sent to an output.
 * The instance of this class -- like all classes from this API except the
 * structs whose name ends with "Props" -- does not actually contain data,
 * it is only a reference and can hence be freely copied around without
 * worrying about memory usage. You MUST call \ref Release once and only once
 * though.
 */
class MfxMesh : public MfxBase
{
private:
	friend class MfxInput;
	MfxMesh(const MfxHost& host, OfxMeshHandle mesh, OfxPropertySetHandle properties);

public:
	/**
	 * A mesh may be invalid if its parent input is not connected to any incoming link.
	 * If this returns false, none of the other methods must be called.
	 */
	bool IsValid() const;

	/**
	 * Populate the provided props structure with this mesh's properties
	 */
	void FetchProperties(MfxMeshProps& props);

	/**
	 * Get the attribute map
	 */
	MfxMesh AllocateAndFetchIOMap(int output_points_count, int origin_points_pool_size);

	/**
	 * Populate the provided pointer to float array with this mesh's transform matrix.
	 * Requires to have called RequestTransform during the describe action
	 */
	void FetchTransform(double**matrix);

	/**
	 * Get an attribute of an input or output mesh by index.
	 * The returned \ref MfxAttribute can be used to get the data buffer and
	 * extra information like type, component count, stride, etc.
	 * If the index is out of range, this throws a \ref MfxSuiteException.
	 */
	MfxAttribute GetAttributeByIndex(int index);
		
	/**
	 * Get an attribute of an input or output mesh by attachment and name.
	 * The returned \ref MfxAttribute can be used to get the data buffer and
	 * extra information like type, component count, stride, etc.
	 * `Get*Attribute` methods are shortcuts for the different values allowed
	 * for the `attachment` argument.
	 * If the queried attribute does not exist, this throws a
	 * \ref MfxSuiteException, you can use \ref HasAttribute to avoid throwing
	 * this exception.
	 */
	MfxAttribute GetAttribute(MfxAttributeAttachment attachment, const char* name);

	/**
	 * Specialization of \ref GetAttribute for point attributes.
	 */
	MfxAttribute GetPointAttribute(const char* name);

	/**
	 * Specialization of \ref GetAttribute for corner attributes.
	 */
	MfxAttribute GetCornerAttribute(const char* name);

	/**
	 * Specialization of \ref GetAttribute for face attributes.
	 */
	MfxAttribute GetFaceAttribute(const char* name);

	/**
	 * Specialization of \ref GetAttribute for global mesh attributes.
	 */
	MfxAttribute GetMeshAttribute(const char* name);

	/**
	 * Tells whether an attribute exists in the input or output mesh.
	 * `Has*Attribute` methods are shortcuts for the different values allowed
	 * for the `attachment` argument.
	 */
    bool HasAttribute(MfxAttributeAttachment attachment, const char* name);

    /**
	 * Specialization of \ref HasAttribute for point attributes.
	 */
    bool HasPointAttribute(const char* name);

    /**
	 * Specialization of \ref HasAttribute for corner attributes.
	 */
    bool HasCornerAttribute(const char* name);

    /**
	 * Specialization of \ref HasAttribute for face attributes.
	 */
    bool HasFaceAttribute(const char* name);

    /**
	 * Specialization of \ref HasAttribute for global mesh attributes.
	 */
    bool HasMeshAttribute(const char* name);

	/**
	 * Always call this at some point, don't use the object afterwise.
	 * (This is not in the dtor because there may be copies of the instance but
	 * Release must be called only once)
	 */
	void Release();

public:
	/**
	 * Define a new attribute on an output mesh.
	 * The returned \ref MfxAttribute can be used to set additionnal properties
	 * such has whether the data is "owned" or borrowed/forwarded from an input.
	 * `Add*Attribute` methods are shortcuts for the different values allowed
	 * for the `attachment` argument.
	 *
	 * \see attributeDefine from ofxMeshEffect.h for more information.
	 *
	 * Call **only for output** meshes and before allocation.
	 */
	MfxAttribute AddAttribute(
		MfxAttributeAttachment attachment,
		const char* name,
		int componentCount,
		MfxAttributeType type,
		MfxAttributeSemantic semantic = MfxAttributeSemantic::None
	);

	/**
	 * Specialization of \ref AddAttribute for point attributes.
	 */
	MfxAttribute AddPointAttribute(
		const char* name,
		int componentCount,
		MfxAttributeType type,
		MfxAttributeSemantic semantic = MfxAttributeSemantic::None
	);

	/**
	 * Specialization of \ref AddAttribute for corner attributes.
	 */
	MfxAttribute AddCornerAttribute(
		const char* name,
		int componentCount,
		MfxAttributeType type,
		MfxAttributeSemantic semantic = MfxAttributeSemantic::None
	);

	/**
	 * Specialization of \ref AddAttribute for face attributes.
	 */
	MfxAttribute AddFaceAttribute(
		const char* name,
		int componentCount,
		MfxAttributeType type,
		MfxAttributeSemantic semantic = MfxAttributeSemantic::None
	);

	/**
	 * Specialization of \ref AddAttribute for global mesh attributes.
	 */
	MfxAttribute AddMeshAttribute(
		const char* name,
		int componentCount,
		MfxAttributeType type,
		MfxAttributeSemantic semantic = MfxAttributeSemantic::None
	);

	/**
	 * Allocate memory for new owned attributes according to the previously
	 * called \ref AddAttribute and likes. Non own attributes are attributes
	 * pointing to externally allocated memory (for instance memory buffers
	 * forwarded from the input) and will hence not be newly allocate.
	 * Call **only for output** meshes.
	 */
    void Allocate(
    	int pointCount,
    	int cornerCount,
    	int faceCount,
    	bool noLooseEdge=true,
    	int constantFaceSize=-1
    );

	/**
	 * Alternatively, Allocate() can be called with a MfxMeshProps struct, which
	 * contains the same counters.
	 */
	void Allocate(const MfxMeshProps& props);

private:
	OfxMeshHandle m_mesh;
	OfxPropertySetHandle m_properties;
};
