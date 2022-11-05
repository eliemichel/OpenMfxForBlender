/*
 * Copyright 2019-2022 Elie Michel
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "MeshProps.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <ofxMeshEffect.h>

namespace OpenMfx {

//-----------------------------------------------------------------------------

OfxStatus MeshProps::fetchProperties(const OfxPropertySuiteV1* propertySuite, OfxPropertySetHandle properties)
{
    int noLooseEdgeInt;

    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshPropPointCount, 0, &pointCount));
    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshPropCornerCount, 0, &cornerCount));
    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshPropFaceCount, 0, &faceCount));
    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshPropNoLooseEdge, 0, &noLooseEdgeInt));
    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshPropConstantFaceSize, 0, &constantFaceSize));
    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshPropAttributeCount, 0, &attributeCount));

    noLooseEdge = (bool)noLooseEdgeInt;
    
    return kOfxStatOK;
}

OfxStatus MeshProps::setProperties(const OfxPropertySuiteV1* propertySuite, OfxPropertySetHandle properties) const
{
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshPropPointCount, 0, pointCount));
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshPropCornerCount, 0, cornerCount));
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshPropFaceCount, 0, faceCount));
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshPropNoLooseEdge, 0, (int)noLooseEdge));
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshPropConstantFaceSize, 0, constantFaceSize));
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshPropAttributeCount, 0, attributeCount));
    
    return kOfxStatOK;
}

} // namespace OpenMfx
