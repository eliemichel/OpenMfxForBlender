/**
 * Open Mesh Effect modifier for Blender
 * Copyright (C) 2019 - 2022 Elie Michel
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

#include "MFX_convert.h"

#include <OpenMfx/Sdk/Cpp/Host/Host>

#include <iostream>
#include <cstring>

using ParameterType = OpenMfx::ParameterType;

void MFX_copy_parameter_value_from_rna(OfxParamHandle param, const OpenMfxParameter *rna)
{
  param->type = static_cast<OpenMfx::ParameterType>(rna->type);
  switch (param->type) {
    case ParameterType::Integer3d:
      param->value[2].as_int = rna->integer_vec_value[2];
    case ParameterType::Integer2d:
      param->value[1].as_int = rna->integer_vec_value[1];
    case ParameterType::Integer:
      param->value[0].as_int = rna->integer_vec_value[0];
      break;

    case ParameterType::Rgba:
      param->value[3].as_double = (double)rna->float_vec_value[3];
    case ParameterType::Double3d:
    case ParameterType::Rgb:
      param->value[2].as_double = (double)rna->float_vec_value[2];
    case ParameterType::Double2d:
      param->value[1].as_double = (double)rna->float_vec_value[1];
    case ParameterType::Double:
      param->value[0].as_double = (double)rna->float_vec_value[0];
      break;

    case ParameterType::Boolean:
      param->value[0].as_bool = rna->integer_vec_value[0];
      break;

    case ParameterType::String:
      param->realloc_string(MOD_OPENMFX_MAX_STRING_VALUE);
      strncpy(param->value[0].as_char, rna->string_value, MOD_OPENMFX_MAX_STRING_VALUE);
      break;

    default:
      std::cerr << "-- Skipping parameter " << param->name
                << " (unsupported type: " << static_cast<int>(param->type) << ")"
                << std::endl;
      break;
  }
}

void MFX_copy_parameter_value_to_rna(OpenMfxParameter *rna, const OfxPropertyStruct *prop)
{
  switch (static_cast<ParameterType>(rna->type)) {
    case ParameterType::Integer3d:
      rna->integer_vec_value[2] = prop->value[2].as_int;
    case ParameterType::Integer2d:
      rna->integer_vec_value[1] = prop->value[1].as_int;
    case ParameterType::Integer:
      rna->integer_vec_value[0] = prop->value[0].as_int;
      break;

    case ParameterType::Rgba:
      rna->float_vec_value[3] = (float)prop->value[3].as_double;
    case ParameterType::Double3d:
    case ParameterType::Rgb:
      rna->float_vec_value[2] = (float)prop->value[2].as_double;
    case ParameterType::Double2d:
      rna->float_vec_value[1] = (float)prop->value[1].as_double;
    case ParameterType::Double:
      rna->float_vec_value[0] = (float)prop->value[0].as_double;
      break;

    case ParameterType::Boolean:
      rna->integer_vec_value[0] = (int)prop->value[0].as_int;
      break;

    case ParameterType::String:
      strncpy(rna->string_value,
              prop->value[0].as_char,
              MOD_OPENMFX_MAX_STRING_VALUE);
      break;

    default:
      std::cerr << "-- Skipping parameter " << rna->name
                << " (unsupported type: " << rna->type << ")"
                << std::endl;
      break;
  }
}

void MFX_copy_parameter_minmax_to_rna(int rna_type,
                                  int & int_rna,
                                  float & float_rna,
                                  const OfxPropertyStruct *prop)
{
  switch (static_cast<ParameterType>(rna_type)) {
    case ParameterType::Integer3d:
    case ParameterType::Integer2d:
    case ParameterType::Integer:
      int_rna = prop->value[0].as_int;
      break;

    case ParameterType::Rgba:
    case ParameterType::Double3d:
    case ParameterType::Rgb:
    case ParameterType::Double2d:
    case ParameterType::Double:
      float_rna = static_cast<float>(prop->value[0].as_double);
      break;

    case ParameterType::Boolean:
      int_rna = prop->value[0].as_int;
      break;
  }
}

void MFX_copy_parameter_value_to_rna(OpenMfxParameter *rna, const OfxParamHandle param)
{
  rna->type = static_cast<int>(param->type);
  switch (param->type) {
    case ParameterType::Integer3d:
      rna->integer_vec_value[2] = param->value[2].as_int;
    case ParameterType::Integer2d:
      rna->integer_vec_value[1] = param->value[1].as_int;
    case ParameterType::Integer:
      rna->integer_vec_value[0] = param->value[0].as_int;
      break;

    case ParameterType::Rgba:
      rna->float_vec_value[3] = (float)param->value[3].as_double;
    case ParameterType::Double3d:
    case ParameterType::Rgb:
      rna->float_vec_value[2] = (float)param->value[2].as_double;
    case ParameterType::Double2d:
      rna->float_vec_value[1] = (float)param->value[1].as_double;
    case ParameterType::Double:
      rna->float_vec_value[0] = (float)param->value[0].as_double;
      break;

    case ParameterType::Boolean:
      rna->integer_vec_value[0] = (int)param->value[0].as_int;
      break;

    case ParameterType::String:
      strncpy(rna->string_value, param->value[0].as_char, MOD_OPENMFX_MAX_STRING_VALUE);
      break;

    default:
      std::cerr << "-- Skipping parameter " << rna->name << " (unsupported type: " << rna->type
                << ")" << std::endl;
      break;
  }
}