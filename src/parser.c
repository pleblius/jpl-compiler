#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#include "parser.h"
#include "stringops.h"

#define NO_PRECEDENCE -1
#define MIN_PRECEDENCE 0

static int parse_exit_status = EXIT_SUCCESS;
static TokenVec *token_vector;
static NodeVec *node_vector;
static Vector *cmd_list;

extern char *token_names[];

int parse_tokens(TokenVec *tokens, NodeVec **output_nodes, Vector **output_cmds) {
    if (!tokens || !output_cmds || !output_nodes) return EXIT_FAILURE;

    token_vector = tokens;
    node_vector = nodevec_create_cap(token_vector->size);
    cmd_list = vector_create_cap(token_vector->size);
    if (!node_vector || !cmd_list) return EXIT_FAILURE;

    uint32_t index = 0;
    uint64_t cmd_index = 0;
    if (peek_token_type(index) == NEWLINE) ++index;

    while (peek_token_type(index) != END_OF_FILE) {
        if (parse_command(&index, &cmd_index)) {
            vector_append(cmd_list, (void*) cmd_index);
            expect_token(NEWLINE, index++, NULL);
        }
        else {
            TokenType type;
            while (1) {
                type = peek_token_type(index);
                if (type == NEWLINE) {
                    ++index;
                    break;
                } 
                else if (type == END_OF_FILE) break;
                else if (index >= token_vector->size) break;
                ++index;
            }
        }

        if (index >= token_vector->size) break;
    }

    *output_nodes = node_vector;
    *output_cmds = cmd_list;
    return parse_exit_status;
}

int expect_token(TokenType expected_type, uint32_t index, StringRef *output) {
    if (peek_token_type(index) != expected_type) {
        Token *expected = malloc(sizeof(Token)); if (!expected) return 0;
        *expected = (Token) {expected_type, 0, {strlen(token_names[expected_type]), token_names[expected_type]}};

        add_parse_error(UNEXPECTED_TOKEN, tokenvec_get(token_vector, index), expected);
        parse_exit_status = EXIT_FAILURE;
        return 0;
    }

    if (output != NULL) {
        *output = ((Token *) tokenvec_get(token_vector, index))->strref;
    }
    return 1;
}

TokenType peek_token_type(uint32_t index) {
    Token *token = tokenvec_get(token_vector, index);
    if (!token) return INVALID;
    return token->type;
}

void parse_error(ParseErrorType type, uint32_t first_index) {
    if (first_index < token_vector->size)
        add_parse_error(type, tokenvec_get(token_vector, first_index), NULL);
    parse_exit_status = EXIT_FAILURE;   
}

int try_find_next(uint32_t *p_index, TokenType intermediate_token, TokenType end_token) {
    uint32_t index = *p_index;
    TokenType type;

    while (1) {
        type = peek_token_type(index);

        if (type == end_token || type == intermediate_token) break;
        else if (type == NEWLINE || type == END_OF_FILE) {
            *p_index = index;
            return 0;
        }
        else if (index > token_vector->size)
            return 0;
        else
            ++index;
    }

    *p_index = index;
    return 1;
}

int parse_command(uint32_t *p_index, uint64_t *output_index) {
    uint32_t index = *p_index;
    uint64_t output = 0;
    AstNode cmd = get_empty_node();
    
    switch (peek_token_type(index)) {
        case READ:
            cmd.type.cmd = READ_CMD;
            expect_token(READ, index++, NULL);
            expect_token(IMAGE, index++, NULL);
            expect_token(STRING, index++, &cmd.string);
            expect_token(TO, index++, NULL);
            parse_lvalue(&index, &cmd.field1.node);
            break;
        case WRITE:
            cmd.type.cmd = WRITE_CMD;
            expect_token(WRITE, index++, NULL);
            expect_token(IMAGE, index++, NULL);
            parse_expression(&index, &cmd.field1.node, MIN_PRECEDENCE);
            expect_token(TO, index++, NULL);
            expect_token(STRING, index++, &cmd.string);
            break;
        case LET:
            cmd.type.cmd = LET_CMD;
            expect_token(LET, index++, NULL);
            parse_lvalue(&index, &cmd.field1.node);
            expect_token(EQUALS, index++, NULL);
            parse_expression(&index, &cmd.field2.node, MIN_PRECEDENCE);
            break;
        case ASSERT:
            cmd.type.cmd = ASSERT_CMD;
            expect_token(ASSERT, index++, NULL);
            parse_expression(&index, &cmd.field1.node, MIN_PRECEDENCE);
            expect_token(COMMA, index++, NULL);
            expect_token(STRING, index++, &cmd.string);
            break;
        case PRINT:
            cmd.type.cmd = PRINT_CMD;
            expect_token(PRINT, index++, NULL);
            expect_token(STRING, index++, &cmd.string);
            break;
        case SHOW:
            cmd.type.cmd = SHOW_CMD;
            expect_token(SHOW, index++, NULL);
            parse_expression(&index, &cmd.field1.node, MIN_PRECEDENCE);
            break;
        case TIME:
            cmd.type.cmd = TIME_CMD;
            expect_token(TIME, index++, NULL);
            parse_command(&index, &cmd.field1.node);
            break;
        case FN:
            cmd.type.cmd = FN_CMD;
            expect_token(FN, index++, NULL);
            expect_token(VARIABLE, index++, &cmd.string);
            expect_token(LPAREN, index++, NULL);

            cmd.field1.list = vector_create(); if (!cmd.field1.list) return 0;

            while(1) {
                if (peek_token_type(index) == RPAREN) break;
                if (parse_binding(&index, &output))
                    vector_append(cmd.field1.list, (void*) output);
                else if (!try_find_next(&index, COMMA, RPAREN))
                    break;
                if (peek_token_type(index) == RPAREN) break;
                else if (!expect_token(COMMA, index++, NULL)) break;
            }

            expect_token(RPAREN, index++, NULL);
            expect_token(COLON, index++, NULL);
            parse_type(&index, &cmd.field2.node);
            expect_token(LCURLY, index++, NULL);
            if (peek_token_type(index) == RCURLY) break;
            expect_token(NEWLINE, index++, NULL);

            cmd.field3.list = vector_create(); if (!cmd.field3.list) return 0;
            while (1) {
                if (peek_token_type(index) == RCURLY)
                    break;
                
                if (parse_statement(&index, &output))
                    vector_append(cmd.field3.list, (void*) output);

                if (!expect_token(NEWLINE, index++, NULL))
                    break;
            }

            expect_token(RCURLY, index++, NULL);
            break;
        case STRUCT:
            cmd.type.cmd = STRUCT_CMD;
            expect_token(STRUCT, index++, NULL);
            expect_token(VARIABLE, index++, &cmd.string);
            expect_token(LCURLY, index++, NULL);
            if (peek_token_type(index) == RCURLY) break;
            expect_token(NEWLINE, index++, NULL);

            cmd.field1.list = vector_create(); if (!cmd.field1.list) return 0;

            while (1) {
                if (peek_token_type(index) == RCURLY) break;

                if (parse_binding(&index, &output))
                    vector_append(cmd.field1.list, (void*) output);
                else if (!try_find_next(&index, NEWLINE, RCURLY))
                    break;
                
                if (!expect_token(NEWLINE, index++, NULL))
                    break;
            }
            expect_token(RCURLY, index++, NULL);

            break;
        default:
            parse_error(BAD_CMD, index);
            return 0;
    }

    *output_index = nodevec_append(node_vector, cmd);
    *p_index = index;
    return 1;
}

int parse_lvalue(uint32_t *p_index, uint64_t *output_index) {
    uint32_t index = *p_index;
    AstNode lvalue = get_empty_node();

    switch (peek_token_type(index)) {
        case VARIABLE:
            lvalue.type.lvalue = VAR_LVALUE;
            expect_token(VARIABLE, index++, &lvalue.string);

            if (peek_token_type(index) == LSQUARE) {
                lvalue.type.lvalue = ARRAY_LVALUE;
                lvalue.field1.list = vector_create(); if (!lvalue.field1.list) return 0;
                expect_token(LSQUARE, index++, NULL);

                StringRef var;
                while(1) {
                    if (peek_token_type(index) == RSQUARE) break;
                    if (!expect_token(VARIABLE, index, &var)) break;
            
                    vector_append(lvalue.field1.list, (void*) ((uint64_t) index));
                    ++index;
                    
                    if (peek_token_type(index) == RSQUARE) break;
                    else if (!expect_token(COMMA, index++, NULL)) break;
                }

                expect_token(RSQUARE, index++, NULL);
            }
            break;
        default:
            parse_error(BAD_LVALUE, index);
            return 0;
    }

    *output_index = nodevec_append(node_vector, lvalue);
    *p_index = index;
    return 1;
}

int parse_expression(uint32_t *p_index, uint64_t *output_index, int min_precedence) {
    uint32_t index = *p_index;
    uint64_t expr_index;
    int precedence = NO_PRECEDENCE;

    parse_expression_literal(&index, &expr_index);
    
    if (peek_token_type(index) == OP) {
        if (!is_binary_operator(index))
            parse_error(BAD_BINARY, index);
        
        precedence = get_operator_precedence(tokenvec_get(token_vector, index)->strref);
    }

    while (precedence >= min_precedence) {
        AstNode expr = get_empty_node();
        expr.type.expr = BINOP_EXPR;
        // LHS
        expr.field1.node = expr_index;
        expect_token(OP, index++, &expr.string);
        // RHS
        parse_expression(&index, &expr.field2.node, precedence + 1);

        if (peek_token_type(index) == OP && is_binary_operator(index))
            precedence = get_operator_precedence(tokenvec_get(token_vector, index)->strref);
        else
            precedence = NO_PRECEDENCE;

        expr_index = nodevec_append(node_vector, expr);
    }

    *p_index = index;
    *output_index = expr_index;
    return 1;
}

int parse_expression_literal(uint32_t *p_index, uint64_t *output_index) {
    uint32_t index = *p_index;
    AstNode expr = get_empty_node();

    uint64_t output;
    StringRef var;
    switch(peek_token_type(index)) {
        case OP:
            expr.type.expr = UNOP_EXPR;
            expect_token(OP, index++, &expr.string);
            if (expr.string.length != 1 || (*expr.string.string != '-' && *expr.string.string != '!'))
                parse_error(BAD_UNARY, index);
            parse_expression_literal(&index, &expr.field1.node);
            break;
        case INTVAL:
            expr.type.expr = INT_EXPR;
            expect_token(INTVAL, index, &expr.string);
            uint64_t val = strtol(expr.string.string, NULL, 10);
            if (errno == ERANGE)
                parse_error(INT_RANGE, index);
            
            expr.field1.int_value = val;
            ++index;
            break;
        case FLOATVAL:
            expr.type.expr = FLOAT_EXPR;
            expect_token(FLOATVAL, index, &expr.string);
            double fval = strtod(expr.string.string, NULL);
            if (errno == ERANGE)
                parse_error(FLOAT_RANGE, index);
            
            expr.field1.float_value = fval;
            ++index;
            break;
        case TRUE:
            expr.type.expr = TRUE_EXPR;
            expect_token(TRUE, index++, &expr.string);
            break;
        case FALSE:
            expr.type.expr = FALSE_EXPR;
            expect_token(FALSE, index++, &expr.string);
            break;
        case VOID:
            expr.type.expr = VOID_EXPR;
            expect_token(VOID, index++, NULL);
            break;
        case LPAREN:
            expect_token(LPAREN, index++, NULL);
            parse_expression(&index, &expr.field1.node, MIN_PRECEDENCE);
            if (!expect_token(RPAREN, index++, NULL))
                add_parse_error(UNCLOSED_PAREN, tokenvec_get(token_vector, index), tokenvec_get(token_vector, *p_index));
            expr = nodevec_pop_last(node_vector);
            break;
        case VARIABLE:
            expr.type.expr = VAR_EXPR;
            expect_token(VARIABLE, index++, &expr.string);
            
            if (peek_token_type(index) == LCURLY) {
                expr.type.expr = STRUCTLITERAL_EXPR;
                expect_token(LCURLY, index++, NULL);

                expr.field1.list = vector_create(); if (!expr.field1.list) return 0;

                while (1) {
                    if (peek_token_type(index) == RCURLY) break;

                    if (parse_expression(&index, &output, MIN_PRECEDENCE))
                        vector_append(expr.field1.list, (void*) output);
                    else if (!try_find_next(&index, COMMA, RCURLY))
                        break;
                    
                    if (peek_token_type(index) == RCURLY) break;
                    else if (!expect_token(COMMA, index++, NULL)) break;
                }

                expect_token(RCURLY, index++, NULL);
            }
            else if (peek_token_type(index) == LPAREN) {
                expr.type.expr = CALL_EXPR;
                expect_token(LPAREN, index++, NULL);

                expr.field1.list = vector_create(); if (!expr.field1.list) return 0;
                while (1) {
                    if (peek_token_type(index) == RPAREN) break;
                    if (parse_expression(&index, &output, MIN_PRECEDENCE))
                        vector_append(expr.field1.list, (void*) output);
                    else if (!try_find_next(&index, COMMA, RPAREN))
                        break;
                    if (peek_token_type(index) == RPAREN) break;
                    else if (!expect_token(COMMA, index++, NULL)) break;
                }
                expect_token(RPAREN, index++, NULL);
            }
            break;
        case LSQUARE:
            expr.type.expr = ARRAYLITERAL_EXPR;
            expect_token(LSQUARE, index++, &expr.string);

            expr.field1.list = vector_create(); if (!expr.field1.list) return 0;
            while (1) {
                if (peek_token_type(index) == RSQUARE) break;
                if (parse_expression(&index, &output, MIN_PRECEDENCE))
                    vector_append(expr.field1.list, (void*) output);
                else if (!try_find_next(&index, COMMA, RSQUARE))
                    break;
                if (peek_token_type(index) == RSQUARE) break;
                else if (!expect_token(COMMA, index++, NULL)) break;
            }
            expect_token(RSQUARE, index++, NULL);
            break;
        case ARRAY:
            expr.type.expr = ARRAYLOOP_EXPR;
            expect_token(ARRAY, index++, NULL);
            expect_token(LSQUARE, index++, NULL);

            expr.field1.list = vector_create();
            expr.field2.list = vector_create();
            if (!expr.field1.list || !expr.field2.list) return 0;

            while (1) {
                if (peek_token_type(index) == RSQUARE) break;
                expect_token(VARIABLE, index, &var);
                vector_append(expr.field1.list, (void*) (uint64_t) index);
                ++index;
                expect_token(COLON, index++, NULL);
                parse_expression(&index, &output, MIN_PRECEDENCE);
                vector_append(expr.field2.list, (void*) output);
                if (peek_token_type(index) == RSQUARE) break;
                else if (!expect_token(COMMA, index++, NULL)) break;
            }
            expect_token(RSQUARE, index++, NULL);
            parse_expression(&index, &expr.field3.node, MIN_PRECEDENCE);
            break;
        case SUM:
            expr.type.expr = SUMLOOP_EXPR;
            expect_token(SUM, index++, NULL);
            expect_token(LSQUARE, index++, NULL);

            expr.field1.list = vector_create();
            expr.field2.list = vector_create();
            if (!expr.field1.list || !expr.field2.list) return 0;

            while (1) {
                if (peek_token_type(index) == RSQUARE) break;
                expect_token(VARIABLE, index, &var);
                vector_append(expr.field1.list, (void*) (uint64_t) index);
                ++index;
                expect_token(COLON, index++, NULL);
                parse_expression(&index, &output, MIN_PRECEDENCE);
                vector_append(expr.field2.list, (void*) output);
                if (peek_token_type(index) == RSQUARE) break;
                else if (!expect_token(COMMA, index++, NULL)) break;
            }
            expect_token(RSQUARE, index++, NULL);
            parse_expression(&index, &expr.field3.node, MIN_PRECEDENCE);
            break;
        case IF:
            expr.type.expr = IF_EXPR;
            expect_token(IF, index++, NULL);
            parse_expression(&index, &expr.field1.node, MIN_PRECEDENCE);
            expect_token(THEN, index++, NULL);
            parse_expression(&index, &expr.field2.node, MIN_PRECEDENCE);
            expect_token(ELSE, index++, NULL);
            parse_expression(&index, &expr.field3.node, MIN_PRECEDENCE);
            break;
        default:
            parse_error(BAD_EXPR, index);
            return 0;
    }

    AstNode superexpr;
    TokenType type = peek_token_type(index);
    while (type == DOT || type == LSQUARE) {
        superexpr = get_empty_node();
        superexpr.field1.node = nodevec_append(node_vector, expr);

        if (type == DOT) {
            superexpr.type.expr = DOT_EXPR;
            expect_token(DOT, index++, NULL);
            expect_token(VARIABLE, index++, &superexpr.string);
        }
        else if (type == LSQUARE) {
            superexpr.type.expr = ARRAYINDEX_EXPR;
            expect_token(LSQUARE, index++, NULL);
            superexpr.field2.list = vector_create(); if (!superexpr.field2.list) return 0;
            while (1) {
                if (peek_token_type(index) == RSQUARE) break;
                if (parse_expression(&index, &output, MIN_PRECEDENCE))
                    vector_append(superexpr.field2.list, (void*) output);
                else if (!try_find_next(&index, COMMA, RSQUARE))
                    break;
                if (peek_token_type(index) == RSQUARE) break;
                else if (!expect_token(COMMA, index++, NULL)) break;
            }
            expect_token(RSQUARE, index++, NULL);
        }

        expr = superexpr;
        type = peek_token_type(index);
    }

    *output_index = nodevec_append(node_vector, expr);
    *p_index = index;
    return 1;
}

int parse_binding(uint32_t *p_index, uint64_t *output_index) {
    uint32_t index = *p_index;
    AstNode bind = get_empty_node();

    if (!parse_lvalue(&index, &bind.field1.node)) {
        parse_error(BAD_BIND, *p_index);
    }
    expect_token(COLON, index++, NULL);
    parse_type(&index, &bind.field2.node);

    *output_index = nodevec_append(node_vector, bind);
    *p_index = index;
    return 1;
}

int parse_type(uint32_t *p_index, uint64_t *output_index) {
    uint32_t index = *p_index;
    AstNode type = get_empty_node();

    switch(peek_token_type(index)) {
        case INT:
            type.type.type = INT_TYPE;
            expect_token(INT, index++, NULL);
            break;
        case BOOL:
            type.type.type = BOOL_TYPE;
            expect_token(BOOL, index++, NULL);
            break;
        case FLOAT:
            type.type.type = FLOAT_TYPE;
            expect_token(FLOAT, index++, NULL);
            break;
        case VOID:
            type.type.type = VOID_TYPE;
            expect_token(VOID, index++, NULL);
            break;
        case VARIABLE:
            type.type.type = STRUCT_TYPE;
            expect_token(VARIABLE, index++, &type.string);
            break;
        default:
            parse_error(BAD_TYPE, index);
            return 0;
    }

    while (peek_token_type(index) == LSQUARE) {
        AstNode supertype = get_empty_node();
        supertype.type.type = ARRAY_TYPE;
        supertype.field2.node = nodevec_append(node_vector, type);
        supertype.field1.int_value = 1;
        expect_token(LSQUARE, index++, NULL);

        while (1) {
            if (peek_token_type(index) == RSQUARE)
                break;

            if (!expect_token(COMMA, index++, NULL))
                break;
            
            supertype.field1.int_value += 1;
        }

        expect_token(RSQUARE, index++, NULL);

        type = supertype;
    }

    *output_index = nodevec_append(node_vector, type);
    *p_index = index;
    return 1;
}

int parse_statement(uint32_t *p_index, uint64_t *output_index) {
    uint32_t index = *p_index;
    AstNode stmt = get_empty_node();

    switch (peek_token_type(index)) {
        case LET:
            stmt.type.stmt = LET_STMT;
            expect_token(LET, index++, NULL);
            parse_lvalue(&index, &stmt.field1.node);
            expect_token(EQUALS, index++, NULL);
            parse_expression(&index, &stmt.field2.node, MIN_PRECEDENCE);
            break;
        case ASSERT:
            stmt.type.stmt = ASSERT_STMT;
            expect_token(ASSERT, index++, NULL);
            parse_expression(&index, &stmt.field1.node, MIN_PRECEDENCE);
            expect_token(COMMA, index++, NULL);
            expect_token(STRING, index++, &stmt.string);
            break;
        case RETURN:
            stmt.type.stmt = RETURN_STMT;
            expect_token(RETURN, index++, NULL);
            parse_expression(&index, &stmt.field1.node, MIN_PRECEDENCE);
            break;
        default:
            parse_error(BAD_STMT, index);
            return 0;
    }

    *output_index = nodevec_append(node_vector, stmt);
    *p_index = index;
    return 1;
}

int is_binary_operator(uint32_t token_index) {
    Token *token = tokenvec_get(token_vector, token_index);
    if (!token) return 0;
    if (token->type != OP) return 0;

    StringRef string = token->strref;
    switch (*string.string) {
        case '!':
            if (string.length != 2 || *(string.string+1) != '=') return 0;
            return 1;
        default:
            return 1;
    }
}

int get_operator_precedence(StringRef string) {
    switch (*string.string) {
        case '*':
        case '/':
        case '%':
            return 5;
        case '+':
        case '-':
            return 4;
        case '<':
        case '>':
        case '=':
        case '!':
            return 3;
        case '&':
        case '|':
            return 2;
        default:
            return NO_PRECEDENCE;
    }
}
