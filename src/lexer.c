/* lexer.c --- lexing functions for the 16candles compiler.
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

#include "lexer.h"

static bool invalid_word = false;

c16_param *no_param;

// Takes the first expression from a string.
// Pass null to take the next expression out of the string.
c16_expr *get_expr(char *str){
    char *in = (str) ? str : NULL;
    c16_expr *ret = malloc(sizeof(c16_expr));
    char *tok;
    static const char* const bin_ops[][2] = { { "and",   "&&" },
                                              { "or",    "||" },
                                              { "xand",  "!&" },
                                              { "xor",   "!|" },
                                              { "lshift","<<" },
                                              { "rshift",">>" },
                                              { "add",   "+"  },
                                              { "sub",   "-"  },
                                              { "mul",   "*"  },
                                              { "div",   "/"  },
                                              { "mod",   "%"  },
                                              { "min",   ""   },
                                              { "max",   ""   } };
    static const char* const un_ops[][2] =  { { "inv",   "~"  },
                                              { "inc",   "++" },
                                              { "dec",   "--" },
                                              { "set",   "="  },
                                              { "swap",  "\\\\" },
                                              { "gt",    ">"  },
                                              { "lt",    "<"  },
                                              { "gte",   ">=" },
                                              { "lte",   "<=" },
                                              { "eq",    "==" },
                                              { "neq",   "!=" } };
    static const char* const nrp_ops[][2] = { { "push",  ":"  },
                                              { "write", ""   },
                                              { "pop",   "$"  },
                                              { "peek",  "@"  },
                                              { "read",  ""   } };
    static const char* const fr_ops[][2] =  { { "flush", "#"  },
                                              { "halt",  ""   },
                                              { "nop",   ""   } };
    static const char* const jmp_ops[][2] = { { "jmp",   "=>" },
                                              { "jmpt",  "->" },
                                              { "jmpf",  "<-" } };
    int n;
    ret->is_valid = true;
    tok = strtok(in," \n");
    if (!tok){
        ret->op       = OP_TERM;
        ret->param_1  = no_param;
        ret->param_2  = no_param;
        ret->param_3  = no_param;
        ret->str_data = NULL;
        return ret;
    }
    if (tok[0] == '@' && strlen(tok) > 1){
        ret->op       = OP_LABEL;
        ret->param_1  = no_param;
        ret->param_2  = no_param;
        ret->param_3  = no_param;
        ret->str_data = &tok[1];
        v_msgf("Adding label to expr list: '%s'\n",tok);
        return ret;
    }
    if (!strcmp(tok,"mset") || !strcmp(tok,":=")){
        ret->op = OP_MSET_;
        ret->param_1  = next_param();
        ret->param_2  = next_param();
        ret->param_3  = no_param;
        ret->str_data = NULL;
        specialize_expr(ret);
        return ret;
    }
    for (n = 0;n < 13;n++){
        if (!strcmp(tok,bin_ops[n][0])
            || !strcmp(tok,bin_ops[n][1])){
            ret->op       = parse_opcode(tok);
            ret->param_1  = next_param();
            ret->param_2  = next_param();
            ret->param_3  = next_param();
            ret->str_data = NULL;
            specialize_expr(ret);
            return ret;
        }
    }
    for (n = 0;n < 11;n++){
        if (!strcmp(tok,un_ops[n][0])
            || !strcmp(tok,un_ops[n][1])){
            ret->op       = parse_opcode(tok);
            ret->param_1  = next_param();
            ret->param_2  = next_param();
            ret->param_3  = no_param;
            ret->str_data = NULL;
            specialize_expr(ret);
            return ret;
        }
    }
    for (n = 0;n < 5;n++){
        if (!strcmp(tok,nrp_ops[n][0])
            || !strcmp(tok,nrp_ops[n][1])){
            ret->op       = parse_opcode(tok);
            ret->param_1  = next_param();
            ret->param_2  = no_param;
            ret->param_3  = no_param;
            ret->str_data = NULL;
            specialize_expr(ret);
            return ret;
        }
    }
    for (n = 0;n < 3;n++){
        if (!strcmp(tok,fr_ops[n][0])
            || !strcmp(tok,fr_ops[n][1])){
            ret->op       = parse_opcode(tok);
            ret->param_1  = no_param;
            ret->param_2  = no_param;
            ret->param_3  = no_param;
            ret->str_data = NULL;
            specialize_expr(ret);
            return ret;
        }
    }
    for (n = 0;n < 3;n++){
        if (!strcmp(tok,jmp_ops[n][0])
            || !strcmp(tok,jmp_ops[n][1])){
            ret->op            = parse_opcode(tok);
            ret->param_1       = malloc(sizeof(c16_param));
            ret->param_1->type = WORDLIT_PARAM;
            ret->param_1->data = 0;
            ret->param_2       = no_param;
            ret->param_3       = no_param;
            ret->str_data      = strdup(strtok(NULL," \n"));
            return ret;
        }
    }
    v_msgf("Read an unreckognized token: '%s'\n",tok);
    no_param = malloc(sizeof(c16_param));
    no_param->type = NO_PARAM;
    no_param->str  = NULL;
    ret->op        = OP_INVALID;
    ret->param_1   = no_param;
    ret->param_2   = no_param;
    ret->param_3   = no_param;
    ret->str_data  = strdup(tok);
    specialize_expr(ret);
    return ret;
}

void specialize_expr(c16_expr *expr){
    char *buffer;
    if (expr->op == OP_INVALID){
        expr->is_valid = false;
        return;
    }
    if (expr->op <= OP_MAX_){
        check_param(expr,expr->param_1,false);
        check_param(expr,expr->param_2,false);
        check_param(expr,expr->param_3,true);
        if (expr->param_1->type == WORDLIT_PARAM){
            if (expr->param_2->type == WORDLIT_PARAM){
                expr->op += LIT_LIT;
            }else{
                expr->op += LIT_REG;
            }
        }else{
            if (expr->param_2->type == WORDLIT_PARAM){
                expr->op += REG_LIT;
            }else{
                expr->op += REG_REG;
            }
        }
        return;
    }
    if (expr->op <= OP_LT_){
        check_param(expr,expr->param_1,false);
        check_param(expr,expr->param_2,false);
        if (expr->param_1->type == WORDLIT_PARAM){
            if (expr->param_2->type == WORDLIT_PARAM){
                expr->op += LIT_LIT;
            }else{
                expr->op += LIT_REG;
            }
        }else{
            if (expr->param_2->type == WORDLIT_PARAM){
                expr->op += REG_LIT;
            }else{
                expr->op += REG_REG;
            }
        }
        return;
    }
    if (expr->op <= OP_SET_){
        check_param(expr,expr->param_1,false);
        check_param(expr,expr->param_2,true);;
        if (expr->param_1->type == WORDLIT_PARAM){
            expr->op += LIT;
        }else{
            expr->op += REG;
        }
        return;
    }
    if (expr->op <= OP_WRITE_){
        check_param(expr,expr->param_1,false);
        if (expr->param_1->type == WORDLIT_PARAM){
            expr->op += LIT;
        }else{
            expr->op += REG;
        }
        return;
    }
    if (expr->op == OP_MSET_){
        if (expr->param_1->type == MEMADDR_PARAM){
            check_param(expr,expr->param_2,true);
            expr->op = OP_MSET_MEMADDR;
        }else if (expr->param_1->type == MEMREG_PARAM){
            check_param(expr,expr->param_2,true);
            expr->op = OP_MSET_MEMREG;
        }else{
            if (expr->param_1->type != NO_PARAM){
                if (expr->param_1->type == WORDLIT_PARAM){
                    if (expr->param_2->type == MEMADDR_PARAM){
                        expr->op = OP_MSET_LIT_MEMADDR;
                        return;
                    }else if(expr->param_2->type == MEMREG_PARAM){
                        expr->op = OP_MSET_LIT_MEMREG;
                        return;
                    }
                }else if (expr->param_1->type == REG_PARAM){
                    if (expr->param_2->type == MEMADDR_PARAM){
                        expr->op = OP_MSET_REG_MEMADDR;
                        return;
                    }else if (expr->param_2->type == MEMREG_PARAM){
                        expr->op = OP_MSET_REG_MEMREG;
                        return;
                    }
                }else{
                    expr->is_valid = false;
                }
            }
            return;
        }
    }
    if (expr->op == OP_SWAP){
        check_param(expr,expr->param_1,true);
        check_param(expr,expr->param_2,true);
        return;
    }
    if (expr->op == OP_POP || expr->op == OP_PEEK || expr->op == OP_READ){
        check_param(expr,expr->param_1,true);
    }
}

// Checks is a param is valid.
void check_param(c16_expr *expr,c16_param *param,const bool only_register){
    if ((!only_register && param->type == WORDLIT_PARAM)
        || param->type == REG_PARAM){
        return;
    }
    expr->op = OP_INVALID;
    if (param->type == NO_PARAM){
        v_msgf("Missing parameter");
        param->str = malloc(64 * sizeof(char));
        snprintf(param->str,64,"Invalid parameter: 'missing': expected "
                 "parameter of type register%s",param->data,
                 (only_register) ? "." : "or word litteral.");
        return;
    }
    v_msgf("Read invalid parameter: \"%x\"\n",param->data);
    param->type = INVALID_PARAM;
    param->str = malloc(64 * sizeof(char));
    snprintf(param->str,64,"Invalid parameter: '%s': expected "
             "parameter of type register%s",param->data,
             (only_register) ? "." : "or word litteral.");
}

// Parses a parameter out of a string.
c16_param *parse_param(char* str){
    c16_param *ret = malloc(sizeof(c16_param));
    c16_word wordlit;
    static char *rs[] = { "ipt","spt","ac1","tst","inp","ac2",
                          "r0","r1","r2","r3","r4","r5","r6","r7","r8","r9",
                          "inp_r","inp_w","r0_f","r0_b",
                          "r1_f","r1_b","r2_f","r2_b","r3_f","r3_b",
                          "r4_f","r4_b","r5_f","r5_b","r6_f","r6_b",
                          "r7_f","r7_b","r8_f","r8_b","r9_f","r9_b" };
    static c16_opreg ts[] = { OP_ipt,OP_spt,OP_ac1,OP_tst,OP_inp,OP_ac2,
                              OP_r0,OP_r1,OP_r2,OP_r3,OP_r4,OP_r5,OP_r6,OP_r7,
                              OP_r8,OP_r9,OP_inp_r,OP_inp_w,OP_r0_f,
                              OP_r0_b,OP_r1_f,OP_r1_b,OP_r2_f,OP_r2_b,OP_r3_f,
                              OP_r3_b,OP_r4_f,OP_r4_b,OP_r5_f,OP_r5_b,OP_r6_f,
                              OP_r6_b,OP_r7_f,OP_r7_b,OP_r8_f,OP_r8_b,OP_r9_f,
                              OP_r9_b };
    int n;
    if (str[0] == '*'){
        for (n = 0;n < 38;n++){
            if (!strcmp(&str[1],rs[n])){
                ret->data = ts[n];
                ret->type = MEMREG_PARAM;
                return ret;
            }
        }
        wordlit = strtoword(&str[1]);
        if (invalid_word){
            ret->type = INVALID_PARAM;
            ret->str = strdup(str);
            return ret;
        }
        ret->type = MEMADDR_PARAM;
        ret->data = wordlit;
        return ret;
    }
    for (n = 0;n < 38;n++){
        if (!strcmp(str,rs[n])){
            ret->data = ts[n];
            ret->type = REG_PARAM;
            return ret;
        }
    }
    wordlit = strtoword(str);
    if (invalid_word){
        ret->type = INVALID_PARAM;
        ret->str = strdup(str);
        return ret;
    }
    ret->type = WORDLIT_PARAM;
    ret->data = wordlit;
    return ret;
}

// Converts a string into a c16_word.
// returns 0 if it is invalid and sets invalid_word to true;
c16_word strtoword(char *str){
    invalid_word = false;
    int n;
    if (!strncmp(str,"0X",2) || !strncmp(str,"0x",2)){
        for (n = 2;n < strlen(str);n++){
            if (!isxdigit(str[n])){
                invalid_word = true;
                return 0;
            }
        }
        return (c16_word) strtoul(str,NULL,16);
    }
    for (n = 0;n < strlen(str);n++){
        if (!isdigit(str[n])){
            invalid_word = true;
            return 0;
        }
    }
    return (c16_word) strtoul(str,NULL,10);
}

// Takes the next parameter from the file, and accounts for reading EOF
// unexpectedly.
c16_param *next_param(){
    char *tok;
    c16_param *ret;
    tok = strtok(NULL," \n");
    if (!tok || tok[0] == '#'){
        ret = malloc(sizeof(c16_param));
        ret->type = INVALID_PARAM;
        ret->str = strdup("Expected additional parameter.");
        return ret;
    }
    return parse_param(tok);
}

// Detirmines the opcode of the command passed.
c16_opcode parse_opcode(char* str){
    static char *ops[][2] = { { "and",   "&&"   },
                              { "or",    "||"   },
                              { "xand",  "!&"   },
                              { "xor",   "!|"   },
                              { "inv",   "~"    },
                              { "lshift","<<"   },
                              { "rshift",">>"   },
                              { "add",   "+"    },
                              { "sub",   "-"    },
                              { "mul",   "*"    },
                              { "div",   "/"    },
                              { "mod",   "%"    },
                              { "inc",   "++"   },
                              { "dec",   "--"   },
                              { "gt",    ">"    },
                              { "lt",    "<"    },
                              { "gte",   ">="   },
                              { "lte",   "<="   },
                              { "eq",    "=="   },
                              { "neq",   "!="   },
                              { "min",   ""     },
                              { "max",   ""     },
                              { "jmp",   "=>"   },
                              { "jmpt",  "->"   },
                              { "jmpf",  "<-"   },
                              { "push",  ":"    },
                              { "pop",   "$"    },
                              { "peek",  "@"    },
                              { "flush", "#"    },
                              { "set",   "="    },
                              { "mset",  ":="   },
                              { "swap" , "\\\\" },
                              { "halt",  ""     },
                              { "nop",   ""     },
                              { "read",  ""     },
                              { "write", ""     } };
    static c16_opcode opcodes[] = { OP_AND_,
                                    OP_OR_,
                                    OP_XAND_,
                                    OP_XOR_,
                                    OP_INV_,
                                    OP_LSHIFT_,
                                    OP_RSHIFT_,
                                    OP_ADD_,
                                    OP_SUB_,
                                    OP_MUL_,
                                    OP_DIV_,
                                    OP_MOD_,
                                    OP_INC_,
                                    OP_DEC_,
                                    OP_GT_,
                                    OP_LT_,
                                    OP_GTE_,
                                    OP_LTE_,
                                    OP_EQ_,
                                    OP_NEQ_,
                                    OP_MIN_,
                                    OP_MAX_,
                                    OP_JMP,
                                    OP_JMPT,
                                    OP_JMPF,
                                    OP_PUSH_,
                                    OP_POP,
                                    OP_PEEK,
                                    OP_FLUSH,
                                    OP_SET_,
                                    OP_MSET_,
                                    OP_SWAP,
                                    OP_HALT,
                                    OP_NOP,
                                    OP_READ,
                                    OP_WRITE_ };
    int n;
    for (n = 0;n < 36;n++){
        if ((!strcmp(str,ops[n][0]) || !strcmp(str,ops[n][1]))){
            return opcodes[n];
        }
    }
    return OP_INVALID;
}


// Frees a param unless it is no_param;
void free_param(c16_param *param){
    if (param->type == NO_PARAM){
        return;
    }
    if (param->type == INVALID_PARAM){
        v_msgf("invalid param: '%s'\n",param->str);
        free(param->str);
    }
    free(param);
}

// Frees an expression by verifying it only frees no_param once.
void free_expr(c16_expr *expr){
    free_param(expr->param_1);
    free_param(expr->param_2);
    free_param(expr->param_3);
    free(expr);
}
