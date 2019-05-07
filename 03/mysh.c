#include "mysh.h"
// #include "string.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

// int is_eq(char *str1, char *str2){
//     return strcmp(str1, str2) == 0 ? 1 : 0;
// }

void exec_command(char **args) {
	fflush(stdout);

	pid_t pid = fork();

	if (pid == -1) {
		perror("fork");
		exit(errno);
	}
	if (pid == 0) {
		// child process
		char *command = args[0];
		execvp(command, args);
	} else {
		// parent process
		wait(NULL);
	}
}

/** Run a node and obtain an exit status. */
int invoke_node(node_t *node) {
	LOG("Invoke: %s", inspect_node(node));
	switch (node->type) {
	case N_COMMAND:
		for (int i = 0; node->argv[i] != NULL; i++) {
			LOG("node->argv[%d]: \"%s\"", i, node->argv[i]);
		}
		exec_command(node->argv);

		return 0;

	case N_PIPE: /* foo | bar */
		LOG("node->lhs: %s", inspect_node(node->lhs));
		LOG("node->rhs: %s", inspect_node(node->rhs));

		return 0;

	case N_REDIRECT_IN:		/* foo < bar */
	case N_REDIRECT_OUT:	/* foo > bar */
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
