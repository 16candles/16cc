/* verbose.h --- The flag and macro to deal with verbose mode
   Copyright (c) 2014 Joe Jevnik

   This program is free software; you can redistribute it and/or modify it
   under the terms of the GNU General Public License as published by the Free
   Software Foundation; either version 2 of the License, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful, but WITHOUT
   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
   FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
   more details.

   You should have received a copy of the GNU General Public License along with
   this program; if not, write to the Free Software Foundation, Inc., 51
   Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA. */

#ifndef VERBOSE_MODE_H
#define VERBOSE_MODE_H

// The flag to print verbose output.
extern int is_verbose;

// Print statments that are only seen while in verbose mode.
#define v_msgf(fmt,...) if (is_verbose) { printf(fmt, ## __VA_ARGS__); }

#endif
