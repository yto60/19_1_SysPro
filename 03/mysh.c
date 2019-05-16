#include "mysh.h"
// #include "string.h"
#include <errno.h>
#include <fcntl.h> // open
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h> // write, close

// int is_eq(char *str1, char *str2){
//     return strcmp(str1, str2) == 0 ? 1 : 0;
// }

void my_exec(char **argv) {
	LOG("%s %s", argv[0], argv[1]);
	char *command = argv[0];
	fflush(stdout);
	execvp(command, argv);
}

void closefd(int *fd) {
	close(fd[0]);
	close(fd[1]);
}

void child_write(int *fd, char **argv) {
	dup2(fd[1], STDOUT_FILENO);
	closefd(fd);
	// fflush(stdout);
	my_exec(argv);
}

void child_read(int *fd, char **argv) {
	dup2(fd[0], STDIN_FILENO);
	closefd(fd);
	// fflush(stdout);
	my_exec(argv);
}

int open_file_read(char *filename) {
	int filed = open(filename, O_RDONLY);
	if (filed == -1) {
		LOG("failed opening file");
		perror("io1");
		exit(errno);
	}
	return filed;
}

int open_file_write(char *filename, int flags) {
	int filed = open(
			filename, flags | O_CREAT,
			S_IRWXU); // ファイルが存在しなかった場合、所有者に読み取り・書き込み・実行権限を持たせたファイルを作成
	if (filed == -1) {
		LOG("failed opening file");
		perror("io1");
		exit(errno);
	}
	return filed;
}

void close_file(int filed) {
	if (close(filed)) {
		perror("io1");
		exit(errno);
	}
}

void dup2_redirect(node_t *node) {
	int filed;

	switch (node->type) {

	case N_REDIRECT_IN:
		filed = open_file_read(node->filename);
		dup2(filed, STDIN_FILENO);
		break;

	case N_REDIRECT_OUT:
		filed = open_file_write(node->filename, O_WRONLY | O_TRUNC);
		dup2(filed, STDOUT_FILENO);
		break;

	case N_REDIRECT_APPEND:
		filed = open_file_write(node->filename, O_RDWR);
		break;

	default:
		return;
	}

	close_file(filed);
	return;
}

void exec_single(node_t *node) {
	// char **argv = node->argv;
	int type = node->type;

	fflush(stdout);
	pid_t pid = fork();

	if (pid == -1) {
		perror("fork");
		exit(errno);
	}
	if (pid == 0) {
		// child process
		// LOG(node->argv[0]);
		char **argv;
		if (type == N_REDIRECT_IN || type == N_REDIRECT_OUT ||
				type == N_REDIRECT_APPEND) {
			dup2_redirect(node);
			fflush(stdout);
			argv = node->lhs->argv;
			// my_exec(node->argv);
		} else {
			argv = node->argv;
		}
		// LOG(node->argv[0]);
		my_exec(argv);
		// char **argv = node->argv;
		// LOG("%s %s", argv[0], argv[1]);
		// char *command = argv[0];
		// execvp(command, argv);
		LOG("exec done");
	} else {
		// parent process
		int st;
		wait(&st);
		LOG("child process finished: %d", st);
	}
}

void exec_pipe_double(char **argvl, char **argvr) {
	fflush(stdout);

	int fd[2];
	pipe(fd);

	// 子1をつくる
	pid_t pidl = fork();
	if (pidl == -1) {
		perror("fork");
		exit(errno);
	}

	if (pidl == 0) {
		// 子1
		child_write(fd, argvl);
	} else {
		// 親
		wait(NULL); // 子1のプロセスが終わるまで待つ
		// 子2をつくる
		pid_t pidr = fork();
		if (pidr == -1) {
			perror("fork");
			exit(errno);
		}
		if (pidr == 0) {
			// 子2
			child_read(fd, argvr);
		} else {
			// 親
			closefd(fd);
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
		exec_single(node);

		return 0;

	case N_PIPE: /* foo | bar */
		LOG("node->lhs: %s", inspect_node(node->lhs));
		LOG("node->rhs: %s", inspect_node(node->rhs));
		exec_pipe_double(node->lhs->argv, node->rhs->argv);

		return 0;

	case N_REDIRECT_IN: /* foo < bar */
	case N_REDIRECT_OUT:
	case N_REDIRECT_APPEND: /* foo >> bar */
		LOG("node->filename: %s", node->filename);
		exec_single(node);
		return 0;

	case N_SEQUENCE: /* foo ; bar */
		LOG("node->lhs: %s", inspect_node(node->lhs));
		LOG("node->rhs: %s", inspect_node(node->rhs));

		return 0;

	case N_AND: /* foo && bar */
	case N_OR:	/* foo || bar */
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
