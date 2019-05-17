#include "mysh.h"
#include <errno.h>	/* errno */
#include <stdbool.h>  /* bool */
#include <stdlib.h>   /* exit */
#include <string.h>   /* strcmp */
#include <sys/wait.h> /* wait */
#include <unistd.h>   /* fork */

/* うまく表示されない場合は，下記の 1 を 0 に切り替えて下さい */
#if 1
#define FIRE "\xF0\x9F\x94\xA5\n"
#else
#define FIRE "FIRE\n"
#endif

/**
 * 引数の2文字列が等しければ真．NULLの場合は両方ともNULLであれば真．
 */
bool streql(const char *lhs, const char *rhs) {
	return (lhs == NULL && rhs == NULL) ||
		   (lhs != NULL && rhs != NULL && strcmp(lhs, rhs) == 0);
}

void wait_children(int pid, int options) {
	int status;

	pid_t waited_pid = waitpid(pid, &status, options);
	// 成功すると子プロセスのプロセスID，状態変化がなければ0，エラーの場合は-1が返る

	if (waited_pid == -1) {
		// エラー処理
		if (errno == ECHILD) {
			/* すでに成仏していた（何もしない） */
		} else {
			PERROR_DIE(NULL);
		}
	} else {
		// 子プロセスの状態が変化している
		LOG("Waited: pid=%d, status=%d, exit_status=%d", waited_pid, status,
			WEXITSTATUS(status));
		if (WIFEXITED(status)) {
			write(STDERR_FILENO, FIRE, 5);
		}
	}
}

void sigchld_handler(int sig) {
	wait_children(-1, WNOHANG);
}

int invoke_node(node_t *node) {
	LOG("Invoke: %s", inspect_node(node));
	pid_t pid;

	/* & 付きで起動しているか否か */
	if (node->async) {
		LOG("{&} found: async execution required");
	}

	/* 子プロセスの生成 */
	fflush(stdout);
	pid = fork();
	if (pid == -1) {
		PERROR_DIE(node->argv[0]);
	}

	if (pid == 0) {
		/* 子プロセス側 */
		if (execvp(node->argv[0], node->argv) == -1) {
			PERROR_DIE(node->argv[0]);
		}
		return 0; /* never happen */
	} else {
		/* 親プロセス側 */

		/* 子に独立したプロセスグループを割り振る */
		if (setpgid(pid, pid) == -1) {
			PERROR_DIE(NULL);
		}

		/* 生成した子プロセスを待機 */
		if (node->async) {
			// シグナルハンドラを設定
			signal(SIGCHLD, sigchld_handler);
		} else {
			// その場で待つ
			wait_children(pid, 0);
		}

		return pid;
	}
}

/* A hook point to initialize shell */
void init_shell(void) {
	LOG("Initializing mysh2...");
}
