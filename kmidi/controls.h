/*

    TiMidity -- Experimental MIDI to WAVE converter
    Copyright (C) 1995 Tuukka Toivonen <toivonen@clinet.fi>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

    controls.h
*/

/* Return values for ControlMode.read */

#include "constants.h"

typedef struct {
  char *id_name, id_character;
  int verbosity, trace_playing, opened;

  int (*open)(int using_stdin, int using_stdout);
  void (*pass_playing_list)(int number_of_files, char *list_of_files[]);
  void (*close)(void);
  int (*read)(int32 *valp);
  int (*cmsg)(int type, int verbosity_level, char *fmt, ...);

  void (*refresh)(void);
  void (*reset)(void);
  void (*file_name)(char *name);
  void (*total_time)(int tt);
  void (*current_time)(int ct);

  void (*note)(int v);
  void (*master_volume)(int mv);
  void (*program)(int channel, int val); /* val<0 means drum set -val */
  void (*volume)(int channel, int val);
  void (*expression)(int channel, int val);
  void (*panning)(int channel, int val);
  void (*sustain)(int channel, int val);
  void (*pitch_bend)(int channel, int val);
  
} ControlMode;

extern ControlMode *ctl_list[], *ctl; 
