/* lexer.h --- lexing functions for the 16candles compiler.
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

#ifndef C16_LEXER_H
#define C16_LEXER_H

#include "../16common/common/arch.h"
#include "verbose.h"

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <ctype.h>
#include <assert.h>



typedef c16_halfword valtype_t;

#define INVALID_PARAM 0xff // Represents that this parameter was not parsed
                           // correctly.
#define NO_PARAM      0x00 // Represents that there is no parameter for the
                           // given parameter slot.
#define REG_PARAM     0x01 // Represents the parameter holds a register or
                           // subregister.
#define MEMADDR_PARAM 0x02 // Represents the parameter holds a memory address.
#define WORDLIT_PARAM 0x03 // Represents the parameter holds a word litteral.
#define MEMREG_PARAM  0x04 // Represents the parameter holds a memory address
                           // that is held in a register.

// The format of the parameters holding data, and a type.
typedef struct{
    c16_word  data; // The value of this parameter.
    valtype_t type; // Represents the type that the parameter is.
    char     *str;  // Represents an error with this param.
} c16_param;

// An expression to be read.
typedef struct{
    c16_opcode op;        // The instruction.
    c16_param *param_1;   // First parameter (optional - no_param).
    c16_param *param_2;   // Second Parameter (optional - no_param).
    c16_param *param_3;   // Result location (optional - no_param).
    bool       is_valid;  // Is this expression completely valid.
    char      *str_data;  // String data if needed.
} c16_expr;

// A label for jumps.
typedef struct{
    char    *name;
    c16_word loc;
} c16_label;

// The single pointer for a non existnant parameter.
extern c16_param *no_param;

// Reads the first expression out of the string.
// Like strtok, pass NULL to take the next expression out of the last string
// passed
c16_expr *get_expr(char*);

// Parse a param from a string. Identifies if it is a litteral, memory addess,
// register, subregister, or invalid type.
c16_param *parse_param(char*);

// Parses a string to a c16_word, if the string is invalid, it will return 0 and
// set invalid_word to true.
c16_word strtoword(char*);

// Reads the next parameter, this function will account for strtok return null
// prematurly.
c16_param *next_param();

// Takes a command an finds looks up the proper opcode for it or returns
// OP_INVALID if it is not a valid command.
c16_opcode parse_opcode(char*);

// Specializes a general expression.
void specialize_expr(c16_expr*);

// Checks to see if a param is valid, and adjusts the c16_expr accordingly.
void check_param(c16_expr*,c16_param*,bool);

// Frees a c16_param, freeing the data only if it holds an error string.
void free_param(c16_param*);

// Frees an c16_expr. Checks to make sure it only frees the no_param c16_param
// once.
void free_expr(c16_expr*);

#endif
