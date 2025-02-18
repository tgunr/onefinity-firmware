/******************************************************************************\
                 This file is part of the Buildbotics firmware.
\******************************************************************************/

#include "var_handlers.h"
#include "vars.h"
#include "cpp_magic.h"
#include "config.h"
#include <string.h>
#include <avr/pgmspace.h>

// Forward declarations of get/set functions
#define VAR(NAME, CODE, TYPE, INDEX, SET, ...)                          \
  extern type_##TYPE get_##NAME(int8_t index);                         \
  IF(SET)(extern void set_##NAME(int8_t index, type_##TYPE value);)

#include "vars.def"
#undef VAR

// Helper function to find index in a string
static int _index(char c, const char *s) {
  const char *index = strchr(s, c);
  return index ? index - s : -1;
}

// Define the handler functions
#define VAR(NAME, CODE, TYPE, INDEX, SET, ...)                          \
  bool handle_##NAME(const char *name, var_info_t *info) {             \
    const char *code = INDEX ? name + 1 : name;                         \
    if (strcmp_P(code, PSTR(#CODE))) return false;                      \
    if (INDEX) {                                                       \
      int i = _index(name[0], INDEX == 1 ? AXES_LABEL :               \
                     INDEX == 2 ? MOTORS_LABEL :                       \
                     INDEX == 3 ? OUTS_LABEL :                         \
                     INDEX == 4 ? ANALOG_LABEL :                       \
                     INDEX == 5 ? VFDREG_LABEL : "");                  \
      if (i == -1) return false;                                       \
      info->index = i;                                                 \
    } else info->index = -1;                                           \
    info->type = TYPE_##TYPE;                                          \
    info->get = (void *)get_##NAME;                                    \
    info->set = NULL;                                                  \
    return true;                                                       \
  }

#include "vars.def"
#undef VAR
