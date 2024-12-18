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

#include "state.h"

#include "exec.h"
#include "command.h"
#include "stepper.h"
#include "spindle.h"
#include "outputs.h"
#include "jog.h"
#include "estop.h"
#include "seek.h"

#include <stdio.h>

// State strings in program memory
static const char str_estopped[] PROGMEM = { 'E', 'S', 'T', 'O', 'P', 'P', 'E', 'D', '\0' };
static const char str_ready[] PROGMEM = { 'R', 'E', 'A', 'D', 'Y', '\0' };
static const char str_running[] PROGMEM = { 'R', 'U', 'N', 'N', 'I', 'N', 'G', '\0' };
static const char str_jogging[] PROGMEM = { 'J', 'O', 'G', 'G', 'I', 'N', 'G', '\0' };
static const char str_stopping[] PROGMEM = { 'S', 'T', 'O', 'P', 'P', 'I', 'N', 'G', '\0' };
static const char str_holding[] PROGMEM = { 'H', 'O', 'L', 'D', 'I', 'N', 'G', '\0' };
static const char str_invalid[] PROGMEM = { 'I', 'N', 'V', 'A', 'L', 'I', 'D', '\0' };

// Hold reason strings in program memory
static const char str_user_pause[] PROGMEM = { 'U', 's', 'e', 'r', ' ', 'p', 'a', 'u', 's', 'e', '\0' };
static const char str_user_stop[] PROGMEM = { 'U', 's', 'e', 'r', ' ', 's', 't', 'o', 'p', '\0' };
static const char str_program_pause[] PROGMEM = { 'P', 'r', 'o', 'g', 'r', 'a', 'm', ' ', 'p', 'a', 'u', 's', 'e', '\0' };
static const char str_optional_pause[] PROGMEM = { 'O', 'p', 't', 'i', 'o', 'n', 'a', 'l', ' ', 'p', 'a', 'u', 's', 'e', '\0' };
static const char str_switch_found[] PROGMEM = { 'S', 'w', 'i', 't', 'c', 'h', ' ', 'f', 'o', 'u', 'n', 'd', '\0' };

static struct {
  bool flushing;
  bool resuming;
  bool stop_requested;
  bool pause_requested;
  bool unpause_requested;

  state_t state;
  uint16_t state_count;
  hold_reason_t hold_reason;
} s = {
  .flushing = true, // Start out flushing
};


PGM_P state_get_pgmstr(state_t state) {
  switch (state) {
  case STATE_ESTOPPED: return str_estopped;
  case STATE_READY:    return str_ready;
  case STATE_RUNNING:  return str_running;
  case STATE_JOGGING:  return str_jogging;
  case STATE_STOPPING: return str_stopping;
  case STATE_HOLDING:  return str_holding;
  }

  return str_invalid;
}


PGM_P state_get_hold_reason_pgmstr(hold_reason_t reason) {
  switch (reason) {
  case HOLD_REASON_USER_PAUSE:     return str_user_pause;
  case HOLD_REASON_USER_STOP:      return str_user_stop;
  case HOLD_REASON_PROGRAM_PAUSE:  return str_program_pause;
  case HOLD_REASON_OPTIONAL_PAUSE: return str_optional_pause;
  case HOLD_REASON_SWITCH_FOUND:   return str_switch_found;
  }

  return str_invalid;
}


state_t state_get() {return s.state;}


static void _set_state(state_t state) {
  if (s.state == state) return; // No change
  if (s.state == STATE_ESTOPPED) return; // Can't leave EStop state
  s.state = state;
  s.state_count++;
}


static void _set_hold_reason(hold_reason_t reason) {s.hold_reason = reason;}
bool state_is_flushing() {return s.flushing && !s.resuming;}
bool state_is_resuming() {return s.resuming;}


static bool _is_idle() {
  return (state_get() == STATE_READY || state_get() == STATE_HOLDING) &&
    !st_is_busy();
}


void state_seek_hold() {
  if (state_get() == STATE_RUNNING) {
    _set_hold_reason(HOLD_REASON_SWITCH_FOUND);
    _set_state(STATE_STOPPING);
  }
}


static void _stop() {
  _set_hold_reason(HOLD_REASON_USER_STOP);

  switch (state_get()) {
  case STATE_RUNNING:
    _set_state(STATE_STOPPING);
    break;

  case STATE_JOGGING:
  case STATE_READY:
  case STATE_HOLDING:
    jog_stop();
    spindle_stop();
    outputs_stop();
    seek_cancel();
    break;

  case STATE_STOPPING:
  case STATE_ESTOPPED:
    break; // Ignore
  }
}


void state_holding() {
  _set_state(STATE_HOLDING);
  if (s.hold_reason == HOLD_REASON_USER_STOP) _stop();
}


void state_running() {
  if (state_get() == STATE_READY) _set_state(STATE_RUNNING);
}


void state_jogging() {
  if (state_get() == STATE_READY || state_get() == STATE_HOLDING)
    _set_state(STATE_JOGGING);
}


void state_idle() {
  if (state_get() == STATE_RUNNING || state_get() == STATE_JOGGING)
    _set_state(STATE_READY);
}


void state_estop() {_set_state(STATE_ESTOPPED);}


void state_callback() {
  if (estop_triggered()) return;

  // Pause
  if (s.pause_requested) {
    if (state_get() == STATE_RUNNING) {
      if (s.pause_requested) _set_hold_reason(HOLD_REASON_USER_PAUSE);
      _set_state(STATE_STOPPING);
    }

    s.pause_requested = false;
  }

  // Stop
  if (s.stop_requested) {
    _stop();
    s.stop_requested = false;
  }

  // Only flush queue when idle (READY or HOLDING)
  if (s.flushing && _is_idle()) {
    command_flush_queue();

    // Resume
    if (s.resuming) s.flushing = s.resuming = false;
  }

  // Unpause
  if (s.unpause_requested && !s.flushing && state_get() != STATE_STOPPING) {
    s.unpause_requested = false;

    if (state_get() == STATE_HOLDING) {
      // Check if any moves are buffered
      if (command_get_count()) _set_state(STATE_RUNNING);
      else _set_state(STATE_READY);
    }
  }
}


// Var callbacks
PGM_P get_state() {return state_get_pgmstr(state_get());}
uint16_t get_state_count() {return s.state_count;}
PGM_P get_hold_reason() {return state_get_hold_reason_pgmstr(s.hold_reason);}


// Command callbacks
stat_t command_pause(char *cmd) {
  pause_t type = (pause_t)(cmd[1] - '0');

  if (type == PAUSE_USER) s.pause_requested = true;
  else command_push(cmd[0], &type);

  return STAT_OK;
}


unsigned command_pause_size() {return sizeof(pause_t);}


void command_pause_exec(void *data) {
  switch (*(pause_t *)data) {
  case PAUSE_PROGRAM_OPTIONAL:
    _set_hold_reason(HOLD_REASON_OPTIONAL_PAUSE);
    break;

  case PAUSE_PROGRAM: _set_hold_reason(HOLD_REASON_PROGRAM_PAUSE); break;
  default: return;
  }

  state_holding();
}


stat_t command_stop(char *cmd) {
  s.stop_requested = true;
  return STAT_OK;
}


stat_t command_unpause(char *cmd) {
  s.unpause_requested = true;
  return STAT_OK;
}


stat_t command_resume(char *cmd) {
  if (s.flushing) s.resuming = true;
  return STAT_OK;
}


stat_t command_flush(char *cmd) {
  s.flushing = true;
  return STAT_OK;
}
