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

#include "Attributes.h"
#include "propertySuite.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <ofxMeshEffect.h>

#include <cstring>
#include <algorithm>

using namespace OpenMfx;

// // OfxMeshAttributeStruct

OfxAttributeStruct::OfxAttributeStruct()
    : properties(PropertySetContext::Attrib)
{}

void OfxAttributeStruct::deep_copy_from(const OfxAttributeStruct &other)
{
  m_name = other.m_name;
  m_attachment = other.m_attachment;
  
  properties.deep_copy_from(other.properties);
}

AttributeAttachment OfxAttributeStruct::attachment() const
{
    return m_attachment;
}

const std::string& OfxAttributeStruct::name() const
{
    return m_name;
}

AttributeType OfxAttributeStruct::type() const
{
    char* stringType;
    OfxPropertySetHandle mutableProperties = const_cast<OfxPropertySetHandle>(&properties);
    propGetString(mutableProperties, kOfxMeshAttribPropType, 0, &stringType);
    return attributeTypeAsEnum(stringType);
}

void OfxAttributeStruct::setType(AttributeType type) {
    propSetString(&properties, kOfxMeshAttribPropType, 0, attributeTypeAsString(type));
}

int OfxAttributeStruct::componentCount() const
{
    int count;
    OfxPropertySetHandle mutableProperties = const_cast<OfxPropertySetHandle>(&properties);
    propGetInt(mutableProperties, kOfxMeshAttribPropComponentCount, 0, &count);
    return count;
}

void OfxAttributeStruct::setComponentCount(int componentCount)
{
    propSetInt(&properties, kOfxMeshAttribPropComponentCount, 0, componentCount);
}

AttributeSemantic OfxAttributeStruct::semantic() const
{
    char *stringSemantic;
    OfxPropertySetHandle mutableProperties = const_cast<OfxPropertySetHandle>(&properties);
    propGetString(mutableProperties, kOfxMeshAttribPropSemantic, 0, &stringSemantic);
    return attributeSemanticAsEnum(stringSemantic);
}

void OfxAttributeStruct::setSemantic(AttributeSemantic semantic)
{
    propSetString(&properties, kOfxMeshAttribPropSemantic, 0, attributeSemanticAsString(semantic));
}

void* OfxAttributeStruct::data() const
{
    void* value;
    OfxPropertySetHandle mutableProperties = const_cast<OfxPropertySetHandle>(&properties);
    propGetPointer(mutableProperties, kOfxMeshAttribPropData, 0, &value);
    return value;
}

void OfxAttributeStruct::setData(void* data)
{
    propSetPointer(&properties, kOfxMeshAttribPropData, 0, data);
}

int OfxAttributeStruct::byteStride() const
{
    int value;
    OfxPropertySetHandle mutableProperties = const_cast<OfxPropertySetHandle>(&properties);
    propGetInt(mutableProperties, kOfxMeshAttribPropStride, 0, &value);
    return value;
}

void OfxAttributeStruct::setByteStride(int byteStride)
{
    propSetInt(&properties, kOfxMeshAttribPropStride, 0, byteStride);
}

void OfxAttributeStruct::setIndex(const Index& index)
{
    m_attachment = index.first;
    m_name = index.second;
}

OfxAttributeStruct::Index OfxAttributeStruct::index() const
{
    return std::make_pair(m_attachment, m_name);
}

bool OfxAttributeStruct::copy_data_from(const OfxAttributeStruct& source, int start, int count)
{
    AttributeType sourceType = attributeTypeAsEnum(source.properties[kOfxMeshAttribPropType].value[0].as_char);
    AttributeType destinationType = attributeTypeAsEnum(properties[kOfxMeshAttribPropType].value[0].as_char);

    int sourceComponentCount = source.properties[kOfxMeshAttribPropComponentCount].value[0].as_int;
    int destinationComponentCount = properties[kOfxMeshAttribPropComponentCount].value[0].as_int;

    char* sourceData = (char*)source.properties[kOfxMeshAttribPropData].value[0].as_pointer;
    char* destinationData = (char*)properties[kOfxMeshAttribPropData].value[0].as_pointer;

    int sourceStride = source.properties[kOfxMeshAttribPropStride].value[0].as_int;
    int destinationStride = properties[kOfxMeshAttribPropStride].value[0].as_int;

    // TODO: use at least OpenMP
    int componentCount = std::min(sourceComponentCount, destinationComponentCount);

    if (sourceType == destinationType)
    {
        size_t componentByteSize = byteSizeOf(sourceType);
        if (componentByteSize == 0) return false;

        for (int i = 0; i < count; ++i) {
            const void* src = (void*)&sourceData[(start + i) * sourceStride];
            void* dst = (void*)&destinationData[(start + i) * destinationStride];
            memcpy(dst, src, componentCount * componentByteSize);
        }
        return true;
    }

    switch (destinationType) {
    case AttributeType::Float:
        switch (sourceType) {
        case AttributeType::UByte:
            for (int i = 0; i < count; ++i) {
                const unsigned char* src = (unsigned char*)&sourceData[(start + i) * sourceStride];
                float* dst = (float*)&destinationData[(start + i) * destinationStride];
                for (int k = 0; k < componentCount; ++k)
                {
                    dst[k] = (float)src[k] / 255.0f;
                }
            }
            return true;
            break;
        }
        break;
    }
    WARN_LOG << "unsupported input/output type combinason in copyAttribute: " << (int)sourceType << " -> " << (int)destinationType;
    return false;
}
