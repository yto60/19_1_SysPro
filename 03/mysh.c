#include "mysh.h"
#include <errno.h>
#include <fcntl.h> // open
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h> // write, close

void closefd(int *fd) {
	close(fd[0]);
	close(fd[1]);
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

int open_file_write(char *filename, int additional_flag) {
	int filed = open(filename, O_WRONLY | O_CREAT | additional_flag, 0666);
	// ファイルが存在しなかった場合、全員に読み取り・書き込み・実行権限を持たせたファイルを作成
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
		filed = open_file_write(node->filename, O_TRUNC);
		dup2(filed, STDOUT_FILENO);
		break;

	case N_REDIRECT_APPEND:
		filed = open_file_write(node->filename, O_APPEND);
		dup2(filed, STDOUT_FILENO);
		break;

	default:
		return;
	}

	close_file(filed);
	return;
}

void my_exec(node_t *node) {
	char **argv;
	int type = node->type;

	if (type == N_REDIRECT_IN || type == N_REDIRECT_OUT ||
			type == N_REDIRECT_APPEND) {
		// リダイレクトありの場合
		dup2_redirect(node);
		argv = node->lhs->argv;
	} else {
		// なしの場合
		argv = node->argv;
	}

	LOG("%s %s", argv[0], argv[1]);
	char *command = argv[0];
	// fflush(stdout);
	execvp(command, argv);
}

void exec_single(node_t *node) {
	fflush(stdout);
	pid_t pid = fork();

	if (pid == -1) {
		perror("fork");
		exit(errno);
	}
	if (pid == 0) {
		// child process
		my_exec(node);
		LOG("exec done");
	} else {
		// parent process
		int st;
		wait(&st);
		LOG("child process finished: %d", st);
	}
}

void exec_pipe(node_t *node, int is_root) {
	int st;
	fflush(stdout);

	int fd[2];
	pipe(fd);

	// 子1をつくる
	pid_t pidl = fork();
	LOG("pidl: %d\n", pidl);
	if (pidl == -1) {
		perror("fork");
		exit(errno);
	}

	if (pidl == 0) {
		// 子1
		dup2(fd[1], STDOUT_FILENO);
		closefd(fd);
		my_exec(node->lhs);
	} else {
		// 親
		// 子2をつくる
		pid_t pidr = fork();
		LOG("pidr: %d\n", pidr);
		if (pidr == -1) {
			perror("fork");
			exit(errno);
		}
		if (pidr == 0) {
			// 子2
			dup2(fd[0], STDIN_FILENO);
			closefd(fd);
			if (node->rhs->type == N_PIPE) {
				exec_pipe(node->rhs, 0);
			} else {
				my_exec(node->rhs);
			}
		} else {
			// 親
			closefd(fd);
			wait(&st); // 子2が終わるまで待つ
			LOG("%d: wait for pidr done (%d)", pidr, st);
		}
		wait(&st); // 子1のプロセスが終わるまで待つ
		LOG("wait for pidl done (%d)", st);
		if (!is_root) {
			exit(0);
		}
	}
}

void exec_sequence(node_t *node) {
	exec_single(node->lhs);
	if (node->rhs->type == N_SEQUENCE) {
		exec_sequence(node->rhs);
	} else {
		exec_single(node->rhs);
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
		exec_pipe(node, 1);

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
		exec_sequence(node);

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
