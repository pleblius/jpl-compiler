#ifndef PROD_H
#define PROD_H

#include <stdint.h>

#include "vector.h"
#include "token.h"

#define MALLOC_FAILURE exit(EXIT_FAILURE)

typedef enum { READ_CMD, WRITE_CMD, LET_CMD, ASSERT_CMD, PRINT_CMD, SHOW_CMD, TIME_CMD, FN_CMD, STRUCT_CMD } CmdType;
typedef enum { INT_EXPR, FLOAT_EXPR, TRUE_EXPR, FALSE_EXPR, VAR_EXPR, ARRAY_EXPR, VOID_EXPR, STRUCTLITERAL_EXPR, DOT_EXPR, ARRAYINDEX_EXPR, CALL_EXPR } ExprType;
typedef enum { VAR_ARG } ArgType;
typedef enum { VAR_LVALUE, ARRAY_LVALUE } LValueType;
typedef enum { INT_TYPE, FLOAT_TYPE, BOOL_TYPE, ARRAY_TYPE, VOID_TYPE, STRUCT_TYPE } TypeType;
typedef enum { LET_STMT, ASSERT_STMT, RETURN_STMT } StmtType;
typedef enum { VAR_BIND } BindType;

typedef struct {
    ArgType type;
    char *string;
} Arg;

typedef struct {
    LValueType type;
    union {
        char *string;
    } field1;

    union {
        Vector *var_list;
    } field2;
} LValue;

typedef struct Type {
    TypeType type;
    
    union {
        char *string;
        struct Type *type;
    } field1;

    uint16_t comma_count;
} Type;

typedef struct {
    BindType type;
    LValue *field1;
    Type *field2;
} Bind;

typedef struct Expr {
    ExprType type;
    union {
        int64_t int_value;
        double float_value;
        char *string;
        struct Expr *expr;
        Vector *expr_list;
    } field1;
    
    union {
        char *string;
        Vector *expr_list;
    } field2;
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
        Vector *binds;
        Vector *vars;
    } field2;

    union {
        Type *type;
        Vector *types;
    } field3;

    union {
        Vector *stmts;
    } field4;
} Cmd;

typedef struct {
    StmtType type;
    
    union {
        LValue *lvalue;
        Expr *expr;
    } field1;

    union {
        Expr *expr;
        char *string;
    } field2;
} Stmt;

int parse_cmd(uint64_t*, Cmd*);
int parse_expr(uint64_t*, Expr*);
int parse_arg(uint64_t*, Arg*);
int parse_lvalue(uint64_t*, LValue*);
int parse_type(uint64_t*, Type*);
int parse_stmt(uint64_t*, Stmt*);
int parse_bind(uint64_t*, Bind*);

int parse_readcmd(uint64_t*, Cmd*);
int parse_writecmd(uint64_t*, Cmd*);
int parse_letcmd(uint64_t*, Cmd*);
int parse_assertcmd(uint64_t*, Cmd*);
int parse_printcmd(uint64_t*, Cmd*);
int parse_showcmd(uint64_t*, Cmd*);
int parse_timecmd(uint64_t*, Cmd*);
int parse_fncmd(uint64_t*, Cmd*);
int parse_structcmd(uint64_t*, Cmd*);
int parse_fnbindings(uint64_t*, Vector*);
int parse_fnstmts(uint64_t*, Vector*);
int parse_structmembers(uint64_t*, Vector*, Vector*);

int parse_intexpr(uint64_t*, Expr*);
int parse_floatexpr(uint64_t*, Expr*);
int parse_trueexpr(uint64_t*, Expr*);
int parse_falseexpr(uint64_t*, Expr*);
int parse_varexpr(uint64_t*, Expr*);
int parse_arrayliteralexpr(uint64_t*, Expr*);
int parse_voidexpr(uint64_t*, Expr*);
int parse_dotexpr(uint64_t*, Expr*);
int parse_arrayindexexpr(uint64_t*, Expr*);
int parse_callexpr(uint64_t*, Expr*);
int parse_structliteralexpr(uint64_t*, Expr*);

int parse_exprlist(uint64_t*, Vector*, TokenType);

int parse_vararg(uint64_t*, Arg*);

int parse_varlvalue(uint64_t*, LValue*);
int parse_arraylvalue(uint64_t*, LValue*);

int parse_arraytype(uint64_t*, Type*);

int parse_inttype(uint64_t*, Type*);
int parse_floattype(uint64_t*, Type*);
int parse_booltype(uint64_t*, Type*);
int parse_voidtype(uint64_t*, Type*);
int parse_structtype(uint64_t*, Type*);

int parse_letstmt(uint64_t*, Stmt*);
int parse_assertstmt(uint64_t*, Stmt*);
int parse_returnstmt(uint64_t*, Stmt*);

char *cmd_string(Cmd*);
char *cmd_stmt_string(Cmd*);
char *cmd_bind_string(Cmd*);
char *cmd_struct_string(Cmd*);

char *expr_string(Expr*);
char *exprlist_string(Vector*);

char *arg_string(Arg*);
char *type_string(Type*);
char *bind_string(Bind*);
char *stmt_string(Stmt*);

char *lvalue_string(LValue*);
char *lvaluelist_string(Vector*);

void free_cmd(Cmd*);
void free_statements(Vector*);
void free_bindings(Vector*);
void free_types(Vector*);

void free_expr(Expr*);
void free_arg(Arg*);
void free_lvalue(LValue*);
void free_type(Type*);
void free_stmt(Stmt*);
void free_bind(Bind*);

void free_exprlist(Vector*);
void free_lvaluelist(Vector*);
#endif // PROD_H
