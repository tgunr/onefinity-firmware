/******************************************************************************\

                 This file is part of the Buildbotics firmware.

                   Copyright (c) 2015 - 2018, Buildbotics LLC
                              All rights reserved.

      This file ("the software") is free software: you can redistribute it
      and/or modify it under the terms of the GNU General Public License,
       version 2 as published by the Free Software Foundation. You should
       have received a copy of the GNU General Public License, version 2
      along with the software. If not, see <http://www.gnu.org/licenses/>.

      The software is distributed in the hope that it will be useful, but
           WITHOUT ANY WARRANTY; without even the implied warranty of
       MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
                Lesser General Public License for more details.

        You should have received a copy of the GNU Lesser General Public
                 License along with the software.  If not, see
                        <http://www.gnu.org/licenses/>.

                 For information regarding this software email:
                   "Joseph Coffland" <joseph@buildbotics.com>

\******************************************************************************/

#pragma once

#include "type.h"
#include "cpp_magic.h"

// Index constants
#define INDEX_AXES 1
#define INDEX_MOTORS 2
#define INDEX_OUTS 3
#define INDEX_ANALOG 4
#define INDEX_VFDREG 5

// Variable types
typedef uint8_t type_b8;
typedef uint8_t type_u8;
typedef uint16_t type_u16;
typedef uint32_t type_u32;
typedef int32_t type_s32;
typedef float type_f32;
typedef const char *type_str;
typedef const char *type_pstr;

// Variable declarations
#define VAR(NAME, CODE, TYPE, INDEX, SET, ...) \
  extern type_##TYPE get_##NAME(int8_t index); \
  IF(SET)(extern void set_##NAME(int8_t index, type_##TYPE value);)

#include "vars.def"
#undef VAR

float var_decode_float(const char *value);
bool var_parse_bool(const char *value);

void vars_init();

void vars_report(bool full);
void vars_report_all(bool enable);
void vars_report_var(const char *code, bool enable);
stat_t vars_print(const char *name);
stat_t vars_set(const char *name, const char *value);
void vars_print_json();
