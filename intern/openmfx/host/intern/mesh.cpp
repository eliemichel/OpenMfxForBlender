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

#include <string.h>

#include "mesh.h"
#include "propertySuite.h"
#include "meshEffectSuite.h"

using namespace OpenMfx;

// // OfxInputStruct

OfxMeshStruct::OfxMeshStruct()
	: properties(PropertySetContext::Mesh)
{}

OfxMeshStruct::~OfxMeshStruct()
{
}

void OfxMeshStruct::deep_copy_from(const OfxMeshStruct &other) {
  this->properties.deep_copy_from(other.properties);
  this->attributes.deep_copy_from(other.attributes);
}

void OfxMeshStruct::free_owned_data()
{
    void* data;
    int is_owner;
    for (int i = 0; i < attributes.count(); ++i) {
        OfxAttributeStruct& attribute = attributes[i];
        propGetPointer(&attribute.properties, kOfxMeshAttribPropData, 0, &data);
        propGetInt(&attribute.properties, kOfxMeshAttribPropIsOwner, 0, &is_owner);
        if (is_owner && NULL != data) {
            delete[] static_cast<char*>(data);  // delete on void* is undefined behaviour
        }
        propSetPointer(&attribute.properties, kOfxMeshAttribPropData, 0, NULL);
        propSetInt(&attribute.properties, kOfxMeshAttribPropIsOwner, 0, 0);
    }
}
