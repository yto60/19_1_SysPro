#include "mysh.h"
// #include "string.h"
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>

// int is_eq(char *str1, char *str2){
//     return strcmp(str1, str2) == 0 ? 1 : 0;
// }

void my_exec(char **args) {
	char *command = args[0];
	LOG("%s %s", command, args[1]);
	execvp(command, args);
}

void exec_single(char **args) {
	fflush(stdout);

	pid_t pid = fork();

	if (pid == -1) {
		perror("fork");
		exit(errno);
	}
	if (pid == 0) {
		// child process
		my_exec(args);
	} else {
		// parent process
		wait(NULL);
	}
}

void exec_double(char **argsl, char **argsr) {
	fflush(stdout);

	int fd[2];
	pipe(fd);

	pid_t pidl = fork();
	if (pidl == -1) {
		perror("fork");
		exit(errno);
	}

	if (pidl == 0) {
		dup2(fd[1], STDOUT_FILENO);
		close(fd[0]);
		close(fd[1]);
		fflush(stdout);
		my_exec(argsl);
		// write(fd[1], stdout, 10);
	} else {
		wait(NULL);
		pid_t pidr = fork();
		if (pidr == -1) {
			perror("fork");
			exit(errno);
		}
		if (pidr == 0) {
			dup2(fd[0], STDIN_FILENO);
			close(fd[0]);
			close(fd[1]);
			fflush(stdout);
			my_exec(argsr);
		} else {
			close(fd[0]);
			close(fd[1]);
			wait(NULL);
		}
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
		exec_single(node->argv);

		return 0;

	case N_PIPE: /* foo | bar */
		LOG("node->lhs: %s", inspect_node(node->lhs));
		LOG("node->rhs: %s", inspect_node(node->rhs));
		exec_double(node->lhs->argv, node->rhs->argv);

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
