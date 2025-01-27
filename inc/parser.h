#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "vector.h"
#include "token.h"

#define IS_COMMAND(t) (t == READ || t == WRITE || t == LET || t == ASSERT || t == PRINT || t == SHOW || t == TIME)
#define IS_EXPR(t) (t == INTVAL || t == FLOATVAL || t == TRUE || t == FALSE || t == VARIABLE || t == LSQUARE)

typedef enum { READ_CMD, WRITE_CMD, LET_CMD, ASSERT_CMD, PRINT_CMD, SHOW_CMD, TIME_CMD } CmdType;
typedef enum { INT_EXPR, FLOAT_EXPR, TRUE_EXPR, FALSE_EXPR, VAR_EXPR, ARRAY_EXPR } ExprType;
typedef enum { VAR_ARG } ArgType;
typedef enum { VAR_LVALUE } LValueType;
typedef enum { VAR_TYPE } TypeType;

typedef struct {
    ArgType type;
    char *string;
} Arg;

typedef struct {
    LValueType type;
    char *string;
} LValue;

typedef struct {
    TypeType type;
    char *string;
} Type;

typedef struct Expr {
    ExprType type;
    union {
        int64_t int_value;
        double float_value;
        char *string;
        struct Expr *expr;
        Vector *expr_list;
    } field1;
} Expr;

typedef struct Cmd {
    CmdType type;
    union {
        char *string;
        struct Cmd *cmd;
        Expr *expr;
        LValue *lvalue;
    } field1;

    union {
        char *string;
        Expr *expr;
        LValue *lvalue;
    } field2;
} Cmd;

int parse();

TokenType peek_token(uint64_t);
int expect_token(uint64_t, TokenType, char**);

int parse_cmd(uint64_t*, Cmd*);
int parse_expr(uint64_t*, Expr*);
int parse_arg(uint64_t*, Arg*);
int parse_lvalue(uint64_t*, LValue*);
int parse_type(uint64_t*, Type*);

int parse_readcmd(uint64_t*, Cmd*);
int parse_writecmd(uint64_t*, Cmd*);
int parse_letcmd(uint64_t*, Cmd*);
int parse_assertcmd(uint64_t*, Cmd*);
int parse_printcmd(uint64_t*, Cmd*);
int parse_showcmd(uint64_t*, Cmd*);
int parse_timecmd(uint64_t*, Cmd*);

int parse_intexpr(uint64_t*, Expr*);
int parse_floatexpr(uint64_t*, Expr*);
int parse_trueexpr(uint64_t*, Expr*);
int parse_falseexpr(uint64_t*, Expr*);
int parse_varexpr(uint64_t*, Expr*);
int parse_arrayexpr(uint64_t*, Expr*);

int parse_vararg(uint64_t*, Arg*);

int parse_varlvalue(uint64_t*, LValue*);

int parse_vartype(uint64_t*, Type*);

char *cmd_string(Cmd*);
char *cmd_string1(Cmd*);
char *cmd_string2(Cmd*);

char *expr_string(Expr*);
char *expr_string1(Expr*);
char *array_expr_string(Expr*);

char *arg_string(Arg*);
char *lvalue_string(LValue*);
char *type_string(Type*);

char *string_dup(char*);

void parse_print_output();

void free_command_list();

void free_cmd(Cmd*);
void free_expr(Expr*);
void free_arg(Arg*);
void free_lvalue(LValue*);
void free_type(Type*);

char *combine_strings(size_t, ...);
#endif // PARSER_H
