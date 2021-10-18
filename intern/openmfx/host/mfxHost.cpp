/*
 * Copyright 2019-2020 Elie Michel
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
 */


#include "intern/util/ofx_util.h"

#include "intern/messages.h"
#include "intern/properties.h"
#include "intern/parameters.h"
#include "intern/inputs.h"
#include "intern/mesheffect.h"
#include "intern/parameterSuite.h"
#include "intern/propertySuite.h"
#include "intern/meshEffectSuite.h"
#include "intern/messageSuite.h"
#include "mfxPluginRegistry.h"

#include "mfxHost.h"
#include "mfxHost/MfxHost"

#include "ofxProperty.h"
#include "ofxParam.h"
#include "ofxMeshEffect.h"
#include "ofxMessage.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

using namespace OpenMfx;
