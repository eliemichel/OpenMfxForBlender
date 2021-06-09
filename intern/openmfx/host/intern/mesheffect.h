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

/** \file
 * \ingroup openmesheffect
 *
 */

#ifndef __MFX_MESHEFFECT_H__
#define __MFX_MESHEFFECT_H__

#include "ofxMeshEffect.h"
#include "ofxExtras.h"

#include "properties.h"
#include "parameters.h"
#include "inputs.h"
#include "messages.h"

// Mesh Effect

struct OfxMeshEffectStruct {
 public:
  OfxMeshEffectStruct(OfxHost *host);
  ~OfxMeshEffectStruct();

  // Disable copy, we handle it explicitely
  OfxMeshEffectStruct(const OfxMeshEffectStruct &) = delete;
  OfxMeshEffectStruct &operator=(const OfxMeshEffectStruct &) = delete;

  void deep_copy_from(const OfxMeshEffectStruct &other);

 public:
  OfxMeshInputSetStruct inputs;
  OfxPropertySetStruct properties;
  OfxParamSetStruct parameters;
  OfxHost *host; // weak pointer, do not deep copy

  // Only the last persistent message is stored
  OfxMessageType messageType;
  char message[1024];
};

#endif // __MFX_MESHEFFECT_H__
