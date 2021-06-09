/**
 * Open Mesh Effect modifier for Blender
 * Copyright (C) 2019 - 2020 Elie Michel
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#pragma once

#include "mfxHost.h"
#include <mfxHost/parameters>

#include "ofxParam.h"

#include "DNA_modifier_types.h"

void copy_parameter_value_from_rna(OfxParamHandle param,
                                   const OpenMfxParameter *rna);

void copy_parameter_value_to_rna(OpenMfxParameter *rna,
                                 const OfxPropertyStruct * prop);

void copy_parameter_minmax_to_rna(int rna_type,
                                  int &int_rna,
                                  float &float_rna,
                                  const OfxPropertyStruct *prop);

void copy_parameter_value_to_rna(OpenMfxParameter *rna,
                                 const OfxParamHandle param);
