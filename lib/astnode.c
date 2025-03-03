#include "astnode.h"

AstNode create_node(uint32_t token_index, uint32_t type, uint32_t node1, uint32_t node2, uint32_t node3, uint32_t node4, StringRef string) {
    return (AstNode) {
        token_index,
        {type},
        {node1},
        {node2},
        {node3},
        {node4},
        string
    };
}

AstNode get_empty_node() {
    return (AstNode) {0, {0}, {0}, {0}, {0}, {0}, {0, 0}};
}
