/* compiler.c --- compilation functions for the 16candles compiler.
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

#include "compiler.h"

int is_verbose;

c16_param *no_param;

// Converts an expression to bytes with a length.
c16_exprbytes *c16_expro_bytes(c16_expr *expr){
    c16_halfword *bytes = malloc(7 * sizeof(c16_halfword));
    c16_exprbytes *arr = malloc(sizeof(c16_exprbytes));
    bytes[0] = expr->op;
    if (expr->param_1->type == REG_PARAM
        || expr->param_1->type == MEMREG_PARAM){
        bytes[1] = (c16_halfword) expr->param_1->data;
        if (expr->param_2->type == REG_PARAM
            || expr->param_2->type == MEMREG_PARAM){
            bytes[2] = (c16_halfword) expr->param_2->data;
            if (expr->param_3->type == REG_PARAM
                || expr->param_3->type == MEMREG_PARAM){
                bytes[3] = (c16_halfword) expr->param_3->data;
            }else{
                bytes[3] = (c16_word) expr->param_3->data >> 8 << 8;
                bytes[4] = (c16_halfword) expr->param_3->data;
            }
        }else{
            bytes[2] = (c16_word) expr->param_2->data >> 8 << 8;
            bytes[3] = (c16_halfword) expr->param_2->data;
            if (expr->param_3->type == REG_PARAM
                || expr->param_3->type == MEMREG_PARAM){
                bytes[4] = (c16_halfword) expr->param_3->data;
            }else{
                bytes[4] = (c16_word) expr->param_3->data >> 8 << 8;
                bytes[5] = (c16_halfword) expr->param_3->data;
            }
        }
    }else{
        bytes[1] = (c16_word) expr->param_1->data >> 8 << 8;
        bytes[2] = (c16_halfword) expr->param_1->data;
        if (expr->param_2->type == REG_PARAM
            || expr->param_2->type == MEMREG_PARAM){
            bytes[3] = (c16_halfword) expr->param_2->data;
            if (expr->param_3->type == REG_PARAM
                || expr->param_3->type == MEMREG_PARAM){
                bytes[4] = (c16_halfword) expr->param_3->data;
            }else{
                bytes[4] = (c16_word) expr->param_3->data >> 8 << 8;
                bytes[5] = (c16_halfword) expr->param_3->data;
            }
        }else{
            bytes[3] = (c16_word) expr->param_2->data >> 8 << 8;
            bytes[4] = (c16_halfword) expr->param_2->data;
            if (expr->param_3->type == REG_PARAM
                || expr->param_3->type == MEMREG_PARAM){
                bytes[5] = (c16_halfword) expr->param_3->data;
            }else{
                bytes[5] = (c16_word) expr->param_3->data >> 8 << 8;
                bytes[6] = (c16_halfword) expr->param_3->data;
            }
        }
    }
    arr->bs = bytes;
    arr->len = expr_byte_len(expr);
    return arr;
}

// Returns the number of bytes expr will be in bytecode.
c16_halfword expr_byte_len(c16_expr *expr){
    if (expr->op <= OP_MAX_REG_REG){
        switch (expr->op % 4){
        case 0:
            return 6;
        case 1:
        case 2:
            return 5;
        case 3:
            return 4;
        }
    }else if (expr->op <= OP_LT_REG_REG){
        switch (expr->op % 4){
        case 0:
            return 5;
        case 1:
        case 2:
            return 4;
        case 3:
            return 3;
        }
    }else if (expr->op <= OP_SET_REG){
        if (expr->op % 2){
            return 3;
        }else{
            return 4;
        }
    }else if (expr->op <= OP_WRITE_REG){
        if (expr->op % 2){
            return 2;
        }else{
            return 3;
        }
    }else if(expr->op == OP_MSET_MEMREG || expr->op == OP_MSET_REG_MEMREG){
        return 3;
    }else if (expr->op == OP_MSET_LIT_MEMADDR || expr->op == OP_SWAP){
        return 5;
    }else if (expr->op == OP_MSET_REG_MEMADDR || expr->op == OP_MSET_MEMADDR
              || expr->op == OP_MSET_LIT_MEMREG){
        return 4;
    }else if (expr->op == OP_POP || expr->op == OP_PEEK || expr->op == OP_READ){
        return 2;
    }else{
        return 1;
    }
}

// Writes an expression to a file.
void write_expr(c16_expr *expr,FILE *out){
    c16_exprbytes *bs;
    int n;
    bs = c16_expro_bytes(expr);
    v_msgf("Writing expr: '%x' of length: '%d\n",expr->op,expr_byte_len(expr));
    for (n = 0;n < bs->len;n++){
        fputc(bs->bs[n],out);
    }
    free(bs->bs);
    free(bs);
}

// Attempts to compile a c16_src to the out file.
bool compile(c16_src *src,char *out_fl){
    unsigned int n,m;
    c16_expr *expr;
    bool found_label;
    no_param       = malloc(sizeof(c16_param));
    no_param->type = NO_PARAM;
    no_param->str  = NULL;
    for (n = 0;n < src->linec;n++){
        if (src->linev[n][0] == '\n' || src->linev[n][0] == '#'){
            continue;
        }
        expr = get_expr(src->linev[n]);
        if (expr->op == OP_LABEL){
            add_label(src,expr->str_data);
            continue;
        }
        if (!expr->is_valid){
            src->is_valid = false;
            printf("Error on line %u: %s\n",n,expr->str_data);
        }
        add_expr(src,expr);
    }
    v_msgf("Encountered EOF, writing in an OP_TERM expr\n");
    expr = malloc(sizeof(c16_expr));
    expr->op = OP_TERM;
    expr->param_1 = no_param;
    expr->param_2 = no_param;
    expr->param_3 = no_param;
    expr->str_data = NULL;
    add_expr(src,expr);
    for (n = 0;n < src->exprc;n++){
        if (src->exprv[n]->op >= OP_JMP && src->exprv[n]->op <= OP_JMPF){
            found_label = false;
            v_msgf("Replacing label: '%s'\n",src->exprv[n]->str_data);
            for (m = 0;m < src->labelc;m++){
                if (!strcmp(src->exprv[n]->str_data,src->labelv[m]->name)){
                    v_msgf("Found match for label: '%s', '%x'\n",
                           src->exprv[n]->str_data,src->labelv[m]->loc);
                    src->exprv[n]->param_1->data = src->labelv[m]->loc;
                    found_label = true;
                    break;
                }
            }
            if (!found_label){
                src->is_valid = false;
                printf("Error: Unable to resolve label: '%s'\n",
                       src->exprv[n]->str_data);
            }
        }
    }
    if (src->is_valid){
        v_msgf("Validated source, writing to file.\n");
        write_src(src,out_fl);
    }
    free(no_param);
    return src->is_valid;
}

// Adds an expression to the c16_src vector of expressions.
void add_expr(c16_src *src,c16_expr *expr){
    if (src->exprc == src->exprm){
        v_msgf("Calling realloc on the src->exprv to change size to: '%u'\n",
               2 * src->exprm);
        assert(realloc(src->exprv,2 * src->exprm));
    }
    src->exprv[src->exprc++] = expr;
    src->bytec += expr_byte_len(expr);
}

// Adds a label to the label table vector.
void add_label(c16_src *src,char *name){
    if (src->labelc == src->labelm){
        v_msgf("Calling realloc on the src->labelv to change size to: '%u'\n",
               2 * src->labelm);
        assert(realloc(src->exprv,2 * src->labelm));
    }
    src->labelv[src->labelc] = malloc(sizeof(c16_label));
    src->labelv[src->labelc]->name = name;
    src->labelv[src->labelc++]->loc  = src->bytec;
}

void write_src(c16_src *src,char *out_fl){
    FILE *out = fopen(out_fl,"w");
    int n;
    if (!out){
        fprintf(stderr,"Error: Unable to open file '%s'\n.",out_fl);
        exit(-1);
    }
    for (n = 0;n < src->exprc;n++){
        if (src->is_valid){
            write_expr(src->exprv[n],out);
        }
        free_expr(src->exprv[n]);
    }
    free_src(src);
}

// Reads a c16_src from file. On invalid file, returns NULL;
c16_src *src_from_file(FILE *in){
    if (!in){
        return NULL;
    }
    char cur_line[64];
    c16_src *ret  = malloc(sizeof(c16_src));
    ret->linem    = 128;
    ret->linec    = 0;
    ret->linev    = malloc(128 * sizeof(char*));
    ret->is_valid = true;
    ret->exprm    = 64;
    ret->exprc    = 0;
    ret->exprv    = malloc(64 * sizeof(c16_expr*));
    ret->bytec    = 0;
    ret->labelm   = 64;
    ret->labelc   = 0;
    ret->labelv   = malloc(64 * sizeof(c16_label*));
    while (fgets(cur_line,64,in)){
        if (ret->linec == ret->linem){
            ret->linem *= 2;
            assert(realloc(ret->linev,ret->linem));
        }
        ret->linev[ret->linec++] = strndup(cur_line,64);
    }
    return ret;
}

void free_src(c16_src *src){
    int n;
    for (n = 0;n < src->linec;n++){
        free(src->linev[n]);
    }
    free(src->linev);
    free(src->exprv);
}

static char *help_msg =
    "Usage:\n\n    16cc [OPTION]... [SOURCE]\n\nWhere SOURCE is the .16c \
source file you wish to compile to the file: \"a.out\".\nThe 16candles \
compiler accecpts the following additional arguments:\n\n    -v --verbose   \
Prints additional information during the compilation process,\n               \
    the default is to output only errors.\n    -o [OUTPUT]    \
The file to name the output binary.\n    -V --version   Print version \
information about the this compiler.\n    -h --help      Prints this \
message.\n";

static char *version_msg =
    "The 16candles compiler: version 0.0.0.1 (2014.1.28)\n\
Copyright (C) 2013 Joe Jevnik and Ted Meyer.\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.";

int main(int argc,char** argv){
    FILE *in,*out = NULL;
    c16_src *src;
    char buffer[80],*out_fl = "a.out";
    int c;
    is_verbose = 0;
    if (argc <= 1){
        puts("Usage: 16cc [OPTION]... [SOURCE]");
        return EXIT_SUCCESS;
    }
    static struct option long_ops[] =
        { { "help",    no_argument,       0,           'h' },
          { "version", no_argument,       0,           'V' },
          { "verbose", no_argument,       &is_verbose, 'v' },
          { 0,         0,                 0,            0  } };
    int opt_index;
    opterr = 0;
    while (1){
        c = getopt_long(argc,argv,"o:OhvV",long_ops,&opt_index);
        if (c == -1){
            break;
        }
        switch(c){
        case 0:
            if (long_ops[opt_index].flag != 0){
                break;
            }
            if (!strcmp("help",long_ops[opt_index].name)){
                puts(help_msg);
                return EXIT_SUCCESS;
            }
            if (!strcmp("helo",long_ops[opt_index].name)){
                puts(version_msg);
                return EXIT_SUCCESS;
            }
        case 'o': // Output file.
            out_fl = optarg;
            break;
        case 'h': // Help.
            puts(help_msg);
            return EXIT_SUCCESS;
        case 'V': // Version.
            puts(version_msg);
            return EXIT_SUCCESS;
        case 'v': // Verbose.
            is_verbose = 1;
            break;
        case 'O': // Optimization TODO.
            break;
        case '?': // Unknown Arg.
            printf("Error: Unrecognized option: '-%c'. See -h for more "
                   "information.\n",optopt);
            return EXIT_FAILURE;
        default:
            return EXIT_FAILURE;
        }
    }
    if (optind >= argc){
        puts("Error: Missing required argument: [SOURCE]");
        return EXIT_FAILURE;
    }
    in = fopen(argv[optind],"r");
    if (!in){
        printf("Error: Cannot open file: %s\n",argv[optind]);
        return EXIT_FAILURE;
    }
    if (!out){
        out = fopen("a.out","w");
    }
    src = src_from_file(in);
    if(!compile(src,out_fl)){
        puts("Oops, something went wrong.");
    }
    return EXIT_SUCCESS;
}
