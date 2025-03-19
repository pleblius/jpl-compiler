

#include "typecheck.h"

static TokenVec *token_list;
static NodeVec *node_list;
static Dict *type_dict;

uint64_t float_index;
uint64_t int_index;
uint64_t bool_index;
uint64_t void_index;
uint64_t rgba_index;
uint64_t intarray_index;
uint64_t rgba_matrix_index;

int type_exit_status = EXIT_SUCCESS;

static char *type_names[] = { "INT", "BOOLEAN", "FLOAT", "ARRAY", "STRUCT", "VOID", "VARIABLE" };

void generate_predefs(NodeVec *nodes) {
    if (!nodes) return;
    AstNode node;
    
    node = (AstNode) {0, {.type=FLOAT_TYPE}, {0}, {0}, {0}, {0}, {0, 0}};
    float_index = nodevec_append(nodes, node);
    node = (AstNode) {0, {.type=INT_TYPE}, {0}, {0}, {0}, {0}, {0, 0}};
    int_index = nodevec_append(nodes, node);
    node = (AstNode) {0, {.type=BOOL_TYPE}, {0}, {0}, {0}, {0}, {0, 0}};
    bool_index = nodevec_append(nodes, node);
    node = (AstNode) {0, {.type=VOID_TYPE}, {0}, {0}, {0}, {0}, {0, 0}};
    void_index = nodevec_append(nodes, node);
    node = (AstNode) {0, {.type=ARRAY_TYPE}, {1}, {int_index}, {0}, {0}, {0, 0}};
    intarray_index = nodevec_append(nodes, node);

    // RGBA struct
    Vector *bind_list = vector_create_cap(4);
    node = (AstNode) {0, {0}, {0}, {float_index}, {0}, {0}, {1, "r"}};
    vector_append(bind_list, (void*) nodevec_append(nodes, node));
    node = (AstNode) {0, {0}, {0}, {float_index}, {0}, {0}, {1, "g"}};
    vector_append(bind_list, (void*) nodevec_append(nodes, node));
    node = (AstNode) {0, {0}, {0}, {float_index}, {0}, {0}, {1, "b"}};
    vector_append(bind_list, (void*) nodevec_append(nodes, node));
    node = (AstNode) {0, {0}, {0}, {float_index}, {0}, {0}, {1, "a"}};
    vector_append(bind_list, (void*) nodevec_append(nodes, node));
    
    node = (AstNode) {0, {.type=STRUCT_TYPE}, {0}, {0}, {0}, {0}, {4, "rgba"}};
    rgba_index = nodevec_append(nodes, node);
    AstNode rgba_struct = (AstNode) {0, {.cmd=STRUCT_CMD}, {.list = bind_list}, {rgba_index}, {0}, {0}, {4, "rgba"}};
    dict_add_ref(type_dict, (StringRef) {4, "rgba"}, (void*) nodevec_append(nodes, rgba_struct));

    // RGBA Array Type
    node = (AstNode) {0, {.type=ARRAY_TYPE}, {2}, {rgba_index}, {0}, {0}, {0, 0}};
    rgba_matrix_index = nodevec_append(nodes, node);

    // Args and Argnum
    AstNode argnum = (AstNode) {0, {.cmd=LET_CMD}, {0}, {int_index}, {0}, {0}, {6, "argnum"}};
    dict_add_ref(type_dict, (StringRef) {6, "argnum"}, (void*) nodevec_append(nodes, argnum));
    AstNode args = (AstNode) {0, {.cmd=LET_CMD}, {0}, {intarray_index}, {0}, {0}, {4, "args"}};
    dict_add_ref(type_dict, (StringRef) {4, "args"}, (void*) nodevec_append(nodes, args));

    // Function definitions

    // Float -> Float
    AstNode bind = (AstNode) {0, {0}, {0}, {float_index}, {0}, {0}, {0, NULL}};
    bind_list = vector_create_cap(1); if (!bind_list) exit(EXIT_FAILURE);
    uint64_t bind_index = nodevec_append(nodes, bind);
    vector_append(bind_list, (void*) bind_index);

    AstNode fn = (AstNode) {0, {.cmd=FN_CMD}, {.list=bind_list}, {float_index}, {0}, {0}, {4, "sqrt"}};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {3, "exp"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {3, "sin"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {3, "cos"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {3, "tan"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {4, "asin"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {4, "acos"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {4, "atan"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {3, "log"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));

    // Float -> Int
    fn.string = (StringRef) {6, "to_int"};
    fn.field2.node = int_index;
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));

    // 2-Float -> Float
    bind_list = vector_create_cap(2); if (!bind_list) exit(EXIT_FAILURE);
    fn.field1.list = bind_list;
    fn.field2.node = float_index;
    vector_append(bind_list, (void*) bind_index);
    vector_append(bind_list, (void*) bind_index);
    fn.string = (StringRef) {3, "pow"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
    fn.string = (StringRef) {5, "atan2"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));

    // Int -> Float
    bind_list = vector_create_cap(1); if (!bind_list) exit(EXIT_FAILURE);
    bind.field2.node = int_index;
    vector_append(bind_list, (void*) nodevec_append(nodes, bind));
    fn.field1.list = bind_list;
    fn.field2.node = float_index;
    fn.string = (StringRef) {8, "to_float"};
    dict_add_ref(type_dict, fn.string, (void*) nodevec_append(nodes, fn));
}

int type_check(TokenVec *tokens, NodeVec *nodes, Vector *cmd_nodes) {
    if (!tokens || !nodes || !cmd_nodes) return EXIT_FAILURE;

    token_list = tokens;
    node_list = nodes;
    type_dict = dict_create_big(); if (!type_dict) return EXIT_FAILURE;

    generate_predefs(nodes);

    for (size_t i = 0; i < cmd_nodes->size; ++i) {
        if (!type_check_cmd((uint64_t) vector_get(cmd_nodes, i)))
            type_exit_status = EXIT_FAILURE;
    }

    return type_exit_status;
}

int type_check_cmd(uint32_t cmd_index) {
    AstNode *cmd = nodevec_get(node_list, cmd_index);
    if (!cmd) return 0;

    switch (cmd->type.cmd) {
        case READ_CMD:
            return type_check_read_cmd(cmd_index);
        case WRITE_CMD:
            return type_check_write_cmd(cmd_index);
        case LET_CMD:
            return type_check_let_cmd(cmd_index);
        case ASSERT_CMD:
            if (!type_check_expr(cmd->field1.node)) return 0;
            if (!expect_expr_type(BOOL_TYPE, cmd->field1.node)) return 0;
            break;
        case PRINT_CMD:
            return 1;
        case SHOW_CMD:
            return type_check_expr(cmd->field1.node);
        case TIME_CMD:
            return type_check_cmd(cmd->field1.node);
        case FN_CMD:
            return type_check_fn_cmd(cmd_index);
        case STRUCT_CMD:
            return type_check_struct_cmd(cmd_index);
    }

    return 1;
}

int type_check_read_cmd(uint32_t cmd_index) {
    AstNode *cmd = nodevec_get(node_list, cmd_index);
    if (!cmd) return 0;

    uint64_t lvalue_index = cmd->field1.node;

    if (!type_check_lvalue(lvalue_index))
        return 0;

    AstNode *lvalue = nodevec_get(node_list, lvalue_index);

    if (lvalue->field1.list != NULL && lvalue->field1.list->size != 2) {
        type_error(BAD_RANK, lvalue->token_index, 0);
        return 0;
    }

    lvalue->field2.node = rgba_matrix_index;
    nodevec_get(node_list, cmd_index)->field2.node = rgba_matrix_index;
    return 1;
}

int type_check_write_cmd(uint32_t cmd_index) {
    AstNode *cmd = nodevec_get(node_list, cmd_index);
    if (!cmd) return 0;

    uint64_t expr_index = cmd->field1.node;

    if (!type_check_expr(expr_index)) return 0;
    AstNode *expr = nodevec_get(node_list, expr_index);

    if (!compare_types(rgba_matrix_index, expr->field4.node)) {
        type_error(UNEXPECTED_TYPE, expr->field4.node, rgba_matrix_index);
        return 0;
    }

    return 1;
}

int type_check_let_cmd(uint32_t cmd_index) {
    AstNode *cmd = nodevec_get(node_list, cmd_index);
    if (!cmd) return 0;

    uint64_t lvalue_index = cmd->field1.node;
    uint64_t expr_index = cmd->field3.node;

    if (!type_check_expr(expr_index) || !type_check_lvalue(lvalue_index))
        return 0;
    
    AstNode *expr = nodevec_get(node_list, expr_index);
    AstNode *lvalue = nodevec_get(node_list, lvalue_index);

    if (lvalue->type.lvalue == ARRAY_LVALUE) {
        if (lvalue->field1.list->size == 0) {
            type_error(EMPTY_ARRAY, lvalue->token_index, 0);
            return 0;
        }
        AstNode *expr_type = nodevec_get(node_list, expr->field4.node);
        if (!expr_type) return 0;
        if (lvalue->field1.list->size != expr_type->field1.int_value) {
            type_error(BAD_DIMENSION, cmd->field1.node, cmd->field3.node);
            return 0;
        }
    }

    lvalue->field2.node = expr->field4.node;
    return 1;
}

int type_check_fn_cmd(uint32_t cmd_index) {
    AstNode *cmd = nodevec_get(node_list, cmd_index);
    if (!cmd) return 0;
    Vector *bind_list = cmd->field1.list; if (!bind_list) return 0;
    uint32_t type_index = cmd->field2.node;
    Vector *stmt_list = cmd->field3.list; if (!stmt_list) return 0;

    uint64_t output;
    if (!dict_add_ref_if_empty(type_dict, cmd->string, (void*) (uint64_t) cmd_index, (void**) &output)) {
        type_error(SHADOWED_VARIABLE, cmd_index, output);
        return 0;
    }

    // Add binds to scope
    uint32_t bind_index;
    uint32_t lvalue_index, bind_type_index;
    AstNode *bind, *lvalue;
    for (size_t i = 0; i < bind_list->size; ++i) {
        bind_index = (uint64_t) vector_get(bind_list, i);
        bind = nodevec_get(node_list, bind_index);
        if (!bind) return 0;
        lvalue_index = bind->field1.node;
        bind_type_index = bind->field2.node;

        if (!type_check_lvalue(lvalue_index)) return 0;
        lvalue = nodevec_get(node_list, lvalue_index);
        lvalue->field2.node = bind_type_index;
    }

    AstNode *fn_type = nodevec_get(node_list, type_index);
    if (!fn_type) return 0;

    int valid_return = compare_types(void_index, type_index);
    AstNode *stmt;
    uint32_t stmt_index;
    for (size_t i = 0; i < stmt_list->size; ++i) {
        stmt_index = (uint64_t) vector_get(stmt_list, i);
        if (!type_check_statement(stmt_index)) return 0;
        stmt = nodevec_get(node_list, stmt_index);

        if (stmt->type.stmt == RETURN_STMT) {
            if (!compare_types(type_index, stmt->field2.node)) {
                type_error(BAD_RETURN, stmt_index, cmd->field2.node);
                return 0;
            }

            valid_return = 1;
        }
    }

    if (!valid_return) {
        type_error(BAD_RETURN, cmd_index, type_index);
        return 0;
    }

    // Free local variables
    for (size_t i = 0; i < bind_list->size; ++i) {
        bind_index = (uint64_t) vector_get(bind_list, i);
        bind = nodevec_get(node_list, bind_index);
        remove_lvalue(bind->field1.node);
    }
    for (size_t i = 0; i < stmt_list->size; ++i) {
        stmt_index = (uint64_t) vector_get(stmt_list, i);
        stmt = nodevec_get(node_list, stmt_index);
        if (stmt->type.stmt == LET_STMT) {
            remove_lvalue(stmt->field1.node);
        }
    }

    return 1;
}

int type_check_struct_cmd(uint32_t cmd_index) {
    AstNode *cmd = nodevec_get(node_list, cmd_index);
    if (!cmd) return 0;

    Vector *member_list = cmd->field1.list; if (!member_list) return 0;

    Dict *struct_dict = dict_create_small();
    if (!struct_dict) exit(EXIT_FAILURE);

    uint32_t member_index;
    AstNode *member;
    AstNode *type;
    uint64_t output;
    for (size_t i = 0; i < member_list->size; ++i) {
        member_index = (uint64_t) vector_get(member_list, i);
        member = nodevec_get(node_list, member_index);
        if (!member) goto free;

        type = nodevec_get(node_list, member->field2.node);
        if (!type) goto free;

        while (type->type.type == ARRAY_TYPE) {
            type = nodevec_get(node_list, type->field2.node);
            if (!type) goto free;
        }

        if (type->type.type == STRUCT_TYPE) {
            if (!dict_try_ref(type_dict, type->string, (void**) &output)) {
                type_error(UNDECLARED_VARIABLE, member->field2.node, 0);
                goto free;
            }

            AstNode *sub_cmd = nodevec_get(node_list, output); if (!cmd) return 0;
            *type = *nodevec_get(node_list, sub_cmd->field2.node);
        }

        if (!dict_add_ref_if_empty(struct_dict, member->string, (void*) (uint64_t) member_index, (void**) &output)) {
            type_error(SHADOWED_VARIABLE, member_index, output);
            goto free;
        }
    }

    if (!dict_add_ref_if_empty(type_dict, cmd->string, (void*) (uint64_t) cmd_index, (void**) &output)) {
        type_error(SHADOWED_VARIABLE, cmd_index, output);
        goto free;
    }

    dict_free(struct_dict);
    return 1;

free:
    dict_free(struct_dict);
    return 0;
}

int type_check_statement(uint32_t stmt_index) {
    AstNode *stmt = nodevec_get(node_list, stmt_index);
    if (!stmt) return 0;

    switch (stmt->type.stmt) {
        case LET_STMT:
            return type_check_let_cmd(stmt_index);
        case ASSERT_STMT:
            if (!type_check_expr(stmt->field1.node)) return 0;
            if (!expect_expr_type(BOOL_TYPE, stmt->field1.node)) return 0;
            break;
        case RETURN_STMT:
            if (!type_check_expr(stmt->field1.node)) return 0;
            stmt->field2.node = nodevec_get(node_list, stmt->field1.node)->field4.node;
            break;
    }

    return 1;
}

int type_check_lvalue(uint32_t lvalue_index) {
    AstNode *lvalue = nodevec_get(node_list, lvalue_index);
    if (!lvalue) return 0;

    uint64_t output;
    if (!dict_add_ref_if_empty(type_dict, lvalue->string, (void*) (uint64_t) lvalue_index, (void**) &output)) {
        type_error(SHADOWED_VARIABLE, lvalue_index, output);
        return 0;
    }

    if (lvalue->type.lvalue == ARRAY_LVALUE) {
        Vector *list = lvalue->field1.list; 
        if (!list) return 0;

        AstNode *member;
        uint32_t member_index;
        for (size_t i = 0; i < list->size; ++i) {
            member_index = (uint64_t) vector_get(list, i);
            member = nodevec_get(node_list, member_index);
            if (!member) return 0;

            if (!dict_add_ref_if_empty(type_dict, member->string, (void*) (uint64_t) member_index, (void**) &output)) {
                type_error(SHADOWED_VARIABLE, member_index, output);
                return 0;
            }

            member->field2.node = int_index;
        }
    }

    return 1;
}

int type_check_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    switch (expr->type.expr) {
        case INT_EXPR:
        case FLOAT_EXPR:
        case TRUE_EXPR:
        case FALSE_EXPR:
        case VOID_EXPR:
            return 1;
        case VAR_EXPR:
            return type_check_var_expr(expr_index);
        case ARRAYLITERAL_EXPR:
            return type_check_arraylit_expr(expr_index);
        case STRUCTLITERAL_EXPR:
            return type_check_structlit_expr(expr_index);
        case DOT_EXPR:
            return type_check_dot_expr(expr_index);
        case ARRAYINDEX_EXPR:
            return type_check_arrayindex_expr(expr_index);
        case CALL_EXPR:
            return type_check_call_expr(expr_index);
        case UNOP_EXPR:
            return type_check_unop_expr(expr_index);
        case BINOP_EXPR:
            return type_check_binop_expr(expr_index);
        case IF_EXPR:
            return type_check_if_expr(expr_index);
        case ARRAYLOOP_EXPR:
        case SUMLOOP_EXPR:
            return type_check_loop_expr(expr_index);
    }
    return 1;
}

int type_check_var_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;
    uint64_t output;
    if (!dict_try_ref(type_dict, expr->string, (void**) &output)) {
        type_error(UNDECLARED_VARIABLE, expr_index, 0);
        return 0;
    }
    AstNode *cmd = nodevec_get(node_list, output); if (!cmd) return 0;
    AstNode *type = nodevec_get(node_list, cmd->field2.node); if (!type) return 0;
    switch (type->type.type) {
        case VAR_TYPE:
            type_error(UNRESOLVED_TYPE, expr_index, output);
            return 0;
        default:
            expr->field4.node = cmd->field2.node;
    }
    return 1;
}

int type_check_arraylit_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    uint32_t type_index = expr->field4.node;

    Vector *expr_list = expr->field1.list;
    if (!expr_list) return 0;
    if (!expr_list->size) {
        type_error(EMPTY_ARRAY, expr_index, 0);
        return 0;
    }

    uint32_t zero_index = (uint64_t) vector_get(expr_list, 0);
    if (!type_check_expr(zero_index)) return 0;

    AstNode *array_type = nodevec_get(node_list, type_index);
    array_type->field2.node = nodevec_get(node_list, zero_index)->field4.node;

    uint32_t sub_index;
    for (size_t i = 1; i < expr_list->size; ++i) {
        sub_index = (uint64_t) vector_get(expr_list, i);
        if (!type_check_expr(sub_index)) return 0;

        if (!compare_expr_types(zero_index, sub_index)) {
            type_error(MISMATCHED_ARRAY, sub_index, zero_index);
            return 0;
        }
    }

    return 1;
}

int type_check_structlit_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    uint64_t output;
    if (!dict_try_ref(type_dict, expr->string, (void**) &output)) {
        type_error(UNDECLARED_VARIABLE, expr_index, 0);
        return 0;
    }
    AstNode *cmd = nodevec_get(node_list, output); if (!cmd) return 0;
    if (!expect_type(STRUCT_TYPE, cmd->field2.node)) return 0;

    Vector *list1 = cmd->field1.list;
    Vector *list2 = expr->field1.list;
    if (!list1 || !list2) return 0;
    if (list1->size != list2->size) {
        type_error(MISMATCHED_MEMBERS, expr_index, output);
        return 0;
    }

    AstNode *sub_expr;
    uint32_t sub_expr_index;
    for (size_t i = 0; i < list1->size; ++i) {
        sub_expr_index = (uint64_t) vector_get(list2, i);
        sub_expr = nodevec_get(node_list, sub_expr_index);
        if (!type_check_expr(sub_expr_index)) return 0;

        AstNode *member = nodevec_get(node_list, (uint64_t) vector_get(list1, i));
        if (!member) return 0;
        if (!compare_types(member->field2.node, sub_expr->field4.node)) {
            type_error(MISMATCHED_MEMBERS, member->field2.node, sub_expr->field4.node);
            return 0;
        }
    }

    nodevec_get(node_list, expr_index)->field4.node = nodevec_get(node_list, output)->field2.node;
    return 1;
}

int type_check_dot_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    StringRef expr_string = expr->string;
    uint32_t sub_expr_index = expr->field1.node;
    if (!type_check_expr(sub_expr_index)) return 0;

    AstNode *sub_expr = nodevec_get(node_list, sub_expr_index);
    if (!sub_expr) return 0;

    if (!expect_type(STRUCT_TYPE, sub_expr->field4.node)) {
        type_error(BAD_DERFERENCE, expr_index, sub_expr->field4.node);
        return 0;
    }

    AstNode *subtype = nodevec_get(node_list, sub_expr->field4.node);
    if (!subtype) return 0;

    uint64_t output;
    if (!dict_try_ref(type_dict, subtype->string, (void**) &output)) {
        type_error(UNDECLARED_VARIABLE, subtype->token_index, 0);
        return 0;
    }

    AstNode *cmd = nodevec_get(node_list, output);
    if (!cmd) return 0;
    Vector *member_list = cmd->field1.list;
    if (!member_list) return 0;

    AstNode *member;
    for (size_t i = 0; i < member_list->size; ++i) {
        member = nodevec_get(node_list, (uint64_t) vector_get(member_list, i));
        if (!member) return 0;

        if (member->string.length != expr_string.length) continue;
        if (strncmp(member->string.string, expr_string.string, expr_string.length)) continue;

        expr->field4.node = member->field2.node;
        return 1;
    }
    
    type_error(BAD_MEMBER, expr_index, cmd->token_index);
    return 0;
}

int type_check_arrayindex_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    uint32_t expr1_index = expr->field1.node;
    Vector *expr_list = expr->field2.list;
    if (!expr_list) return 0;
    if (!expr_list->size) {
        type_error(NO_INDEX, expr_index, 0);
        return 0;
    }

    if (!type_check_expr(expr1_index)) return 0;
    if (!expect_expr_type(ARRAY_TYPE, expr1_index))
        return 0;

    AstNode *sub_expr = nodevec_get(node_list, expr1_index);
    uint32_t sub_expr_type_index = sub_expr->field4.node;
    AstNode *sub_type = nodevec_get(node_list, sub_expr_type_index);

    if (expr_list->size != sub_type->field1.int_value) {
        type_error(BAD_DIMENSION, expr_index, expr->field1.node);
        return 0;
    }

    uint32_t index;
    for (size_t i = 0; i < expr_list->size; ++i) {
        index = (uint64_t) vector_get(expr_list, i);
        if (!type_check_expr(index))
            return 0;
        if (!expect_expr_type(INT_TYPE, index)) {
            type_error(BAD_INDEX, index, 0);
            return 0;
        }
    }

    nodevec_get(node_list, expr_index)->field4.node = nodevec_get(node_list, sub_expr_type_index)->field2.node;

    return 1;
}

int type_check_call_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    uint64_t output;
    if (!dict_try_ref(type_dict, expr->string, (void**) &output)) {
        type_error(UNDECLARED_VARIABLE, expr_index, 0);
        return 0;
    }

    AstNode *cmd = nodevec_get(node_list, output);
    if (!cmd) return 0;
    if (cmd->type.cmd != FN_CMD) {
        type_error(BAD_FN, expr_index, output);
        return 0;
    }

    Vector *bind_list = cmd->field1.list; if (!bind_list) return 0;
    Vector *expr_list = expr->field1.list; if (!expr_list) return 0;
    if (bind_list->size != expr_list->size) {
        type_error(MISMATCHED_MEMBERS, expr_index, output);
        return 0;
    }

    uint32_t sub_index;
    AstNode *bind, *type;
    uint32_t bind_index;
    for (size_t i = 0; i < expr_list->size; ++i) {
        sub_index = (uint64_t) vector_get(expr_list, i);
        if (!type_check_expr(sub_index)) return 0;

        bind_index = (uint64_t) vector_get(bind_list, i);
        bind = nodevec_get(node_list, bind_index);
        if (!bind) return 0;
        type = nodevec_get(node_list, bind->field2.node);
        if (!type) return 0;

        if (!expect_expr_type(type->type.type, sub_index)) {
            type_error(BAD_MEMBER, sub_index, bind->field2.node);
            return 0;
        }
    }

    nodevec_get(node_list, expr_index)->field4.node = nodevec_get(node_list, output)->field2.node;
    return 1;
}

int type_check_unop_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    StringRef expr_string = expr->string;
    uint32_t sub_expr_index = expr->field1.node;

    if (!type_check_expr(expr->field1.node))
        return 0;

    AstNode *sub_expr = nodevec_get(node_list, sub_expr_index);

    AstNode *type = nodevec_get(node_list, sub_expr->field4.node);
    if (!type) return 0;
    
    switch (expr_string.string[0]) {
        case '-':
            if (type->type.type != INT_TYPE && type->type.type != FLOAT_TYPE) {
                type_error(BAD_UNOP, expr->field1.node, expr_index);
                return 0;
            }
            break;
        case '!':
            if (type->type.type != BOOL_TYPE) {
                type_error(BAD_UNOP, expr->field1.node, expr_index);
                return 0;
            }
            break;
        default:
            return 0;
    }

    nodevec_get(node_list, expr_index)->field4.node = sub_expr->field4.node;
    return 1;
}

int type_check_binop_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    uint32_t expr1_index = expr->field1.node;
    uint32_t expr2_index = expr->field2.node;
    uint32_t type_index = expr->field4.node;

    if (!type_check_expr(expr1_index) || !type_check_expr(expr2_index)) return 0;

    if (!compare_expr_types(expr1_index, expr2_index)) {
        type_error(MISMATCHED_BINOP, expr2_index, expr1_index);
        return 0;
    }

    AstNode *expr1 = nodevec_get(node_list, expr1_index);
    if (!expr1) return 0;

    AstNode *type1 = nodevec_get(node_list, expr1->field4.node);
    switch (expr->string.string[0]) {
        case '=':
        case '!':
            if (type1->type.type != INT_TYPE && type1->type.type != FLOAT_TYPE && type1->type.type != BOOL_TYPE) {
                type_error(BAD_BINOP, expr->field1.node, expr_index);
                return 0;
            }
            break;
        case '|':
        case '&':
            if (type1->type.type != BOOL_TYPE) {
                type_error(BAD_BINOP, expr->field1.node, expr_index);
                return 0;
            }
            break;
        default:
            if (type1->type.type != INT_TYPE && type1->type.type != FLOAT_TYPE) {
                type_error(BAD_BINOP, expr->field1.node, expr_index);
                return 0;
            }
    }
    AstNode *type = nodevec_get(node_list, type_index); if (!type) return 0;
    if (type->type.type != BOOL_TYPE)
        nodevec_get(node_list, expr_index)->field4.node = expr1->field4.node;
    return 1;
}

int type_check_if_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    uint32_t expr1_index = expr->field1.node;
    uint32_t expr2_index = expr->field2.node;
    uint32_t expr3_index = expr->field3.node;

    if (!type_check_expr(expr1_index)) return 0;
    if (!expect_expr_type(BOOL_TYPE, expr1_index)) return 0;

    if (!type_check_expr(expr2_index) || !type_check_expr(expr3_index)) return 0;
    if (!compare_expr_types(expr2_index, expr3_index)) {
        type_error(MISMATCHED_IF, expr3_index, expr2_index);
        return 0;
    }

    nodevec_get(node_list, expr_index)->field4.node = nodevec_get(node_list, expr2_index)->field4.node;
    return 1;
}

int type_check_loop_expr(uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    Vector *var_list = expr->field1.list;
    Vector *expr_list = expr->field2.list;
    uint32_t sub_expr_index = expr->field3.node;
    uint32_t type_index = expr->field4.node;
    ExpressionType exprtype = expr->type.expr;

    if (var_list->size == 0 || expr_list->size == 0) {
        type_error(EMPTY_ARRAY, expr_index, 0);
        return 0;
    }

    Token *token;
    uint32_t sub_index;
    uint32_t token_index;
    AstNode temp_cmd;
    StringRef string;
    uint64_t output;
    uint64_t member_index;
    for (size_t i = 0; i < expr_list->size; ++i) {
        sub_index = (uint64_t) vector_get(expr_list, i);
        if (!type_check_expr(sub_index)) return 0;

        if (!expect_expr_type(INT_TYPE, sub_index)) {
            type_error(BAD_INDEX, sub_index, 0);
            return 0;
        }
    }
    for (size_t i = 0; i < var_list->size; ++i) {
        token_index = (uint64_t) vector_get(var_list, i);
        if (!token_index) return 0;
        token = tokenvec_get(token_list, token_index);
        if (!token) return 0;
        string = token->strref;

        temp_cmd = (AstNode) {token_index, {.cmd=LET_CMD}, {0}, {int_index}, {0}, {0}, string};

        member_index = nodevec_append(node_list, temp_cmd);
        if (!dict_add_ref_if_empty(type_dict, string, (void*) member_index, (void**) &output)) {
            type_error(SHADOWED_VARIABLE, member_index, output);
            return 0;
        }
    }

    if (!type_check_expr(sub_expr_index)) return 0;
    if (nodevec_get(node_list, type_index)->type.type == ARRAY_TYPE)
        nodevec_get(node_list, type_index)->field2.node = nodevec_get(node_list, sub_expr_index)->field4.node;
    else
        nodevec_get(node_list, expr_index)->field4.node = nodevec_get(node_list, sub_expr_index)->field4.node;

    type_index = nodevec_get(node_list, expr_index)->field4.node;

    if (exprtype == SUMLOOP_EXPR) {
        if (!compare_types(type_index, int_index) && !compare_types(type_index, float_index)) {
            type_error(BAD_SUM, sub_expr_index, 0);
            return 0;
        }
    }

    for (size_t i = 0; i < var_list->size; ++i) {
        token_index = (uint64_t) vector_get(var_list, i);
        string = tokenvec_get(token_list, token_index)->strref;
        dict_remove_ref(type_dict, string);
    }

    return 1;
}

int compare_expr_types(uint32_t index1, uint32_t index2) {
    AstNode *expr1 = nodevec_get(node_list, index1);
    AstNode *expr2 = nodevec_get(node_list, index2);
    if (!expr1 || !expr2) return 0;

    return compare_types(expr1->field4.node, expr2->field4.node);
}

int compare_types(uint32_t index1, uint32_t index2) {
    AstNode *type1 = nodevec_get(node_list, index1);
    AstNode *type2 = nodevec_get(node_list, index2);
    if (!type1 || !type2) return 0;

    if (type1->type.type != type2->type.type) return 0;

    if (type1->type.type == ARRAY_TYPE)
        return compare_types(type1->field2.node, type2->field2.node);

    return 1;
}

int expect_expr_type(TypeType expected_type, uint32_t expr_index) {
    AstNode *expr = nodevec_get(node_list, expr_index);
    if (!expr) return 0;

    return expect_type(expected_type, expr->field4.node);
}

int expect_type(TypeType expected_type, uint32_t type_index) {
    AstNode *type = nodevec_get(node_list, type_index);
    if (!type) return 0;
    
    if (type->type.type != expected_type) {
        AstNode *new_token = malloc(sizeof(AstNode)); if (!new_token) exit(EXIT_FAILURE);
        *new_token = (AstNode) {0, {.type=expected_type}, {0}, {0}, {0}, {0}, {strlen(type_names[expected_type]), type_names[expected_type]} };

        add_type_error(UNEXPECTED_TYPE, type, new_token);
        
        free(new_token);
        return 0;
    }

    return 1;
}

void type_error(TypeErrorType type, uint32_t error_index, uint32_t ref_index) {
    if (error_index < node_list->size && ref_index < node_list->size)
        add_type_error(type, nodevec_get(node_list, error_index), nodevec_get(node_list, ref_index));
        
    type_exit_status = EXIT_FAILURE;
}

void remove_lvalue(uint32_t lvalue_index) {
    AstNode *lvalue = nodevec_get(node_list, lvalue_index);
    if (!lvalue) return;

    dict_remove_ref(type_dict, lvalue->string);
    if (lvalue->type.lvalue == ARRAY_LVALUE) {
        Vector *list = lvalue->field1.list;
        AstNode *member;
        for (size_t i = 0; i < list->size; ++i) {
            member = nodevec_get(node_list, (uint64_t) vector_get(list, i));
            dict_remove_ref(type_dict, member->string);
        }
    }
}
