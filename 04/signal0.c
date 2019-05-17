#include <signal.h> /* signal */
#include <stdio.h>
#include <unistd.h> /* write */

void sigint_handler(int sig) {
	write(STDERR_FILENO, "korosenaiyo!", 12);
}

void sigalrm_handler(int sig) {
	write(STDERR_FILENO, "*", 1);
	alarm(5);
}

int main() {
	/* 課題2: 端末で ^C (Ctrl-C) をタイプしても殺せない(無視する)ようにせよ．*/
	signal(SIGINT, sigint_handler);

	/* 課題3: 5秒ごとに標準エラー出力に「*」を表示せよ．*/
	signal(SIGALRM, sigalrm_handler);
	alarm(5);

	/* 標準エラー出力にドットを表示し続ける */
	for (;;) {
		for (int j = 0; j < 500000000; j++)
			; /* busy loop */
		write(STDERR_FILENO, ".", 1);
	}
	return 0;
}