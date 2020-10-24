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

#include <cstring>

OfxMessageType parseMessageType(const char *messageType)
{
  if (0 == strcmp(messageType, kOfxMessageFatal)) {
      return OfxMessageType::Fatal;
  }
  if (0 == strcmp(messageType, kOfxMessageError)) {
    return OfxMessageType::Error;
  }
  if (0 == strcmp(messageType, kOfxMessageWarning)) {
    return OfxMessageType::Warning;
  }
  if (0 == strcmp(messageType, kOfxMessageMessage)) {
    return OfxMessageType::Message;
  }
  if (0 == strcmp(messageType, kOfxMessageLog)) {
    return OfxMessageType::Log;
  }
  if (0 == strcmp(messageType, kOfxMessageQuestion)) {
    return OfxMessageType::Question;
  }
  return OfxMessageType::Invalid;
}

const char *messageTypeTag(OfxMessageType type)
{
  switch (type) {
    case OfxMessageType::Invalid:
      return "INVALID";
    case OfxMessageType::Fatal:
      return "FATAL";
    case OfxMessageType::Error:
      return "ERROR";
    case OfxMessageType::Warning:
      return "WARNING";
    case OfxMessageType::Message:
      return "MESSAGE";
    case OfxMessageType::Log:
      return "LOG";
    case OfxMessageType::Question:
      return "QUESTION";
    default:
      return "";
  }
}
