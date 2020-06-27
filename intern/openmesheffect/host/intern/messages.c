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

#include "messages.h"
#include "mesheffect.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static OfxMessageType parseMessageType(const char *messageType)
{
  if (0 == strcmp(messageType, kOfxMessageFatal)) {
    return OFX_MESSAGE_FATAL;
  }
  if (0 == strcmp(messageType, kOfxMessageError)) {
    return OFX_MESSAGE_ERROR;
  }
  if (0 == strcmp(messageType, kOfxMessageWarning)) {
    return OFX_MESSAGE_WARNING;
  }
  if (0 == strcmp(messageType, kOfxMessageMessage)) {
    return OFX_MESSAGE_MESSAGE;
  }
  if (0 == strcmp(messageType, kOfxMessageLog)) {
    return OFX_MESSAGE_LOG;
  }
  if (0 == strcmp(messageType, kOfxMessageQuestion)) {
    return OFX_MESSAGE_QUESTION;
  }
  return OFX_MESSAGE_INVALID;
}

const char *messageTypeTag(OfxMessageType type)
{
  switch (type) {
    case OFX_MESSAGE_INVALID:
      return "INVALID";
    case OFX_MESSAGE_FATAL:
      return "FATAL";
    case OFX_MESSAGE_ERROR:
      return "ERROR";
    case OFX_MESSAGE_WARNING:
      return "WARNING";
    case OFX_MESSAGE_MESSAGE:
      return "MESSAGE";
    case OFX_MESSAGE_LOG:
      return "LOG";
    case OFX_MESSAGE_QUESTION:
      return "QUESTION";
    default:
      return "";
  }
}

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
  printf("[OpenMeshEffect] %s (%p): ", messageTypeTag(type), handle);

  va_list args;
  va_start(args, format);
  vprintf(format, args);
  va_end(args);

  printf("\n");
  return kOfxStatOK;
}

OfxStatus setPersistentMessage(
    void *handle, const char *messageType, const char *messageId, const char *format, ...)
{
  if (handle == NULL) {
    return kOfxStatErrBadHandle;
  }
  OfxMessageType type = parseMessageType(messageType);
  if (type != OFX_MESSAGE_ERROR && type != OFX_MESSAGE_WARNING && type != OFX_MESSAGE_MESSAGE) {
    return kOfxStatErrValue;
  }

  // effect instance handle
  OfxMeshEffectHandle effect = (OfxMeshEffectHandle)handle;
  effect->messageType = type;
  
  va_list args;
  va_start(args, format);
  vsprintf(effect->message, format, args);
  va_end(args);

  return kOfxStatOK;
}

OfxStatus clearPersistentMessage(void* handle)
{
  if (handle == NULL) {
    return kOfxStatErrBadHandle;
  }
  OfxMeshEffectHandle effect = (OfxMeshEffectHandle)handle;
  effect->messageType = OFX_MESSAGE_INVALID;
  return kOfxStatOK;
}
