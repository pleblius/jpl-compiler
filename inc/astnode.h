#ifndef ASTNODE_H
#define ASTNODE_H

#include <stdint.h>
#include "stringops.h"
#include "vector.h"

typedef enum { READ_CMD, WRITE_CMD, LET_CMD, ASSERT_CMD, PRINT_CMD, SHOW_CMD, TIME_CMD, FN_CMD, STRUCT_CMD } CommandType;
typedef enum { INT_EXPR, FLOAT_EXPR, TRUE_EXPR, FALSE_EXPR, VAR_EXPR, VOID_EXPR, ARRAYLITERAL_EXPR, STRUCTLITERAL_EXPR,
                DOT_EXPR, ARRAYINDEX_EXPR, CALL_EXPR, UNOP_EXPR, BINOP_EXPR, IF_EXPR, ARRAYLOOP_EXPR, SUMLOOP_EXPR } ExpressionType;
typedef enum { VAR_LVALUE, ARRAY_LVALUE } LValueType;
typedef enum { LET_STMT, ASSERT_STMT, RETURN_STMT } StatementType;
typedef enum { INT_TYPE, BOOL_TYPE, FLOAT_TYPE, ARRAY_TYPE, STRUCT_TYPE, VOID_TYPE } TypeType;
typedef enum { BINDING } BindingType;


typedef struct AstNode {
    uint32_t token_index;

    union {
        CommandType cmd;
        ExpressionType expr;
        LValueType lvalue;
        StatementType stmt;
        TypeType type;
        BindingType bind;
    } type;

    union {
        uint64_t int_value;
        double float_value;
        uint64_t node;
        Vector *list;
    } field1;
    union {
        uint64_t node;
        Vector *list;
    } field2;
    union {
        uint64_t node;
        Vector *list;
    } field3;
    union {
        uint64_t node;
        Vector *list;
    } field4;

    StringRef string;
} AstNode;

AstNode create_node(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, StringRef);
AstNode get_empty_node();

#endif // ASTNODE_H
