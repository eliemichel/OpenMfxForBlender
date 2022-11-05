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

#include "AttributeProps.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <cstring>
#include <cstdio>
#include <cassert>

namespace OpenMfx {

//-----------------------------------------------------------------------------

OfxStatus AttributeProps::fetchProperties(const OfxPropertySuiteV1* propertySuite, OfxPropertySetHandle properties)
{
    char* type;
    int isOwner;

    MFX_ENSURE(propertySuite->propGetString(properties, kOfxMeshAttribPropType, 0, &type));
    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshAttribPropStride, 0, &this->stride));
    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshAttribPropComponentCount, 0, &this->componentCount));
    MFX_ENSURE(propertySuite->propGetPointer(properties, kOfxMeshAttribPropData, 0, (void**)&this->data));
    MFX_ENSURE(propertySuite->propGetInt(properties, kOfxMeshAttribPropIsOwner, 0, &isOwner));

    this->type = attributeTypeAsEnum(type);
    this->isOwner = (bool)isOwner;

    return kOfxStatOK;
}

OfxStatus AttributeProps::fetchProperties(const OfxPropertySuiteV1 *propertySuite,
                                        const OfxMeshEffectSuiteV1 *meshEffectSuite,
                                        OfxMeshHandle mesh,
                                        const char *attachment,
                                        const char *name)
{
    OfxPropertySetHandle properties;

    MFX_ENSURE(meshEffectSuite->meshGetAttribute(mesh, attachment, name, &properties));
    MFX_ENSURE(fetchProperties(propertySuite, properties));

    return kOfxStatOK;
}

OfxStatus AttributeProps::setProperties(const OfxPropertySuiteV1* propertySuite, OfxPropertySetHandle properties) const
{
    const char* type = attributeTypeAsString(this->type);

    MFX_ENSURE(propertySuite->propSetString(properties, kOfxMeshAttribPropType, 0, type));
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshAttribPropStride, 0, this->stride));
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshAttribPropComponentCount, 0, this->componentCount));
    MFX_ENSURE(propertySuite->propSetPointer(properties, kOfxMeshAttribPropData, 0, (void*)this->data));
    MFX_ENSURE(propertySuite->propSetInt(properties, kOfxMeshAttribPropIsOwner, 0, (int)this->isOwner));

    return kOfxStatOK;
}

} // namespace OpenMfx
