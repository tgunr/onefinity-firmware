/******************************************************************************\
                 This file is part of the Buildbotics firmware.
\******************************************************************************/

#pragma once

#include "type.h"
#include "vars.h"

typedef struct {
  type_t type;
  char name[5];
  int8_t index;
  void *get;
  void *set;
} var_info_t;

// Define handler function prototypes
#define VAR(NAME, CODE, TYPE, INDEX, SET, ...)                          \
  bool handle_##NAME(const char *name, var_info_t *info);

#include "vars.def"
#undef VAR
