#include "propertySuite.h"
#include "types.h"

#include <OpenMfx/Sdk/C/Common>

#include <stdio.h>
#include <string.h>

OfxStatus propSetPointer(OfxPropertySetHandle properties,
                         const char *property,
                         int index,
                         void *value)
{
  printf("[host] propSetPointer(properties %p, %s, %d, %p)\n", properties, property, index, value);

  switch (properties->type) {
    case PROPSET_ATTRIBUTE:
    {
      OfxMeshAttributePropertySet *attrib_props = (OfxMeshAttributePropertySet*)properties;
      if (0 == strcmp(property, kOfxMeshAttribPropData)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        attrib_props->data = (char*)value;
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_UNKNOWN:
    default:
      return kOfxStatErrBadHandle;
  }

  return kOfxStatOK;
}

OfxStatus propGetPointer(OfxPropertySetHandle properties,
                         const char *property,
                         int index,
                         void **value)
{
  printf("[host] propGetPointer(properties %p, %s, %d)\n", properties, property, index);

  switch (properties->type) {
    case PROPSET_ATTRIBUTE:
    {
      OfxMeshAttributePropertySet *attrib_props = (OfxMeshAttributePropertySet*)properties;
      if (0 == strcmp(property, kOfxMeshAttribPropData)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = (void*)attrib_props->data;
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_UNKNOWN:
    default:
      return kOfxStatErrBadHandle;
  }

  return kOfxStatOK;
}

OfxStatus propSetString(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        const char *value)
{
  printf("[host] propSetString(properties %p, %s, %d, %s)\n", properties, property, index, value);

  switch (properties->type) {
    case PROPSET_INPUT:
    {
      OfxMeshInputPropertySet *input_props = (OfxMeshInputPropertySet*)properties;
      if (0 == strcmp(property, kOfxPropLabel)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        strncpy(input_props->label, value, 256);
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_ATTRIBUTE:
    {
      OfxMeshAttributePropertySet *attrib_props = (OfxMeshAttributePropertySet*)properties;
      if (0 == strcmp(property, kOfxMeshAttribPropType)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        strncpy(attrib_props->type, value, 64);
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshAttribPropSemantic)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        strncpy(attrib_props->semantic, value, 64);
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_UNKNOWN:
    default:
      return kOfxStatErrBadHandle;
  }

  return kOfxStatOK;
}

OfxStatus propGetString(OfxPropertySetHandle properties,
                        const char *property,
                        int index,
                        char **value)
{
  printf("[host] propSetString(properties %p, %s, %d)\n", properties, property, index);

  switch (properties->type) {
    case PROPSET_INPUT:
    {
      OfxMeshInputPropertySet *input_props = (OfxMeshInputPropertySet*)properties;
      if (0 == strcmp(property, kOfxPropLabel)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = input_props->label;
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_ATTRIBUTE:
    {
      OfxMeshAttributePropertySet *attrib_props = (OfxMeshAttributePropertySet*)properties;
      if (0 == strcmp(property, kOfxMeshAttribPropType)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = attrib_props->type;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshAttribPropSemantic)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = attrib_props->semantic;
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_UNKNOWN:
    default:
      return kOfxStatErrBadHandle;
  }

  return kOfxStatOK;
}

OfxStatus propSetInt(OfxPropertySetHandle properties,
                     const char *property,
                     int index,
                     int value)
{
  printf("[host] propSetInt(properties %p, %s, %d, %d)\n", properties, property, index, value);

  switch (properties->type) {
    case PROPSET_MESH:
    {
      OfxMeshPropertySet *mesh_props = (OfxMeshPropertySet*)properties;
      if (0 == strcmp(property, kOfxMeshPropPointCount)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        mesh_props->point_count = value;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshPropCornerCount)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        mesh_props->corner_count = value;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshPropFaceCount)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        mesh_props->face_count = value;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshPropConstantFaceSize)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        mesh_props->constant_face_size = value;
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_ATTRIBUTE:
    {
      OfxMeshAttributePropertySet *attrib_props = (OfxMeshAttributePropertySet*)properties;
      if (0 == strcmp(property, kOfxMeshAttribPropComponentCount)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        attrib_props->component_count = value;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshAttribPropStride)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        attrib_props->byte_stride = (size_t)value;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshAttribPropIsOwner)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        attrib_props->is_owner = value;
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_UNKNOWN:
    default:
      return kOfxStatErrBadHandle;
  }

  return kOfxStatOK;
}

OfxStatus propGetInt(OfxPropertySetHandle properties,
                     const char *property,
                     int index,
                     int *value)
{
  printf("[host] propGetInt(properties %p, %s, %d)\n", properties, property, index);

  switch (properties->type) {
    case PROPSET_MESH:
    {
      OfxMeshPropertySet *mesh_props = (OfxMeshPropertySet*)properties;
      if (0 == strcmp(property, kOfxMeshPropPointCount)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = mesh_props->point_count;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshPropCornerCount)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = mesh_props->corner_count;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshPropFaceCount)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = mesh_props->face_count;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshPropConstantFaceSize)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = mesh_props->constant_face_size;
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_ATTRIBUTE:
    {
      OfxMeshAttributePropertySet *attrib_props = (OfxMeshAttributePropertySet*)properties;
      if (0 == strcmp(property, kOfxMeshAttribPropComponentCount)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = attrib_props->component_count;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshAttribPropStride)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = (int)attrib_props->byte_stride;
        return kOfxStatOK;
      }
      else if (0 == strcmp(property, kOfxMeshAttribPropIsOwner)) {
        if (index != 0) {
          return kOfxStatErrBadIndex;
        }
        *value = attrib_props->is_owner;
        return kOfxStatOK;
      }
      return kOfxStatErrBadHandle;
    }
    case PROPSET_UNKNOWN:
    default:
      return kOfxStatErrBadHandle;
  }

  return kOfxStatOK;
}

const OfxPropertySuiteV1 propertySuiteV1 = {
  propSetPointer, // OfxStatus (*propSetPointer)(OfxPropertySetHandle properties, const char *property, int index, void *value);
  propSetString, // OfxStatus (*propSetString) (OfxPropertySetHandle properties, const char *property, int index, const char *value);
  NULL, // OfxStatus (*propSetDouble) (OfxPropertySetHandle properties, const char *property, int index, double value);
  propSetInt, // OfxStatus (*propSetInt)    (OfxPropertySetHandle properties, const char *property, int index, int value);
  NULL, // OfxStatus (*propSetPointerN)(OfxPropertySetHandle properties, const char *property, int count, void *const*value);
  NULL, // OfxStatus (*propSetStringN) (OfxPropertySetHandle properties, const char *property, int count, const char *const*value);
  NULL, // OfxStatus (*propSetDoubleN) (OfxPropertySetHandle properties, const char *property, int count, const double *value);
  NULL, // OfxStatus (*propSetIntN)    (OfxPropertySetHandle properties, const char *property, int count, const int *value);
  propGetPointer, // OfxStatus (*propGetPointer)(OfxPropertySetHandle properties, const char *property, int index, void **value);
  propGetString, // OfxStatus (*propGetString) (OfxPropertySetHandle properties, const char *property, int index, char **value);
  NULL, // OfxStatus (*propGetDouble) (OfxPropertySetHandle properties, const char *property, int index, double *value);
  propGetInt, // OfxStatus (*propGetInt)    (OfxPropertySetHandle properties, const char *property, int index, int *value);
  NULL, // OfxStatus (*propGetPointerN)(OfxPropertySetHandle properties, const char *property, int count, void **value);
  NULL, // OfxStatus (*propGetStringN) (OfxPropertySetHandle properties, const char *property, int count, char **value);
  NULL, // OfxStatus (*propGetDoubleN) (OfxPropertySetHandle properties, const char *property, int count, double *value);
  NULL, // OfxStatus (*propGetIntN)    (OfxPropertySetHandle properties, const char *property, int count, int *value);
  NULL, // OfxStatus (*propReset)    (OfxPropertySetHandle properties, const char *property);
  NULL, // OfxStatus (*propGetDimension)  (OfxPropertySetHandle properties, const char *property, int *count);
};
