#include <stdio.h>

#include "printer.h"
#include "token.h"

#define ADD_SPACE cvec_append(print_buffer, ' ')
#define ADD_NEWLINE cvec_append(print_buffer, '\n');
#define ADD_LPAREN cvec_append(print_buffer, '(');
#define ADD_RPAREN cvec_append(print_buffer, ')');

char *token_output[] = { "ARRAY '", "ASSERT '", "BOOL '", "COLON '", "COMMA '", "DOT '", "ELSE '", "END_OF_FILE",
                        "EQUALS '", "FALSE '", "FLOAT '", "FLOATVAL '", "FN '", "IF '", "IMAGE '", "INT '", "INTVAL '",
                        "LCURLY '",  "LET '", "LPAREN '", "LSQUARE '", "NEWLINE", "OP '", "PRINT '", "RCURLY '", 
                        "READ '", "RETURN '", "RPAREN '", "RSQUARE '", "SHOW '", "STRING '", "STRUCT '", "SUM '", 
                        "THEN '", "TIME '", "TO '", "TRUE '", "VARIABLE '", "VOID '", "WRITE '" };

char *cmd_output[] = { "(ReadCmd ", "(WriteCmd ", "(LetCmd ", "(AssertCmd ", "(PrintCmd ", "(ShowCmd ", "(TimeCmd ", "(FnCmd ", "(StructCmd "};
int cmd_lengths[] = { 9, 10, 8, 11, 10, 9, 9, 7, 11 };

char *expr_output[] = { "(IntExpr ", "(FloatExpr ", "(TrueExpr", "(FalseExpr", "(VarExpr ", "(VoidExpr", "(ArrayLiteralExpr", "(StructLiteralExpr ",
                        "(DotExpr ", "(ArrayIndexExpr ", "(CallExpr ", "(UnopExpr ", "(BinopExpr ", "(IfExpr ", "(ArrayLoopExpr", "(SumLoopExpr" };
int expr_lengths[] = { 9, 11, 9, 10, 9, 9, 17, 19, 9, 16, 10, 10, 11, 8, 14, 12 };

char *lvalue_output[] = { "(VarLValue ", "(ArrayLValue "};
int lvalue_lengths[] = { 11, 13 };

char *stmt_output[] = { "(LetStmt ", "(AssertStmt ", "(ReturnStmt " };
int stmt_lengths[] = { 9, 12, 12 };

char *type_output[] = { "(IntType", "(BoolType", "(FloatType", "(ArrayType ", "(StructType ", "(VoidType" };
int type_lengths[] = { 8, 9, 10, 11, 12, 9 };

static CVec *print_buffer;
static NodeVec *node_list;
static TokenVec *token_list;

void print_tokens(TokenVec *vector) {
    if (!vector) return;
    Token *token;
    size_t num_tokens = sizeof(token_output)/sizeof(char*);
    print_buffer = cvec_create_cap(vector->size << 8); if (!print_buffer) return;
    
    char *token_string;
    for (size_t i = 0; i < vector->size; ++i) {
        token = tokenvec_get(vector, i);
        if (token->type >= num_tokens) {
            fprintf(stderr, "Token string index out of bounds.\n");
            exit(EXIT_FAILURE);
        }
        token_string = token_output[token->type];
        
        switch (token->type) {
            case NEWLINE:
            case END_OF_FILE:
                cvec_append_array_line(print_buffer, token_string, strlen(token_string));
                break;
            default:
                cvec_append_array(print_buffer, token_string, strlen(token_string));
                cvec_append_ref(print_buffer, token->strref);
                cvec_append_array(print_buffer, "\'\n", 2);
        }
    }
    cvec_print(print_buffer);
    cvec_destroy(print_buffer);
}

void print_nodes(NodeVec *nodes, Vector *commands, TokenVec *tokens) {
    if (!nodes || !commands) return;

    node_list = nodes;
    token_list = tokens;

    print_buffer = cvec_create_cap(nodes->size << 8); if (!print_buffer) return;

    for (size_t i = 0; i < commands->size; ++i) {
        uint64_t command_index = (uint64_t) vector_get(commands, i);
        print_command(nodevec_get(nodes, command_index));
        ADD_NEWLINE;
    }

    cvec_append(print_buffer, '\n');
    cvec_print(print_buffer);
    cvec_destroy(print_buffer);
}

void print_command(AstNode *cmd) {
    char *header = cmd_output[cmd->type.cmd];

    cvec_append_array(print_buffer, header, cmd_lengths[cmd->type.cmd]);
    Vector *list;

    switch (cmd->type.cmd) {
        case READ_CMD:
            cvec_append_ref(print_buffer, cmd->string);
            ADD_SPACE;
            print_lvalue(nodevec_get(node_list, cmd->field1.node));
            break;
        case WRITE_CMD:
            print_expression(nodevec_get(node_list, cmd->field1.node));
            ADD_SPACE;
            cvec_append_ref(print_buffer, cmd->string);
            break;
        case LET_CMD:
            print_lvalue(nodevec_get(node_list, cmd->field1.node));
            ADD_SPACE;
            print_expression(nodevec_get(node_list, cmd->field2.node));
            break;
        case ASSERT_CMD:
            print_expression(nodevec_get(node_list, cmd->field1.node));
            ADD_SPACE;
            cvec_append_ref(print_buffer, cmd->string);
            break;
        case SHOW_CMD:
            print_expression(nodevec_get(node_list, cmd->field1.node));
            break;
        case PRINT_CMD:
            cvec_append_ref(print_buffer, cmd->string);
            break;
        case TIME_CMD:
            print_command(nodevec_get(node_list, cmd->field1.node));
            break;
        case FN_CMD:
            cvec_append_ref(print_buffer, cmd->string);
            ADD_SPACE;
            ADD_LPAREN;
            ADD_LPAREN;
            list = cmd->field1.list;
            for (size_t i = 0; i < list->size; ++i) { 
                if (i) ADD_SPACE;
                print_binding(nodevec_get(node_list, (uint64_t) vector_get(list, i)));
            }
            vector_destroy_light(list);
            ADD_RPAREN;
            ADD_RPAREN;
            ADD_SPACE;
            print_type(nodevec_get(node_list, cmd->field2.node));
            list = cmd->field3.list;
            for (size_t i = 0; i < list->size; ++i) {
                ADD_SPACE;
                print_statement(nodevec_get(node_list, (uint64_t) vector_get(list, i)));
            }
            vector_destroy_light(list);
            break;
        case STRUCT_CMD:
            cvec_append_ref(print_buffer, cmd->string);
            list = cmd->field1.list;
            for (size_t i = 0; i < list->size; ++i) {
                ADD_SPACE;
                AstNode *bind = nodevec_get(node_list, (uint64_t) vector_get(list, i));
                AstNode *lvalue = nodevec_get(node_list, bind->field1.node);
                cvec_append_ref(print_buffer, lvalue->string);
                ADD_SPACE;
                print_type(nodevec_get(node_list, bind->field2.node));
            }
            vector_destroy_light(list);
            break;
        default:
            return;
    }

    ADD_RPAREN;
}

void print_lvalue(AstNode *lvalue) {
    char *header = lvalue_output[lvalue->type.lvalue];
    cvec_append_array(print_buffer, header, lvalue_lengths[lvalue->type.lvalue]);

    switch (lvalue->type.lvalue) {
        case VAR_LVALUE:
            cvec_append_ref(print_buffer, lvalue->string);
            break;
        case ARRAY_LVALUE:
            cvec_append_ref(print_buffer, lvalue->string);
            Vector *list = lvalue->field1.list;
            for (size_t i = 0; i < list->size; ++i) {
                ADD_SPACE;
                cvec_append_ref(print_buffer, tokenvec_get(token_list, (uint64_t) vector_get(list, i))->strref);
            }
            vector_destroy_light(list);
            break;
        default:
            return;
    }

    ADD_RPAREN;
}

void print_expression(AstNode *expr) {
    char *header = expr_output[expr->type.expr];
    cvec_append_array(print_buffer, header, expr_lengths[expr->type.expr]);

    char buffer[30];
    Vector *list;
    Vector *list2;
    switch (expr->type.expr) {
        case INT_EXPR:
            if (sprintf(buffer, "%ld", expr->field1.int_value) < 0)
                return;
            cvec_append_array(print_buffer, buffer, strlen(buffer));
            break;
        case FLOAT_EXPR:
            if (sprintf(buffer, "%ld", (uint64_t) expr->field1.float_value) < 0)
                return;
            cvec_append_array(print_buffer, buffer, strlen(buffer));
            break;
        case TRUE_EXPR:
        case FALSE_EXPR:
        case VOID_EXPR:
            break;
        case VAR_EXPR:
            cvec_append_ref(print_buffer, expr->string);
            break;
        case ARRAYLITERAL_EXPR:
            list = expr->field1.list;
            for (size_t i = 0; i < list->size; ++i) {
                ADD_SPACE;
                print_expression(nodevec_get(node_list, (size_t) vector_get(list, i)));
            }
            vector_destroy_light(list);
            break;
        case STRUCTLITERAL_EXPR:
            cvec_append_ref(print_buffer, expr->string);
            list = expr->field1.list;
            for (size_t i = 0; i < list->size; ++i) {
                ADD_SPACE;
                print_expression(nodevec_get(node_list, (size_t) vector_get(list, i)));
            }
            vector_destroy_light(list);
            break;
        case DOT_EXPR:
            print_expression(nodevec_get(node_list, expr->field1.node));
            ADD_SPACE;
            cvec_append_ref(print_buffer, expr->string);
            break;
        case ARRAYINDEX_EXPR:
            print_expression(nodevec_get(node_list, expr->field1.node));
            list = expr->field2.list;
            for (size_t i = 0; i < list->size; ++i) {
                ADD_SPACE;
                print_expression(nodevec_get(node_list, (size_t) vector_get(list, i)));
            }
            vector_destroy_light(list);
            break;
        case CALL_EXPR:
            cvec_append_ref(print_buffer, expr->string);
            list = expr->field1.list;
            for (size_t i = 0; i < list->size; ++i) {
                ADD_SPACE;
                print_expression(nodevec_get(node_list, (size_t) vector_get(list, i)));
            }
            vector_destroy_light(list);
            break;
        case UNOP_EXPR:
            cvec_append_ref(print_buffer, expr->string);
            ADD_SPACE;
            print_expression(nodevec_get(node_list, expr->field1.node));
            break;
        case BINOP_EXPR:
            print_expression(nodevec_get(node_list, expr->field1.node));
            ADD_SPACE;
            cvec_append_ref(print_buffer, expr->string);
            ADD_SPACE;
            print_expression(nodevec_get(node_list, expr->field2.node));
            break;
        case IF_EXPR:
            print_expression(nodevec_get(node_list, expr->field1.node));
            ADD_SPACE;
            print_expression(nodevec_get(node_list, expr->field2.node));
            ADD_SPACE;
            print_expression(nodevec_get(node_list, expr->field3.node));
            break;
        case ARRAYLOOP_EXPR:
            // Flow
        case SUMLOOP_EXPR:
            list = expr->field1.list;
            list2 = expr->field2.list;
            for (size_t i = 0; i < list->size; ++i) {
                ADD_SPACE;
                cvec_append_ref(print_buffer, tokenvec_get(token_list, (uint64_t) vector_get(list, i))->strref);
                ADD_SPACE;
                print_expression(nodevec_get(node_list, (size_t) vector_get(list2, i)));
            }
            ADD_SPACE;
            print_expression(nodevec_get(node_list, expr->field3.node));
            break;
        default:
            return;
    }

    ADD_RPAREN;
}

void print_type(AstNode *type) {
    char *header = type_output[type->type.type];
    cvec_append_array(print_buffer, header, type_lengths[type->type.type]);

    char buffer[20];
    switch (type->type.type) {
        case INT_TYPE:
        case FLOAT_TYPE:
        case BOOL_TYPE:
        case VOID_TYPE:
            break;
        case ARRAY_TYPE:
            print_type(nodevec_get(node_list, type->field2.node));
            ADD_SPACE;
            if (sprintf(buffer, "%ld", type->field1.int_value) < 0)
                return;
            cvec_append_array(print_buffer, buffer, strlen(buffer));
            break;
        case STRUCT_TYPE:
            cvec_append_ref(print_buffer, type->string);
            break;
        default:
            return;
    }

    ADD_RPAREN;
}

void print_statement(AstNode *stmt) {
    char *header = stmt_output[stmt->type.stmt];
    cvec_append_array(print_buffer, header, stmt_lengths[stmt->type.stmt]);

    switch (stmt->type.stmt) {
        case LET_STMT:
            print_lvalue(nodevec_get(node_list, stmt->field1.node));
            ADD_SPACE;
            print_expression(nodevec_get(node_list, stmt->field2.node));
            break;
        case ASSERT_STMT:
            print_expression(nodevec_get(node_list, stmt->field1.node));
            ADD_SPACE;
            cvec_append_ref(print_buffer, stmt->string);
            break;
        case RETURN_STMT:
            print_expression(nodevec_get(node_list, stmt->field1.node));
            break;
        default:
            return;
    }

    ADD_RPAREN;
}

void print_binding(AstNode *bind) {
    print_lvalue(nodevec_get(node_list, bind->field1.node));
    ADD_SPACE;
    print_type(nodevec_get(node_list, bind->field2.node));
}
