/*
 * Copyright 2019-2021 Elie Michel
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
#include <stdio.h>
#include "plugin_support.h"

#define MFX_ENSURE(op) status = op; if (kOfxStatOK != status) return status;

PluginRuntime gRuntime;

enum AttributeType mfxAttrAsEnum(const char *attr_type)
{
  if (0 == strcmp(attr_type, kOfxMeshAttribTypeUByte)) {
    return MFX_UBYTE_ATTR;
  }
  if (0 == strcmp(attr_type, kOfxMeshAttribTypeInt)) {
    return MFX_INT_ATTR;
  }
  if (0 == strcmp(attr_type, kOfxMeshAttribTypeFloat)) {
    return MFX_FLOAT_ATTR;
  }
  printf("Warning: inknown attribute type: %s\n", attr_type);
  return MFX_UNKNOWN_ATTR;
}

OfxStatus getAttribute(OfxMeshHandle mesh, const char *attachment, const char *name, Attribute *attr)
{
  const OfxMeshEffectSuiteV1 *meshEffectSuite = gRuntime.meshEffectSuite;
  const OfxPropertySuiteV1 *propertySuite = gRuntime.propertySuite;
  OfxStatus status;

  OfxPropertySetHandle attr_props;
  char *type;
  MFX_ENSURE(meshEffectSuite->meshGetAttribute(mesh, attachment, name, &attr_props));
  MFX_ENSURE(propertySuite->propGetString(attr_props, kOfxMeshAttribPropType, 0, &type));
  MFX_ENSURE(propertySuite->propGetInt(attr_props, kOfxMeshAttribPropStride, 0, &attr->stride));
  MFX_ENSURE(propertySuite->propGetInt(attr_props, kOfxMeshAttribPropComponentCount, 0, &attr->componentCount));
  MFX_ENSURE(propertySuite->propGetPointer(attr_props, kOfxMeshAttribPropData, 0, (void**)&attr->data));
  attr->type = mfxAttrAsEnum(type);

  return kOfxStatOK;
}

OfxStatus getPointAttribute(OfxMeshHandle mesh, const char *name, Attribute *attr)
{
  return getAttribute(mesh, kOfxMeshAttribPoint, name, attr);
}

OfxStatus getCornerAttribute(OfxMeshHandle mesh, const char *name, Attribute *attr)
{
  return getAttribute(mesh, kOfxMeshAttribCorner, name, attr);
}

OfxStatus getFaceAttribute(OfxMeshHandle mesh, const char *name, Attribute *attr)
{
  return getAttribute(mesh, kOfxMeshAttribFace, name, attr);
}

OfxStatus copyAttribute(Attribute *destination, const Attribute *source, int start, int count)
{
  int componentCount = source->componentCount < destination->componentCount ? source->componentCount : destination->componentCount;

  if (source->type == destination->type)
  {
    size_t componentByteSize = 0;
    switch (source->type)
    {
    case MFX_UBYTE_ATTR:
      componentByteSize = sizeof(unsigned char);
      break;
    case MFX_INT_ATTR:
      componentByteSize = sizeof(int);
      break;
    case MFX_FLOAT_ATTR:
      componentByteSize = sizeof(float);
      break;
    default:
      printf("Error: unsupported attribute type: %d\n", source->type);
      return kOfxStatErrFatal;
    }

    for (int i = 0; i < count; ++i) {
      const void *src = (void*)&source->data[(start + i) * source->stride];
      void *dst = (void*)&destination->data[(start + i) * destination->stride];
      memcpy(dst, src, componentCount * componentByteSize);
    }
    return kOfxStatOK;
  }

  switch (destination->type) {
  case MFX_FLOAT_ATTR:
    switch (source->type) {
    case MFX_UBYTE_ATTR:
      for (int i = 0; i < count; ++i) {
        const unsigned char *src = (unsigned char *)&source->data[(start + i) * source->stride];
        float *dst = (float*)&destination->data[(start + i) * destination->stride];
        for (int k = 0; k < componentCount; ++k)
        {
          dst[k] = (float)src[k] / 255.0f;
        }
      }
      return kOfxStatOK;
      break;
    }
    break;
  }
  printf("Warning: unsupported input/output type combinason in copyAttribute: %d -> %d\n", source->type, destination->type);
  return kOfxStatErrUnsupported;
}
