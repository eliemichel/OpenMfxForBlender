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

#pragma once

#include "Logger.h"
#include "status.h"

#include <ofxCore.h>

#include <stdio.h>

/**
 * Macro to wrap calls to any OpenFX function (which returns an OfxStatus)
 * This macro only displays an error message when the returned status is not OK
 */
#define MFX_CHECK(call) { \
  OfxStatus status = call; \
  if (kOfxStatOK != status) { \
    printf("Call '" #call "' returned an invalid status: %d (%s)\n", status, ofxStatusName(status)); \
  } \
}

/**
 * Similar to MFX_CHECK except that this one returns the non-OK status, so it
 * can only be called from a function that itself returns an OfxStatus.
 */
#define MFX_ENSURE(call) { \
  OfxStatus status = call; \
  if (kOfxStatOK != status) { \
    printf("Call '" #call "' returned an invalid status: %d (%s)\n", status, ofxStatusName(status)); \
    return status; \
  } \
}

/**
 * Make a class move only (disable default copy but not default move semantics)
 * Example:
 *   class Foo {
 * public:
 *     Foo();
 *     MOVE_ONLY(Foo)
 *   }
 */
#define MOVE_ONLY(T) \
T(const T &) = delete; \
T &operator=(const T &) = delete; \
T(T&&) = default; \
T &operator=(T&&) = default;
