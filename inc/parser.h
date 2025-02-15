#ifndef PARSER_H
#define PARSER_H

#include <stdint.h>

#include "vector.h"
#include "vecs.h"
#include "token.h"
#include "prod.h"

// typedef enum { READ_CMD, WRITE_CMD, LET_CMD, ASSERT_CMD, PRINT_CMD, SHOW_CMD, TIME_CMD, FN_CMD, STRUCT_CMD } CmdType;
// typedef enum { INT_EXPR, FLOAT_EXPR, TRUE_EXPR, FALSE_EXPR, VAR_EXPR, ARRAY_EXPR, VOID_EXPR, 
//                 STRUCTLITERAL_EXPR, DOT_EXPR, ARRAYINDEX_EXPR, CALL_EXPR, UNOP_EXPR, BINOP_EXPR, IF_EXPR,
//                 ARRAY_LOOP_EXPR, SUM_LOOP_EXPR } ExprType;
// typedef enum { VAR_ARG } ArgType;
// typedef enum { VAR_LVALUE, ARRAY_LVALUE } LValueType;
// typedef enum { INT_TYPE, FLOAT_TYPE, BOOL_TYPE, ARRAY_TYPE, VOID_TYPE, STRUCT_TYPE } TypeType;
// typedef enum { LET_STMT, ASSERT_STMT, RETURN_STMT } StmtType;
// typedef enum { VAR_BIND } BindType;

// const TokenType READ_CMD_ARRAY[] = { READ, IMAGE, STRING, TO, LVALUE, END };
// const TokenType WRITE_CMD_ARRAY[] = { WRITE, IMAGE, EXPR, TO, STRING, END };
// const TokenType LET_CMD_ARRAY[] = { LET, LVALUE, EQUALS, EXPR, END };
// const TokenType PRINT_CMD_ARRAY[] = { PRINT, STRING, END };
// const TokenType SHOW_CMD_ARRAY[] = { SHOW, EXPR, END };
// const TokenType TIME_CMD_ARRAY[] = { TIME, CMD, END };
// const TokenType FN_CMD_ARRAY[] = { FN, VARIABLE, LPAREN, BINDS, RPAREN, COLON, LCURLY, NEWLINE, STMTS, RCURLY, END };
// const TokenType STRUCT_CMD_ARRAY[] = { STRUCT, VARIABLE, LCURLY, NEWLINE, STRUCTBINDS, RCURLY, END };

// const TokenType INT_EXPR_ARRAY[] = { INTVAL, END };
// const TokenType FLOAT_EXPR_ARRAY[] = { FLOATVAL, END };
// const TokenType TRUE_EXPR_ARRAY[] = { TRUE, END };
// const TokenType FALSE_EXPR_ARRAY[] = { FALSE, END };
// const TokenType VAR_EXPR_ARRAY[] = { VARIABLE, END };
// const TokenType ARRAY_EXPR_ARRAY[] = { LSQUARE, EXPRS, RSQUARE, END };
// const TokenType VOID_EXPR_ARRAY[] = { VOID, END };
// const TokenType PAREN_EXPR_ARRAY[] = { LPAREN, EXPR, RPAREN, END };
// const TokenType STRUCTLIT_EXPR_ARRAY[] = { VARIABLE, LCURLY, EXPRS, RCURLY, END };
// const TokenType DOT_EXPR_ARRAY[] = { EXPR, DOT, VARIABLE, END };
// const TokenType ARRAYINDEX_EXPR_ARRAY[] = { EXPR, LSQUARE, EXPRS, RSQUARE, END };
// const TokenType CALL_EXPR_ARRAY[] = { VARIABLE, LPAREN, EXPRS, RPAREN, END };
// const TokenType UNOP_EXPR_ARRAY[] = { OP, EXPR, END };
// const TokenType BINOP_EXPR_ARRAY[] = { EXPR, OP, EXPR, END };
// const TokenType IF_EXPR_ARRAY[]  = { IF, EXPR, THEN, EXPR, ELSE, EXPR, END };
// const TokenType ARRAYLOOP_EXPR_ARRAY[] = { ARRAY, LSQUARE, LOOPBINDS, RSQUARE, EXPR, END };
// const TokenType SUMLOOP_EXPR_ARRAY[] = { SUM, LSQUARE, LOOPBINDS, RSQUARE, EXPR, END };

// const TokenType VAR_ARG_ARRAY[] = { VARIABLE, END };

// const TokenType VAR_LVALUE_ARRAY[] = { VARIABLE, END };
// const TokenType ARRAY_LVALUE_ARRAY[] = { VARIABLE, LSQUARE, VARIABLES, RSQUARE, END };

// const TokenType INT_TYPE_ARRAY[] = { INT, END };
// const TokenType FLOAT_TYPE_ARRAY[] = { FLOAT, END };
// const TokenType BOOL_TYPE_ARRAY[] = { BOOL, END };
// const TokenType ARRAY_TYPE_ARRAY[] = { TYPE, LSQUARE, COMMAS, RSQUARE, END };
// const TokenType VOID_TYPE_ARRAY[] = { VOID, END };
// const TokenType STRUCT_TYPE_ARRAY[] = { VARIABLE, END };

// const TokenType LET_STMT_ARRAY[] = { LET, LVALUE, EQUALS, EXPR, END };
// const TokenType ASSERT_STMT_ARRAY[] = { ASSERT, EXPR, COMMA, STRING, END };
// const TokenType RETURN_STMT_ARRAY[] = { RETURN, EXPR, END };

// const TokenType VAR_BIND_ARRAY[] = { LVALUE, COLON, TYPE, END };

// const TokenType STMTS_ARRAY[] = { STMT, NEWLINE, END };
// const TokenType BINDS_ARRAY[] = { BIND, COMMA, END };
// const TokenType STRUCTBINDS_ARRAY[] = { VARIABLE, COLON, TYPE, NEWLINE, END };
// const TokenType EXPRS_ARRAY[] = { EXPR, COMMA, END };
// const TokenType LOOPBINDS_ARRAY[] = { VARIABLE, COLON, EXPR, COMMA, END };
// const TokenType COMMAS_ARRAY[] = { VARIABLE, COLON, EXPR, COMMA, END };

// typedef struct {
//     ArgType type;
//     uint64_t index1;
// } Arg;

// typedef struct {
//     LValueType type;
//     union {
//         StringRef string;
//     } field1;

//     union {
//         Vector *var_list;
//     } field2;
// } LValue;

// typedef struct Type {
//     TypeType type;
    
//     union {
//         StringRef string;
//         struct Type *type;
//     } field1;

//     uint16_t comma_count;
// } Type;

// typedef struct {
//     BindType type;
//     LValue *field1;
//     Type *field2;
// } Bind;

// typedef struct Expr {
//     ExprType type;

//     union {
//         int64_t int_value;
//         double float_value;
//         StringRef string;
//         struct Expr *expr;
//         Vector *expr_list;
//         Vector *var_list;
//     } field1;
    
//     union {
//         StringRef string;
//         Vector *expr_list;
//         struct Expr *expr;
//     } field2;

//     union {
//         struct Expr *expr;
//     } field3;
// } Expr;

// typedef struct Cmd {
//     CmdType type;
//     union {
//         StringRef string;
//         struct Cmd *cmd;
//         Expr *expr;
//         LValue *lvalue;
//     } field1;

//     union {
//         StringRef string;
//         Expr *expr;
//         LValue *lvalue;
//         Vector *binds;
//         Vector *vars;
//     } field2;

//     union {
//         Type *type;
//         Vector *types;
//     } field3;

//     union {
//         Vector *stmts;
//     } field4;
// } Cmd;

// typedef struct {
//     StmtType type;
    
//     union {
//         LValue *lvalue;
//         Expr *expr;
//     } field1;

//     union {
//         Expr *expr;
//         StringRef string;
//     } field2;
// } Stmt;

int parse(TokenVec*);
TokenType peek_token(uint64_t);
int expect_token(uint64_t, TokenType, StringRef*);
void set_fail(int);

#endif // PARSER_H
