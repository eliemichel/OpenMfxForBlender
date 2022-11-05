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

#pragma once

#include "Properties.h"
#include "Parameters.h"
#include "Inputs.h"
#include "messages.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <ofxCore.h>
#include <ofxMeshEffect.h>

// Mesh Effect

namespace OpenMfx {
typedef OfxMeshEffectStruct MeshEffect;
} // namespace OpenMfx

struct OfxMeshEffectStruct {
public:
	OfxMeshEffectStruct(OfxHost* host, OfxPlugin* plugin);
	MOVE_ONLY(OfxMeshEffectStruct)

	void deep_copy_from(const OfxMeshEffectStruct& other);

public:
	OfxMeshInputSetStruct inputs;
	OfxPropertySetStruct properties;
	OfxParamSetStruct parameters;
	OfxHost* host; // weak pointer, do not deep copy
	OfxPlugin* plugin; // weak pointer, do not deep copy

	// Only the last persistent message is stored
	OfxMessageType messageType;
	char message[1024];
};
