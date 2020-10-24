/*
 * Copyright 2019 Elie Michel
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

#include <stdio.h>
#include <string.h>

#include "util/memory_util.h"

#include "mesheffect.h"
#include "propertySuite.h"

// OFX MESH EFFECT SUITE

// // Mesh Effect

OfxMeshEffectStruct::OfxMeshEffectStruct(OfxHost *host)
	: properties(PropertySetContext::MeshEffect)
{
  this->host = host;
  this->inputs.host = host;
  this->parameters.effect_properties = &this->properties;
  this->messageType = OfxMessageType::Invalid;
  this->message[0] = '\0';
}

OfxMeshEffectStruct::~OfxMeshEffectStruct()
{
}

void OfxMeshEffectStruct::deep_copy_from(const OfxMeshEffectStruct &other)
{
  this->inputs.deep_copy_from(other.inputs);
  this->properties.deep_copy_from(other.properties);
  this->parameters.deep_copy_from(other.parameters);
  this->parameters.effect_properties = &this->properties;
  this->host = other.host;  // not deep copied, as this is a weak pointer
  this->messageType = other.messageType;
  strncpy(this->message, other.message, sizeof(other.message));
}
