#include "mysh.h"

/** Run a node and obtain an exit status. */
int invoke_node(node_t *node) {
    LOG("Invoke: %s", inspect_node(node));
    switch (node->type) {
    case N_COMMAND:
        for (int i = 0; node->argv[i] != NULL; i++) {
            LOG("node->argv[%d]: \"%s\"", i, node->argv[i]);
        }

        return 0;

    case N_PIPE: /* foo | bar */
        LOG("node->lhs: %s", inspect_node(node->lhs));
        LOG("node->rhs: %s", inspect_node(node->rhs));

        return 0;

    case N_REDIRECT_IN:     /* foo < bar */
    case N_REDIRECT_OUT:    /* foo > bar */
    case N_REDIRECT_APPEND: /* foo >> bar */
        LOG("node->filename: %s", node->filename);

        return 0;

    case N_SEQUENCE: /* foo ; bar */
        LOG("node->lhs: %s", inspect_node(node->lhs));
        LOG("node->rhs: %s", inspect_node(node->rhs));

        return 0;

    case N_AND: /* foo && bar */
    case N_OR:  /* foo || bar */
        LOG("node->lhs: %s", inspect_node(node->lhs));
        LOG("node->rhs: %s", inspect_node(node->rhs));

        return 0;

    case N_SUBSHELL: /* ( foo... ) */
        LOG("node->lhs: %s", inspect_node(node->lhs));

        return 0;

    default:
        return 0;
    }
}
