#ifndef _types_h_
#define _types_h_

/*****************************************************************************/
/* Data Structures (and their ctor/dtor/copy) */

#include <ofxMeshEffect.h>
#include <ofxCore.h>

typedef enum OfxPropertySetType {
  PROPSET_UNKNOWN,
  PROPSET_INPUT,
  PROPSET_PARAM,
  PROPSET_MESH,
  PROPSET_ATTRIBUTE,
} OfxPropertySetType;

typedef struct OfxPropertySetStruct {
  OfxPropertySetType type;
} OfxPropertySetStruct;

typedef struct OfxParamPropertySet {
  OfxPropertySetStruct *header;
  char label[256];
} OfxParamPropertySet;

typedef union OfxParamValueStruct {
    void *as_pointer;
    const char *as_const_char;
    char *as_char;
    int as_int;
    double as_double;
    int as_bool;
} OfxParamValueStruct;

typedef struct OfxParamStruct {
  int is_valid;
  char name[64];
  char type[64];
  OfxParamValueStruct values[4];
  OfxParamPropertySet properties;
} OfxParamStruct;

typedef struct OfxParamSetStruct {
  OfxParamStruct entries[16];
} OfxParamSetStruct;

typedef struct OfxMeshPropertySet {
  OfxPropertySetStruct *header;
  int point_count;
  int corner_count;
  int face_count;
  int constant_face_size;
} OfxMeshPropertySet;

typedef struct OfxMeshAttributePropertySet {
  OfxPropertySetStruct *header;
  int is_valid;
  char name[64];
  char attachment[64];
  int component_count;
  char type[64];
  char semantic[64];
  char *data;
  size_t byte_stride;
  int is_owner;
} OfxMeshAttributePropertySet;

typedef struct OfxMeshStruct {
  OfxMeshAttributePropertySet attributes[32];
  OfxMeshPropertySet properties;
} OfxMeshStruct;

typedef struct OfxMeshInputPropertySet {
  OfxPropertySetStruct *header;
  char label[256];
} OfxMeshInputPropertySet;

typedef struct OfxMeshInputStruct {
  int is_valid;
  char name[256];
  OfxMeshStruct mesh;
  OfxMeshInputPropertySet properties;
} OfxMeshInputStruct;

typedef struct OfxMeshEffectStruct {
  int is_valid;
  OfxMeshInputStruct inputs[16];
  OfxParamSetStruct parameters;
} OfxMeshEffectStruct;

void meshInputPropertySetCopy(OfxMeshInputPropertySet *dst, const OfxMeshInputPropertySet *src);

void parameterPropertySetCopy(OfxParamPropertySet *dst, const OfxParamPropertySet *src);

void meshPropertySetCopy(OfxMeshPropertySet *dst, const OfxMeshPropertySet *src);

void propertySetCopy(OfxPropertySetHandle dst, const OfxPropertySetStruct *src);

void propertySetInit(OfxPropertySetHandle propertySet, OfxPropertySetType type);

void attributeInit(OfxMeshAttributePropertySet *attrib);

OfxStatus attributeAlloc(OfxMeshAttributePropertySet *attrib, OfxMeshPropertySet *props);

void attributeDestroy(OfxMeshAttributePropertySet *attrib);

void attributeShallowCopy(OfxMeshAttributePropertySet *dst, const OfxMeshAttributePropertySet *src);

void parameterCopy(OfxParamHandle dst, const OfxParamStruct *src);

void parameterSetInit(OfxParamSetHandle parameterSet);

void parameterSetCopy(OfxParamSetHandle dst, const OfxParamSetStruct *src);

void paramInit(OfxParamHandle param);

void meshInit(OfxMeshHandle mesh);

void meshDestroy(OfxMeshHandle mesh);

void meshShallowCopy(OfxMeshHandle dst, const OfxMeshStruct *src);

void meshInputInit(OfxMeshInputHandle input);

void meshInputCopy(OfxMeshInputHandle dst, const OfxMeshInputStruct *src);

void meshInputDestroy(OfxMeshInputHandle input);

void meshEffectInit(OfxMeshEffectHandle meshEffect);

void meshEffectDestroy(OfxMeshEffectHandle meshEffect);

void meshEffectCopy(OfxMeshEffectHandle dst, const OfxMeshEffectStruct *src);

#endif // _types_h_
