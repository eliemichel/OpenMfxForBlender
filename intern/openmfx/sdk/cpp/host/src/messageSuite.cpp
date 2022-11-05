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

#include "messageSuite.h"
#include "messages.h"
#include "MeshEffect.h"

#include <OpenMfx/Sdk/Cpp/Common>

#include <string>
#include <cstdio>
#include <cstring>
#include <cstdarg>

// // Message Suite Entry Points

const OfxMessageSuiteV2 gMessageSuiteV2 = {
    message,
    setPersistentMessage,
    clearPersistentMessage,
};

OfxStatus message(
    void *handle, const char *messageType, const char *messageId, const char *format, ...)
{
  OfxMessageType type = parseMessageType(messageType);
  std::string content;

  va_list args;
  va_start(args, format);
  int length = snprintf(NULL, 0, format, args) + 1;
  content.resize(length);
  snprintf(&content[0], length, format, args);
  va_end(args);

  LOG << messageTypeTag(type) << " (" << handle << ") " << content;
  return kOfxStatOK;
}

OfxStatus setPersistentMessage(
    void *handle, const char *messageType, const char *messageId, const char *format, ...)
{
  if (handle == NULL) {
    return kOfxStatErrBadHandle;
  }
  OfxMessageType type = parseMessageType(messageType);
  if (type != OfxMessageType::Error && type != OfxMessageType::Warning &&
      type != OfxMessageType::Message) {
    return kOfxStatErrValue;
  }

  // effect instance handle
  OfxMeshEffectHandle effect = (OfxMeshEffectHandle)handle;
  effect->messageType = type;
  
  va_list args;
  va_start(args, format);
  snprintf(effect->message, 1024, format, args);
  va_end(args);

  return kOfxStatOK;
}

OfxStatus clearPersistentMessage(void* handle)
{
  if (handle == NULL) {
    return kOfxStatErrBadHandle;
  }
  OfxMeshEffectHandle effect = (OfxMeshEffectHandle)handle;
  effect->messageType = OfxMessageType::Invalid;
  return kOfxStatOK;
}
