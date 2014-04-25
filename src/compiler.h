/* compiler.h --- compilation functions for the 16candles compiler.
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

#ifndef C16_COMPILER_H
#define C16_COMPILER_H

#include "common/arch.h"
#include "lexer.h"
#include "verbose.h"

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>


// The type that defines a unit of source code.
typedef struct{
    unsigned int linem;    // Line Max.
    unsigned int linec;    // Line Count.
    char       **linev;    // Line Values.
    bool         is_valid; // Is this source valid in its entirety.
    unsigned int exprm;    // Expressions Max.
    unsigned int exprc;    // Expressions Count.
    c16_expr   **exprv;    // Expressions Values.
    unsigned int bytec;    // Byte Count: Used in label resolution.
    unsigned int labelm;   // Label Max.
    unsigned int labelc;   // Label Count.
    c16_label  **labelv;   // Label Values.
} c16_src;

// An array of halfwords and the number of halfwords to write.
typedef struct{
    c16_halfword *bs;  // The expression in bytecode.
    c16_halfword  len; // The length of the bytecode.
} c16_exprbytes;

// Adds an expression to the source struct.
void add_expr(c16_src*,c16_expr*);

// Adds a label to the source label vector.
void add_label(c16_src*,char*);

// Returns true iff the c16_expr is setup in a valid way.
bool is_valid_expr(c16_expr*);

// Writes the expression to the file.
void write_expr(c16_expr*,FILE*);

// Returns the number of bytes the expression will be in bytecode.
c16_halfword expr_byte_len(c16_expr*);

// Writes the src to the file.
void write_src(c16_src*,char*);

// Allocates a new struct source struct from a file.
c16_src *src_from_file(FILE*);

// Frees a source struct.
void free_src(c16_src*);

// Attempts to compile the source to the out file. Returns the status of
// compilation
bool compile(c16_src*,char*);

#endif
