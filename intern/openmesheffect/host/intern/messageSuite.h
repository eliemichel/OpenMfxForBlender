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

#ifndef __MFX_MESSAGE_SUITE_H__
#define __MFX_MESSAGE_SUITE_H__

// // Message Suite Entry Points

#include "ofxMessage.h"

#ifdef __cplusplus
extern "C" {
#endif

// See ofxMessage.h for docstrings

extern const OfxMessageSuiteV2 gMessageSuiteV2;

OfxStatus message(
    void *handle, const char *messageType, const char *messageId, const char *format, ...);
OfxStatus setPersistentMessage(
    void *handle, const char *messageType, const char *messageId, const char *format, ...);
OfxStatus clearPersistentMessage(void *handle);

#ifdef __cplusplus
}
#endif

#endif // __MFX_MESSAGE_SUITE_H__
