#ifndef AST_H
#define AST_H

typedef enum { READ_CMD, WRITE_CMD, LET_CMD, ASSERT_CMD, PRINT_CMD, SHOW_CMD, TIME_CMD } CmdType;
typedef enum { INT_EXPR, FLOAT_EXPR, TRUE_EXPR, FALSE_EXPR, VAR_EXPR, ARRAY_LITERAL_EXPR } ExprType;
typedef enum { VAR_LVALUE } LValueType;
typedef enum { VAR_ARG } ArgType;
typedef enum { CMD_PROD, EXPR_RPOD, LVALUE_PROD, ARG_PROD } ProdType;

typedef struct {
    ProdType type;
    union sub_type {
        CmdType cmd_type;
        ExprType expr_type;
        LValueType lvalue_type;
        ArgType arg_type;
    } sub_type;

    void *field1;
    void *field2;
} ASTNode;

#endif // AST_H
