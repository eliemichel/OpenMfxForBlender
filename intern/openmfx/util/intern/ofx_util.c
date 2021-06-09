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

#include "ofx_util.h"

const char * getOfxStateName(OfxStatus status) {
  static const char *names[] = {
    "kOfxStatOK",
    "kOfxStatFailed",
    "kOfxStatErrFatal",
    "kOfxStatErrUnknown",
    "kOfxStatErrMissingHostFeature",
    "kOfxStatErrUnsupported",
    "kOfxStatErrExists",
    "kOfxStatErrFormat",
    "kOfxStatErrMemory",
    "kOfxStatErrBadHandle",
    "kOfxStatErrBadIndex",
    "kOfxStatErrValue",
    "kOfxStatReplyYes",
    "kOfxStatReplyNo",
    "kOfxStatReplyDefault",
    "Invalid status!"
  };
  if (status < 0 || status > 15) {
    return names[15];
  }
  return names[(size_t)status];
}
