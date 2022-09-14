#pragma once

#include "pgmspace.h"


typedef enum {
#define STAT_MSG(NAME, TEXT) STAT_##NAME,
#include "messages.def"
#undef STAT_MSG

  STAT_MAX,
  STAT_DO_NOT_EXCEED = 255 // Do not exceed 255
} stat_t;


typedef enum {
  STAT_LEVEL_INFO,
  STAT_LEVEL_DEBUG,
  STAT_LEVEL_WARNING,
  STAT_LEVEL_ERROR,
} status_level_t;


extern stat_t status_code;

const char *status_to_pgmstr(stat_t code);
const char *status_level_pgmstr(status_level_t level);
stat_t status_message_P(const char *location, status_level_t level,
                        stat_t code, const char *msg, ...);

#define TO_STRING(x) _TO_STRING(x)
#define _TO_STRING(x) #x

#define STATUS_LOCATION PSTR(__FILE__ ":" TO_STRING(__LINE__))

#define STATUS_MESSAGE(LEVEL, CODE, MSG, ...)                           \
  status_message_P(STATUS_LOCATION, LEVEL, CODE, PSTR(MSG), ##__VA_ARGS__)

#define STATUS_INFO(MSG, ...)                                   \
  STATUS_MESSAGE(STAT_LEVEL_INFO, STAT_OK, MSG, ##__VA_ARGS__)

#define STATUS_DEBUG(MSG, ...)                                  \
  STATUS_MESSAGE(STAT_LEVEL_DEBUG, STAT_OK, MSG, ##__VA_ARGS__)

#define STATUS_WARNING(CODE, MSG, ...)                          \
  STATUS_MESSAGE(STAT_LEVEL_WARNING, CODE, MSG, ##__VA_ARGS__)

#define STATUS_ERROR(CODE, MSG, ...)                            \
  STATUS_MESSAGE(STAT_LEVEL_ERROR, CODE, MSG, ##__VA_ARGS__)


#ifdef DEBUG
#define DEBUG_CALL(FMT, ...) \
  printf_P(PSTR("%s(" FMT ")\n"), __FUNCTION__, ##__VA_ARGS__)
#else // DEBUG
#define DEBUG_CALL(...)
#endif // DEBUG
