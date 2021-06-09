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
 * Plugin Supports - helpers common to all plugins
 *
 */

#ifndef __MFX_PLUGIN_SUPPORT_H__
#define __MFX_PLUGIN_SUPPORT_H__

#include "ofxCore.h"
#include "ofxMeshEffect.h"

typedef struct PluginRuntime {
  OfxHost *host;
  const OfxPropertySuiteV1 *propertySuite;
  const OfxParameterSuiteV1 *parameterSuite;
  const OfxMeshEffectSuiteV1 *meshEffectSuite;
} PluginRuntime;

enum AttributeType {
  MFX_UNKNOWN_ATTR = -1,
  MFX_UBYTE_ATTR,
  MFX_INT_ATTR,
  MFX_FLOAT_ATTR,
};

typedef struct Attribute {
  enum AttributeType type;
  int stride;
  int componentCount;
  char *data;
} Attribute;

/**
 * Convert a type string from MeshEffect API to its local enum counterpart
 */
enum AttributeType mfxAttrAsEnum(const char *attr_type);

/**
 * Get attribute info from low level open mesh effect API and store it in a struct Attribute
 */
OfxStatus getAttribute(OfxMeshHandle mesh, const char *attachment, const char *name, Attribute *attr);

// Sugar for getAttribute()
OfxStatus getPointAttribute(OfxMeshHandle mesh, const char *name, Attribute *attr);
OfxStatus getCornerAttribute(OfxMeshHandle mesh, const char *name, Attribute *attr);
OfxStatus getFaceAttribute(OfxMeshHandle mesh, const char *name, Attribute *attr);

/**
 * Copy attribute and try to cast. If number of component is different, copy the common components
 * only.
 */
OfxStatus copyAttribute(Attribute *destination, const Attribute *source, int start, int count);

// !global!
extern PluginRuntime gRuntime;

#endif // __MFX_PLUGIN_SUPPORT_H__
