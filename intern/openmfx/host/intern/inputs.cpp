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

#include "inputs.h"

#include <cstring>

using namespace OpenMfx;

// // OfxInputStruct

OfxMeshInputStruct::OfxMeshInputStruct()
    : properties(PropertySetContext::Input)
    , host(nullptr)
{
  properties["OfxInputPropRequestGeometry"].value->as_int = 1;
  properties["OfxInputPropRequestTransform"].value->as_int = 0;
}

void OfxMeshInputStruct::deep_copy_from(const OfxMeshInputStruct &other)
{
  this->m_name = other.m_name;
  this->properties.deep_copy_from(other.properties);
  this->mesh.deep_copy_from(other.mesh);
  this->host = other.host;  // not deep copied, as this is a weak pointer
}

// // OfxInputSetStruct

OfxMeshInputSetStruct::OfxMeshInputSetStruct()
{
  host = nullptr;
}

void OfxMeshInputSetStruct::onNewItem(OfxMeshInputStruct & input)
{
  input.host = host;
}
