/*
 * Copyright 2019 - 2020 Elie Michel
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

#ifndef __MFX_MESSAGES_H__
#define __MFX_MESSAGES_H__

#include "ofxMessage.h"

enum class OfxMessageType {
  Invalid = -1, // meaning "don't show"
  Fatal,
  Error,
  Warning,
  Message,
  Log,
  Question,
};

/**
 * Get message type as an enum value from its string based expression
 */
OfxMessageType parseMessageType(const char *messageType);

/**
 * Return a human readable version of OfxMessageType value
 */
const char *messageTypeTag(OfxMessageType type);

#endif // __MFX_MESSAGES_H__
