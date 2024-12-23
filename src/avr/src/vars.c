#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/pgmspace.h>

#include "vars.h"
#include "config.h"
#include "type.h"
#include "status.h"
#include "hardware.h"
#include "axis.h"
#include "cpp_magic.h"
#include "report.h"
#include "command.h"
#include "base64.h"

// Variable info
typedef struct {
  type_t type;
  uint8_t index;
  void *get;
  void *set;
  uint8_t code;
} var_info_t;

// Function prototypes
static bool type_eq(type_t type, const type_u* a, const type_u* b);
static void type_set(type_t type, uint8_t index, void* ptr, const type_u* value);
static type_u type_get(type_t type, uint8_t index, const void* ptr);
static bool _find_var(const char* name, var_info_t* info);
static void _report_var_value(const char* name, var_info_t* info, bool* reported);
static bool decode_type(const char *value, type_t type, type_u *v);
static bool _is_valid_value(type_t type, const type_u *value);

// Variable handlers
typedef bool (*var_handler_t)(const char* name, var_info_t* info);

// Type definitions
typedef uint8_t u8_t;
typedef int8_t s8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef int32_t s32_t;
typedef float f32_t;
typedef bool b8_t;
typedef char* str_t;
typedef const char* pstr_t;

// Forward declare all handlers and init functions
#define VAR(NAME, CODE, TYPE, ARGS, REPORT, PERSIST) \
  static TYPE##_t NAME##_state; \
  static void NAME##_init(void); \
  static bool NAME##_handler(const char* name, var_info_t* info);
#include "vars.def"
#undef VAR

// Variable codes
#define VAR(NAME, CODE, TYPE, ARGS, REPORT, PERSIST) \
  static uint8_t var_code_##CODE = __COUNTER__;
#include "vars.def"
#undef VAR

// Format strings
static const char code_fmt[] PROGMEM = { '"', '%', 's', ':', '\0' };
static const char indexed_code_fmt[] PROGMEM = { '"', '%', 'c', '%', 's', ':', '\0' };
static const char u8_fmt[] PROGMEM = { '%', 'u', '\0' };
static const char s8_fmt[] PROGMEM = { '%', 'd', '\0' };
// Report vars
static uint8_t _report_var[32] = {0,};

// Bit manipulation functions
static bool _get_report_var(uint8_t index) {
  return _report_var[index >> 3] & (1 << (index & 7));
}

static void _set_report_var(uint8_t index, bool enable) {
  if (enable) _report_var[index >> 3] |= 1 << (index & 7);
  else _report_var[index >> 3] &= ~(1 << (index & 7));
}

// Variable initialization and handler implementation
#define VAR(NAME, CODE, TYPE, ARGS, REPORT, PERSIST) \
  static void NAME##_init() { \
    NAME##_state = 0; \
  } \
  static bool NAME##_handler(const char* name, var_info_t* info) { \
    if (strcmp_P(name, PSTR(#CODE))) return false; \
    info->type = TYPE_##TYPE; \
    info->index = 0; \
    info->get = &NAME##_state; \
    info->set = &NAME##_state; \
    info->code = REPORT; \
    return true; \
  }
#include "vars.def"
#undef VAR

// Variable handlers array
static const var_handler_t var_handlers[] PROGMEM = {
#define VAR(NAME, CODE, TYPE, ARGS, REPORT, PERSIST) NAME##_handler,
#include "vars.def"
#undef VAR
};

// Name resolver
static const char *_resolve_name(const char *_name) {
  static char name[5];
  strncpy(name, _name, sizeof(name) - 1);
  name[sizeof(name) - 1] = 0;
  return name;
}

// Find variable by name
static bool _find_var(const char *_name, var_info_t *info) {
  const char *name = _resolve_name(_name);
  if (!name) return false;

  // Find var
  var_handler_t handler;
  for (uint8_t i = 0; i < sizeof(var_handlers) / sizeof(var_handler_t); i++) {
    memcpy_P(&handler, &var_handlers[i], sizeof(handler));
    if (handler(name, info)) return true;
  }

  return false;
}

void vars_init() {
  memset(_report_var, 0, sizeof(_report_var));

#define VAR(NAME, CODE, TYPE, ARGS, REPORT, PERSIST) NAME##_init();
#include "vars.def"
#undef VAR
}

void vars_report(bool full) {
  bool reported = false;
  fputs_P(PSTR("{"), stdout);

  for (uint8_t i = 0; i < sizeof(var_handlers) / sizeof(var_handler_t); i++) {
    var_info_t info;
    var_handler_t handler;
    memcpy_P(&handler, &var_handlers[i], sizeof(handler));
    
    // Get handler name from var_handlers array
    char name[5] = {0};
    snprintf(name, sizeof(name), "%d", i);
    
    if (handler(name, &info) && (full || _get_report_var(info.code)))
      _report_var_value(name, &info, &reported);
  }

  fputs_P(PSTR("}"), stdout);
}

void vars_report_all(bool enable) {
#define VAR(NAME, CODE, TYPE, INDEX, SET, REPORT, ...)                  \
  _set_report_var(var_code_##CODE, enable);
#include "vars.def"
#undef VAR
}

type_u get_var(const char* name) {
  var_info_t info;
  if (!_find_var(name, &info)) return (type_u){0};
  return type_get(info.type, info.index, info.get);
}

bool set_var(const char* name, const type_u* value) {
  var_info_t info;
  if (!_find_var(name, &info) || !info.set) return false;
  type_set(info.type, info.index, info.set, value);
  return true;
}

bool vars_equal(const char* name, const type_u* value) {
  var_info_t info;
  if (!_find_var(name, &info)) return false;
  type_u current = get_var(name);
  return type_eq(info.type, value, &current);
}

bool var_exists(const char* name) {
  var_info_t info;
  return _find_var(name, &info);
}

// Add validation helper
static bool _is_valid_value(type_t type, const type_u *value) {
    if (!value) return false;
    
    switch (type) {
        case TYPE_str:
        case TYPE_pstr:
            return value->_str != NULL;
        default:
            return true; // Other types are valid by default
    }
}

// Print type value
static void type_print(type_t type, const type_u *value) {
    if (!_is_valid_value(type, value)) {
        printf_P(PSTR("null"));
        return;
    }

    switch (type) {
        case TYPE_u8: printf_P(u8_fmt, value->_u8); break;
        case TYPE_s8: printf_P(s8_fmt, value->_s8); break;
        case TYPE_u16: printf_P(PSTR("%u"), value->_u16); break;
        case TYPE_u32: printf_P(PSTR("%lu"), value->_u32); break;
        case TYPE_s32: printf_P(PSTR("%ld"), value->_s32); break;
        case TYPE_f32: printf_P(PSTR("%f"), value->_f32); break;
        case TYPE_b8: printf_P(PSTR("%d"), value->_b8); break;
        case TYPE_str: printf_P(PSTR("\"%s\""), value->_str); break;
        case TYPE_pstr: printf_P(PSTR("\"%S\""), value->_str); break;
    }
}

// Report variable value
static void _report_var_value(const char* name, var_info_t* info, bool* reported) {
  if (!info->get) return;
  
  if (*reported) fputc(',', stdout);
  *reported = true;

  if (info->index) {
    char code[5];
    strncpy(code, name, sizeof(code));
    code[sizeof(code) - 1] = 0;
    fprintf_P(stdout, PSTR("\"%s%u\":"), code, info->index);
  } else fprintf_P(stdout, PSTR("\"%s\":"), name);

  type_u value = type_get(info->type, info->index, info->get);
  type_print(info->type, &value);
}

// Type equality comparison
static bool type_eq(type_t type, const type_u* a, const type_u* b) {
  switch (type) {
    case TYPE_u8: return a->_u8 == b->_u8;
    case TYPE_s8: return a->_s8 == b->_s8;
    case TYPE_u16: return a->_u16 == b->_u16;
    case TYPE_u32: return a->_u32 == b->_u32;
    case TYPE_s32: return a->_s32 == b->_s32;
    case TYPE_f32: return a->_f32 == b->_f32;
    case TYPE_b8: return a->_b8 == b->_b8;
    case TYPE_str: return !strcmp(a->_str, b->_str);
    case TYPE_pstr: return !strcmp_P(a->_str, b->_str);
  }
  return false;
}

// Get typed value
static type_u type_get(type_t type, uint8_t index, const void* ptr) {
  type_u value = {0};
  switch (type) {
    case TYPE_u8: value._u8 = ((uint8_t*)ptr)[index]; break;
    case TYPE_s8: value._s8 = ((int8_t*)ptr)[index]; break;
    case TYPE_u16: value._u16 = ((uint16_t*)ptr)[index]; break;
    case TYPE_u32: value._u32 = ((uint32_t*)ptr)[index]; break;
    case TYPE_s32: value._s32 = ((int32_t*)ptr)[index]; break;
    case TYPE_f32: value._f32 = ((float*)ptr)[index]; break;
    case TYPE_b8: value._b8 = ((bool*)ptr)[index]; break;
    case TYPE_str: value._str = ((char**)ptr)[index]; break;
    case TYPE_pstr: value._str = ((const char**)ptr)[index]; break;
  }
  return value;
}

// Set typed value
static void type_set(type_t type, uint8_t index, void* ptr, const type_u* value) {
  switch (type) {
    case TYPE_u8: ((uint8_t*)ptr)[index] = value->_u8; break;
    case TYPE_s8: ((int8_t*)ptr)[index] = value->_s8; break;
    case TYPE_u16: ((uint16_t*)ptr)[index] = value->_u16; break;
    case TYPE_u32: ((uint32_t*)ptr)[index] = value->_u32; break;
    case TYPE_s32: ((int32_t*)ptr)[index] = value->_s32; break;
    case TYPE_f32: ((float*)ptr)[index] = value->_f32; break;
    case TYPE_b8: ((bool*)ptr)[index] = value->_b8; break;
    case TYPE_str: strcpy(((char*)ptr), value->_str); break;
    case TYPE_pstr: strcpy_P((char*)ptr, value->_str); break;
  }
}

void vars_print_json() {
  bool reported = false;
  for (uint8_t i = 0; i < sizeof(var_handlers) / sizeof(var_handler_t); i++) {
    var_info_t info;
    var_handler_t handler;
    memcpy_P(&handler, &var_handlers[i], sizeof(handler));
    
    // Get handler name from var_handlers array
    char name[5] = {0};
    snprintf(name, sizeof(name), "%d", i);
    
    if (handler(name, &info)) {
      _report_var_value(name, &info, &reported);
    }
  }
}

// Helper function to decode type values
static bool decode_type(const char *value, type_t type, type_u *v) {
  switch (type) {
    case TYPE_u8: return b64_decode(value, strlen(value), (uint8_t *)&v->_u8);
    case TYPE_s8: return b64_decode(value, strlen(value), (uint8_t *)&v->_s8);
    case TYPE_u16: return b64_decode(value, strlen(value), (uint8_t *)&v->_u16);
    case TYPE_u32: return b64_decode(value, strlen(value), (uint8_t *)&v->_u32);
    case TYPE_s32: return b64_decode(value, strlen(value), (uint8_t *)&v->_s32);
    case TYPE_f32: return b64_decode_float(value, &v->_f32);
    case TYPE_b8: {
      uint8_t tmp;
      if (!b64_decode(value, strlen(value), &tmp)) return false;
      v->_b8 = tmp;
      return true;
    }
    case TYPE_str:
    case TYPE_pstr:
      v->_str = (char *)value;
      return true;
  }
  return false;
}

stat_t command_report(char *cmd) {
  report_request_full();
  return STAT_OK;
}

stat_t command_var(char *cmd) {
  // Parse variable name and value
  char *name = cmd + 1;
  char *value = strchr(name, '=');
  if (!value) return STAT_INVALID_COMMAND;
  *value++ = 0;

  // Get variable info
  var_info_t info;
  if (!_find_var(name, &info)) return STAT_UNRECOGNIZED_NAME;

  // Parse and set value
  type_u v = {0};
  if (!decode_type(value, info.type, &v)) return STAT_INVALID_VALUE;
  if (!info.set) return STAT_READ_ONLY;
  type_set(info.type, info.index, info.set, &v);

  return STAT_OK;
}

stat_t command_sync_var(char *cmd) {
  return command_var(cmd);
}

unsigned command_sync_var_size() {
  return sizeof(var_info_t) + sizeof(type_u);
}

void command_sync_var_exec(void *data) {
  var_info_t *info = (var_info_t *)data;
  type_u *value = (type_u *)(info + 1);
  type_set(info->type, info->index, info->set, value);
}
