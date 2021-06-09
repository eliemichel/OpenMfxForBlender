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

#include "mfxConvert.h"
#include "mfxHost.h"

#include <iostream>
#include <cstring>

void copy_parameter_value_from_rna(OfxParamHandle param, const OpenMeshEffectParameter *rna)
{
  param->type = static_cast<ParamType>(rna->type);
  switch (rna->type) {
    case PARAM_TYPE_INTEGER_3D:
      param->value[2].as_int = rna->integer_vec_value[2];
    case PARAM_TYPE_INTEGER_2D:
      param->value[1].as_int = rna->integer_vec_value[1];
    case PARAM_TYPE_INTEGER:
      param->value[0].as_int = rna->integer_vec_value[0];
      break;

    case PARAM_TYPE_RGBA:
      param->value[3].as_double = (double)rna->float_vec_value[3];
    case PARAM_TYPE_DOUBLE_3D:
    case PARAM_TYPE_RGB:
      param->value[2].as_double = (double)rna->float_vec_value[2];
    case PARAM_TYPE_DOUBLE_2D:
      param->value[1].as_double = (double)rna->float_vec_value[1];
    case PARAM_TYPE_DOUBLE:
      param->value[0].as_double = (double)rna->float_vec_value[0];
      break;

    case PARAM_TYPE_BOOLEAN:
      param->value[0].as_bool = rna->integer_vec_value[0];
      break;

    case PARAM_TYPE_STRING:
      param->realloc_string(MOD_OPENMESHEFFECT_MAX_STRING_VALUE);
      strncpy(param->value[0].as_char, rna->string_value, MOD_OPENMESHEFFECT_MAX_STRING_VALUE);
      break;

    default:
      std::cerr << "-- Skipping parameter " << param->name
                << " (unsupported type: " << param->type << ")"
                << std::endl;
      break;
  }
}

void copy_parameter_value_to_rna(OpenMeshEffectParameter *rna, const OfxPropertyStruct *prop)
{
  switch (rna->type) {
    case PARAM_TYPE_INTEGER_3D:
      rna->integer_vec_value[2] = prop->value[2].as_int;
    case PARAM_TYPE_INTEGER_2D:
      rna->integer_vec_value[1] = prop->value[1].as_int;
    case PARAM_TYPE_INTEGER:
      rna->integer_vec_value[0] = prop->value[0].as_int;
      break;

    case PARAM_TYPE_RGBA:
      rna->float_vec_value[3] = (float)prop->value[3].as_double;
    case PARAM_TYPE_DOUBLE_3D:
    case PARAM_TYPE_RGB:
      rna->float_vec_value[2] = (float)prop->value[2].as_double;
    case PARAM_TYPE_DOUBLE_2D:
      rna->float_vec_value[1] = (float)prop->value[1].as_double;
    case PARAM_TYPE_DOUBLE:
      rna->float_vec_value[0] = (float)prop->value[0].as_double;
      break;

    case PARAM_TYPE_BOOLEAN:
      rna->integer_vec_value[0] = (int)prop->value[0].as_int;
      break;

    case PARAM_TYPE_STRING:
      strncpy(rna->string_value,
              prop->value[0].as_char,
              MOD_OPENMESHEFFECT_MAX_STRING_VALUE);
      break;

    default:
      std::cerr << "-- Skipping parameter " << rna->name
                << " (unsupported type: " << rna->type << ")"
                << std::endl;
      break;
  }
}

void copy_parameter_minmax_to_rna(int rna_type,
                                  int & int_rna,
                                  float & float_rna,
                                  const OfxPropertyStruct *prop)
{
  switch (rna_type) {
    case PARAM_TYPE_INTEGER_3D:
    case PARAM_TYPE_INTEGER_2D:
    case PARAM_TYPE_INTEGER:
      int_rna = prop->value[0].as_int;
      break;

    case PARAM_TYPE_RGBA:
    case PARAM_TYPE_DOUBLE_3D:
    case PARAM_TYPE_RGB:
    case PARAM_TYPE_DOUBLE_2D:
    case PARAM_TYPE_DOUBLE:
      float_rna = static_cast<float>(prop->value[0].as_double);
      break;

    case PARAM_TYPE_BOOLEAN:
      int_rna = prop->value[0].as_int;
      break;
  }
}

void copy_parameter_value_to_rna(OpenMeshEffectParameter *rna, const OfxParamHandle param)
{
  rna->type = static_cast<int>(param->type);
  switch (rna->type) {
    case PARAM_TYPE_INTEGER_3D:
      rna->integer_vec_value[2] = param->value[2].as_int;
    case PARAM_TYPE_INTEGER_2D:
      rna->integer_vec_value[1] = param->value[1].as_int;
    case PARAM_TYPE_INTEGER:
      rna->integer_vec_value[0] = param->value[0].as_int;
      break;

    case PARAM_TYPE_RGBA:
      rna->float_vec_value[3] = (float)param->value[3].as_double;
    case PARAM_TYPE_DOUBLE_3D:
    case PARAM_TYPE_RGB:
      rna->float_vec_value[2] = (float)param->value[2].as_double;
    case PARAM_TYPE_DOUBLE_2D:
      rna->float_vec_value[1] = (float)param->value[1].as_double;
    case PARAM_TYPE_DOUBLE:
      rna->float_vec_value[0] = (float)param->value[0].as_double;
      break;

    case PARAM_TYPE_BOOLEAN:
      rna->integer_vec_value[0] = (int)param->value[0].as_int;
      break;

    case PARAM_TYPE_STRING:
      strncpy(rna->string_value, param->value[0].as_char, MOD_OPENMESHEFFECT_MAX_STRING_VALUE);
      break;

    default:
      std::cerr << "-- Skipping parameter " << rna->name << " (unsupported type: " << rna->type
                << ")" << std::endl;
      break;
  }
}