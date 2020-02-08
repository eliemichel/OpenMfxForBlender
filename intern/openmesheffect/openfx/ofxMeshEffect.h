#ifndef _ofxMeshEffect_h_
#define _ofxMeshEffect_h_

/*
Software License :

Copyright (c) 2019 - 2020, Elie Michel. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name The Open Effects Association Ltd, nor the names of its 
      contributors may be used to endorse or promote products derived from this
      software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "ofxCore.h"
#include "ofxParam.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @file ofxMeshEffect.h
Contains the part of the API that relates to mesh processing. This is what is called "Open Mesh Effect". For more details on the basic OFX architecture, see \ref Architecture.
*/

/** @brief String used to label OFX Mesh Effect Plug-ins

    Set the pluginApi member of the OfxPluginHeader inside any OfxMeshEffectPluginStruct
    to be this so that the host knows the plugin is an image effect.
 */
#define kOfxMeshEffectPluginApi "OfxMeshEffectPluginAPI"

/** @brief The current version of the Image Effect API
 */
#define kOfxMeshEffectPluginApiVersion 1

/** @brief Blind declaration of an OFX mesh effect
*/
typedef struct OfxMeshEffectStruct *OfxMeshEffectHandle;

/** @brief Blind declaration of an OFX mesh effect input
*/
typedef struct OfxMeshInputStruct *OfxMeshInputHandle;

/** @brief Blind declaration of an OFX geometry data
*/
typedef struct OfxMeshStruct *OfxMeshHandle;

/** @brief Use to define the generator mesh effect context See \ref ::kOfxMeshEffectPropContext */
#define kOfxMeshEffectContextGenerator "OfxMeshEffectContextGenerator"

/** @brief Use to define the filter mesh effect context See \ref ::kOfxMeshEffectPropContext */
#define kOfxMeshEffectContextFilter "OfxMeshEffectContextFilter"

/** @brief Use to define the general mesh effect context See \ref ::kOfxMeshEffectPropContext */
#define kOfxMeshEffectContextGeneral "OfxMeshEffectContextGeneral"

/** @brief Identifier of the main mesh input of a mesh effect
 */
#define kOfxMeshMainInput "OfxMeshMainInput"

/** @brief Identifier of the main mesh output of a mesh effect
 */
#define kOfxMeshMainOutput "OfxMeshMainOutput"

/**
   \defgroup MeshAttrib attachements

Mesh attributes can be attached to either points, vertices, faces or the whole mesh. Mandatory
attributes include a position for points, a point index for vertices and a vertex count for faces.
*/
/*@{*/
/** @brief Mesh attribute attachement to points
 */
#define kOfxMeshAttribPoint "OfxMeshAttribPoint"

/** @brief Mesh attribute attachement to vertex
 */
#define kOfxMeshAttribVertex "OfxMeshAttribVertex"

/** @brief Mesh attribute attachement to faces
 */
#define kOfxMeshAttribFace "OfxMeshAttribFace"

/** @brief Mesh attribute attachement to the whole mesh
 */
#define kOfxMeshAttribMesh "OfxMeshAttribMesh"

/** @brief Name of the point attribut for position
 */
#define kOfxMeshAttribPointPosition "OfxMeshAttribPointPosition"

/** @brief Name of the vertex attribut for point index
 */
#define kOfxMeshAttribVertexPoint "OfxMeshAttribVertexPoint"

/** @brief Name of the face attribut for vertex count
 */
#define kOfxMeshAttribFaceCounts "OfxMeshAttribFaceCounts"

/** @brief Attribute type integer 32 bit
 */
#define kOfxMeshAttribTypeInt "OfxMeshAttribTypeInt"

/** @brief Attribute type float 32 bit
 */
#define kOfxMeshAttribTypeFloat "OfxMeshAttribTypeFloat"

/*@}*/

/**
   \addtogroup ActionsAll
*/
/*@{*/
/**
   \defgroup MeshEffectActions Mesh Effect Actions

These are the list of actions passed to a mesh effect plugin's main function. For more details on how to deal with actions, see \ref MeshEffectActions.
*/
/*@{*/

/** @brief

 Sometimes an effect can pass through an input uprocessed, for example a
 displace effect with a displace amplitude of 0. This action can be called by a
 host before it attempts to cook an effect to determine if it can simply
 copy input directly to output without having to call the render action
 on the effect.

 If the effect does not need to process any geometry, it should set the
 value of the \ref kOfxPropName to the input that the host should us as the
 output instead, and the \ref kOfxPropTime property on ``outArgs`` to be
 the time at which the frame should be fetched from a clip.

 The default action is to call the render action on the effect.


 @param  handle handle to the instance, cast to an \ref OfxMeshEffectHandle
 @param  inArgs has the following properties
     - \ref kOfxPropTime the time at which to test for identity

 @param  outArgs has the following properties which the plugin can set
     - \ref kOfxPropName
     this to the name of the input that should be used if the effect is
     an identity transform, defaults to the empty string
     - \ref kOfxPropTime
     the time to use from the indicated source input as an identity
     mesh (allowing time slips to happen), defaults to the value in
     \ref kOfxPropTime in inArgs



 @returns
     -  \ref kOfxStatOK, the action was trapped and the effect should not have its
     render action called, the values in outArgs
     indicate what frame from which input to use instead
     -  \ref kOfxStatReplyDefault, the action was not trapped and the host should
     call the cook action
     -  \ref kOfxStatErrMemory, in which case the action may be called again after
     a memory purge
     -  \ref kOfxStatFailed, something wrong, but no error code appropriate,
     plugin to post message
     -  \ref kOfxStatErrFatal

 */
#define kOfxMeshEffectActionIsIdentity            "OfxMeshEffectActionIsIdentity"

/** @brief

 This action is where an effect gets to compute geometry and turn its input
 mesh and parameter set into an output mesh. This is possibly quite
 complicated and covered in the \ref CookingEffects "Cooking Mesh Effects" chapter.

 The cook action *must* be trapped by the plug-in, it cannot return
 \ref kOfxStatReplyDefault.

 @param  handle handle to the instance, cast to an \ref OfxMeshEffectHandle
 @param  inArgs has the following properties
     -  \ref kOfxPropTime the time at which to cook

 @param  outArgs is redundant and should be set to NULL

\pre
     -  \ref kOfxActionCreateInstance has been called on the instance
     
 @returns
     -  \ref kOfxStatOK, the effect cooked normaly
     -  \ref kOfxStatErrMemory, in which case the action may be called again after
     a memory purge
     -  \ref kOfxStatFailed, something wrong, but no error code appropriate,
     plugin to post message
     -  \ref kOfxStatErrFatal

 */
#define kOfxMeshEffectActionCook                "OfxMeshEffectActionCook"

/** @brief

 This action is unique to OFX Mesh Effect plug-ins. Because a plugin is
 able to exhibit different behaviour depending on the context of use,
 each separate context will need to be described individually. It is
 within this action that mesh effects describe which parameters and
 input geometry it requires.

 This action will be called multiple times, one for each of the contexts
 the plugin says it is capable of implementing. If a host does not
 support a certain context, then it need not call
 \ref kOfxMeshEffectActionDescribeInContext for that context.

 This action *must* be trapped, it is not optional.

 @param  handle handle to the context descriptor, cast to an \ref OfxMeshEffectHandle
 this may or may not be the same as passed to \ref kOfxActionDescribe

 @param  inArgs has the following property:
     - \ref kOfxMeshEffectPropContext the context being described

 @param  outArgs is redundant and is set to NULL

\pre
     - \ref kOfxActionDescribe has been called on the descriptor handle,
     - \ref kOfxActionCreateInstance has not been called

 @returns
     -  \ref kOfxStatOK, the action was trapped and all was well
     -  \ref kOfxStatErrMissingHostFeature, in which the context will be ignored
     by the host, the plugin may post a message
     -  \ref kOfxStatErrMemory, in which case the action may be called again after
     a memory purge
     -  \ref kOfxStatFailed, something wrong, but no error code appropriate,
     plugin to post message
     -  \ref kOfxStatErrFatal

 */
#define kOfxMeshEffectActionDescribeInContext     "OfxMeshEffectActionDescribeInContext"

/*@}*/
/*@}*/

/**
   \addtogroup PropertiesAll
*/
/*@{*/
/**
   \defgroup MeshEffectPropDefines Mesh Effect Property Definitions 

These are the list of properties used by the Mesh Effects API.
*/
/*@{*/
/** @brief Indicates to the host the contexts a plugin can be used in.

   - Type - string X N
   - Property Set - mesh effect descriptor passed to kOfxActionDescribe (read/write)
   - Default - this has no defaults, it must be set
   - Valid Values - This must be one of
      - ::kOfxMeshEffectContextGenerator
      - ::kOfxMeshEffectContextFilter
      - ::kOfxMeshEffectContextGeneral
*/
#define kOfxMeshEffectPropSupportedContexts "OfxMeshEffectPropSupportedContexts"

/** @brief The plugin handle passed to the initial 'describe' action.

   - Type - pointer X 1
   - Property Set - plugin instance, (read only)

This value will be the same for all instances of a plugin.
*/
#define kOfxMeshEffectPropPluginHandle "OfxMeshEffectPropPluginHandle"

/** @brief Indicates the context a plugin instance has been created for.

   - Type - string X 1
   - Property Set - image effect instance (read only)
   - Valid Values - This must be one of
      - ::kOfxMeshEffectContextGenerator
      - ::kOfxMeshEffectContextFilter
      - ::kOfxMeshEffectContextGeneral

 */
#define kOfxMeshEffectPropContext "OfxMeshEffectPropContext"

/** @brief The number of points in a mesh

    - Type - integer X 1
    - Property Set - a mesh instance (read only)

This property is the number of points allocated in the mesh object.
 */
#define kOfxMeshPropPointCount "OfxMeshPropPointCount"

/** @brief The number of vertices in a mesh

    - Type - integer X 1
    - Property Set - a mesh instance (read only)

This property is the number of vertices allocated in the mesh object.
 */
#define kOfxMeshPropVertexCount "OfxMeshPropVertexCount"

/** @brief The number of faces in a mesh

    - Type - integer X 1
    - Property Set - a mesh instance (read only)

This property is the number of faces allocated in the mesh object.
 */
#define kOfxMeshPropFaceCount "OfxMeshPropFaceCount"

/** @brief The number of attribtues in a mesh

    - Type - integer X 1
    - Property Set - a mesh instance (read only)

This property is the number of attribtues stored in the mesh object. Attributes can be attached to
either points, vertices, faces or the whole mesh. There are at least three attributes in a geometry
namely the point's position, the vertex' point association and the face's vertex count.
 */
#define kOfxMeshPropAttributeCount "OfxMeshPropAttributeCount"

/**  @brief The data pointer of an attribute.

    - Type - pointer X 1
    - Property Set - a mesh instance (read only)

This property contains a pointer to memory where attribute data is stored, whose size depend on the
attribute attachement (point/vertex/face/mesh) and attribute type (int, float, vector, etc.)
*/
#define kOfxMeshAttribPropData "OfxMeshAttribPropData"

/**  @brief The number of components an attribute.

    - Type - int X 1
    - Property Set - a mesh attribute (read only)

An attribute can have between 1 and 4 components.
*/
#define kOfxMeshAttribPropComponentCount "OfxMeshAttribPropComponentCount"

/**  @brief The type of an attribute.

    - Type - string X 1
    - Property Set - a mesh attribute (read only)

An attribute can have type kOfxMeshAttribTypeFloat or kOfxMeshAttribTypeInt
*/
#define kOfxMeshAttribPropType "OfxMeshAttribPropType"

/*@}*/
/*@}*/


/** @brief Tells the binary the path to the bundle from which it is loaded.
 *
 * This is useful for the binary to locate the resource directory, wose content
 * my impact the number of available plugins. A host may not call this, but if
 * it does, it must be only once and before calling OfxGetNumberOfPlugins().
 *
 * A binary may not implement this symbol, because core OpenFX does not
 * introduce it.
 */
OfxExport void OfxSetBundleDirectory(const char *path);

/** @brief the string that names mesh effect suites, passed to OfxHost::fetchSuite */
#define kOfxMeshEffectSuite "OfxMeshEffectSuite"

/** @brief The OFX suite for mesh effects

This suite provides the functions needed by a plugin to defined and use a mesh effect plugin.
 */
typedef struct OfxMeshEffectSuiteV1 {
  /** @brief Retrieves the property set for the given mesh effect

  \arg meshEffect    mesh effect to get the property set for
  \arg propHandle    pointer to a the property set pointer, value is returned here

  The property handle is for the duration of the mesh effect handle.

  @returns
  - ::kOfxStatOK       - the property set was found and returned
  - ::kOfxStatErrBadHandle  - if the paramter handle was invalid
  - ::kOfxStatErrUnknown    - if the type is unknown
  */
  OfxStatus (*getPropertySet)(OfxMeshEffectHandle meshEffect,
            OfxPropertySetHandle *propHandle);
  
  /** @brief Retrieves the parameter set for the given mesh effect

  \arg meshEffect   mesh effect to get the property set for
  \arg paramSet     pointer to a the parameter set, value is returned here

  The param set handle is valid for the lifetime of the mesh effect handle.

  @returns
  - ::kOfxStatOK       - the property set was found and returned
  - ::kOfxStatErrBadHandle  - if the paramter handle was invalid
  - ::kOfxStatErrUnknown    - if the type is unknown
  */
  OfxStatus (*getParamSet)(OfxMeshEffectHandle meshEffect,
         OfxParamSetHandle *paramSet);

  /** @brief Define an input to the effect. 
      
   \arg pluginHandle - the handle passed into 'describeInContext' action
   \arg name - unique name of the input to define
   \arg propertySet - a property handle for the input descriptor will be returned here

   This function defines an input to a host, the returned property set is used to describe
   various aspects of the input to the host. Note that this does not create an input instance.
   
\pre
 - we are inside the describe in context action.

  @returns
  */
  OfxStatus (*inputDefine)(OfxMeshEffectHandle meshEffect,
        const char *name,  
        OfxPropertySetHandle *propertySet);

  /** @brief Get the propery handle of the named geometry input in the given instance 
   
   \arg meshEffect - an instance handle to the plugin
   \arg name        - name of the input, previously used in an input define call
   \arg input        - where to return the input
   \arg propertySet  if not null, the descriptor handle for a parameter's property set will be placed here.

  The propertySet will have the same value as would be returned by OfxMeshEffectSuiteV1::inputGetPropertySet

      This return a input handle for the given instance, note that this will \em not be the same as the
      input handle returned by inputDefine and will be distinct to input handles in any other instance
      of the plugin.

      Not a valid call in any of the describe actions.

\pre
 - create instance action called,
 - \e name passed to inputDefine for this context,
 - not inside describe or describe in context actions.
 
\post
 - handle will be valid for the life time of the instance.

  */
  OfxStatus (*inputGetHandle)(OfxMeshEffectHandle meshEffect,
           const char *name,
           OfxMeshInputHandle *input,
           OfxPropertySetHandle *propertySet);

  /** @brief Retrieves the property set for a given input

  \arg input         input effect to get the property set for
  \arg propHandle    pointer to a the property set handle, value is returned here

  The property handle is valid for the lifetime of the input, which is generally the lifetime of the instance.

  @returns
  - ::kOfxStatOK            - the property set was found and returned
  - ::kOfxStatErrBadHandle  - if the input handle was invalid
  - ::kOfxStatErrUnknown    - if the type is unknown
  */
  OfxStatus (*inputGetPropertySet)(OfxMeshInputHandle input,
          OfxPropertySetHandle *propHandle);

  /** @brief Get a handle for a mesh in an input at the indicated time

      \arg input       - the input to extract the mesh from
      \arg time        - time to fetch the mesh at
      \arg meshHandle  - mesh containing the mesh's data
      \arg propertySet - property set containing the mesh properties (may be NULL)

  A mesh is fetched from an input at the indicated time and returned in the meshHandle.

If inputGetMesh is called twice with the same parameters, then two separate mesh handles will be returned, each of which must be release. The underlying implementation could share mesh data pointers and use reference counting to maintain them.

\pre
 - input was returned by inputGetHandle

\post
 - mesh handle is only valid for the duration of the action inputGetMesh is called in
 - mesh handle to be disposed of by inputReleaseMesh before the action returns

@returns
- ::kOfxStatOK     - the mesh was successfully fetched and returned in the handle,
- ::kOfxStatFailed - the mesh could not be fetched because it does not exist in the input at the indicated time, the plugin
                     should continue operation, but assume the mesh was empty.
- ::kOfxStatErrBadHandle - the input handle was invalid,
- ::kOfxStatErrMemory - the host had not enough memory to complete the operation, plugin should abort whatever it was doing.

  */
  OfxStatus (*inputGetMesh)(OfxMeshInputHandle input,
                            OfxTime time,
                            OfxMeshHandle *meshHandle,
                            OfxPropertySetHandle *propertySet);

  /** @brief Releases the mesh handle previously returned by inputGetMesh

\pre
 - meshHandle was returned by inputGetMesh

\post
 - all operations on meshHandle will be invalid

@returns
- ::kOfxStatOK - the mesh was successfully released,
- ::kOfxStatErrBadHandle - the mesh handle was invalid,
 */
  OfxStatus (*inputReleaseMesh)(OfxMeshHandle meshHandle);

  /** @brief Ensure that an attribute is attached to a mesh

      \arg meshHandle       - mesh handle
      \arg attachment       - attribute attachment (see \ref MeshAttrib)
      \arg name             - attribute name
      \arg componentCount   - nomber of components in the attribute, from 1 to 4 (1 is a scalar
                              attribtue, 2 is a vector2, etc.)
      \arg type             - type of the attribute data (float or int, see \ref MeshAttrib)
      \arg attributeHandle  - property set for returning attribute properties, might be NULL.

\pre
 - meshHandle was returned by inputGetMesh
 - attachment is a valid attachement

\post
 - attributeHandle is a valid attribute handle

@returns
- ::kOfxStatOK - the attribute was successfully fetched and returned in the handle,
- ::kOfxStatErrBadIndex - the attribute could not be fetched because it does not exist, or the
                          attachement is not valid.
- ::kOfxStatErrValue - the component count or type is not valid.
- ::kOfxStatErrBadHandle - the mesh handle was invalid,
 */
  OfxStatus(*attributeDefine)(OfxMeshHandle meshHandle,
                              const char *attachment,
                              const char *name,
                              int componentCount,
                              const char *type,
                              OfxPropertySetHandle *attributeHandle);

  /** @brief Get an attribute handle from a mesh

      \arg meshHandle       - mesh handle
      \arg attachment       - attribute attachement (see \ref MeshAttrib)
      \arg name             - attribute name
      \arg attributeHandle  - property set for returning attribute properties

\pre
 - meshHandle was returned by inputGetMesh
 - attachment is a valid attachement

\post
 - attributeHandle is a valid attribute handle

@returns
- ::kOfxStatOK - the attribute was successfully fetched and returned in the handle,
- ::kOfxStatErrBadIndex - the attribute could not be fetched because it does not exist, or the
                          attachement is not valid.
- ::kOfxStatErrBadHandle - the mesh handle was invalid,
 */
  OfxStatus(*meshGetAttribute)(OfxMeshHandle meshHandle,
                               const char *attachment,
                               const char *name,
                               OfxPropertySetHandle *attributeHandle);

  /** @brief Retrieves the property set for a given mesh

  \arg mesh          mesh to get the property set for
  \arg propHandle    pointer to a the property set handle, value is returned here

  The property handle is valid for the lifetime of the mesh.

  @returns
  - ::kOfxStatOK            - the property set was found and returned
  - ::kOfxStatErrBadHandle  - if the mesh handle was invalid
  - ::kOfxStatErrUnknown    - if the type is unknown
  */
  OfxStatus (*meshGetPropertySet)(OfxMeshHandle mesh,
          OfxPropertySetHandle *propHandle);

/** @brief Allocate memory of a mesh in an output input

      \arg meshHandle  - mesh handle

\pre
 - meshHandle was not allocated yet
 - meshHandle was returned by inputGetMesh
 - inputReleaseMesh has not been called yet
 - meshHandle kOfxMeshPropPointCount, kOfxMeshPropVertexCount, kOfxMeshPropFaceCount properties
 must have been set.

\post
 - all attribut data pointers have been allocated
 - meshHandle attributes will no longer change (no call to meshDefineAttribute)

@returns
- ::kOfxStatOK           - the mesh was successfully allocated,
- ::kOfxStatErrBadHandle - the mesh handle was invalid,
- ::kOfxStatErrMemory    - the host had not enough memory to complete the operation, plugin should abort whatever it was doing.

  */
  OfxStatus (*meshAlloc)(OfxMeshHandle meshHandle);

  /** @brief Returns whether to abort processing or not.

      \arg meshEffect  - instance of the mesh effect

  A host may want to signal to a plugin that it should stop whatever cooking it is doing and start again. 
  Generally this is done in interactive threads in response to users tweaking some parameter.

  This function indicates whether a plugin should stop whatever processing it is doing.
  
  @returns
     - 0 if the effect should continue whatever processing it is doing
     - 1 if the effect should abort whatever processing it is doing  
 */
  int (*abort)(OfxMeshEffectHandle meshEffect);

} OfxMeshEffectSuiteV1;



#ifdef __cplusplus
}
#endif

#endif // _ofxMeshEffect_h_
