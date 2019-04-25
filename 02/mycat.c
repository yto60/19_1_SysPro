#include "mycat.h"
#include "main.h"

#include <fcntl.h>	// open
#include <unistd.h> // write, close

// エラー出力用
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @param size: オプション処理後の引数の個数
 * @param args: オプション処理後の引数群
 */

char *filename;
FILE *fin;
int fd, rd;

int printErrorIo1() {
	perror("io1");
	exit(errno);
}

void fopenFile(int readFromStdin) {
	if (readFromStdin) {
		fin = stdin;
	} else {
		fin = fopen(filename, "r");
		if (fin == NULL) {
			printErrorIo1();
		}
	}
}

void freadFile() {
	while (1) {
		char buf[buffer_size];
		for (int i = 0; i < buffer_size; i++) buf[i] = '\0';

		for (int i = 0; i < buffer_size && !feof(fin); i++) {
			fread(buf + i, sizeof(char), 1, fin); // read 1 letter
		}

		if (fwrite(buf, 1, strlen(buf), stdout) < 1) {
			return;
		}
	}
}

void fcloseFile(int readFromStdin) {
	if (!readFromStdin && fclose(fin) == EOF) {
		printErrorIo1();
	}
}

void openFile(int readFromStdin) {
	if (readFromStdin) {
		fd = STDIN_FILENO;
	} else {
		fd = open(filename, O_RDONLY);
		if (fd == -1) {
			printErrorIo1();
		}
	}
}

void readFile() {
	while (1) {
		char buf[buffer_size];
		for (int i = 0; i < buffer_size; i++) buf[i] = EOF;
		rd = 1;
		// for (int i = 0; i < buffer_size && rd > 0; i++) {
		// 	rd = read(fd, buf + i, 1);
		// 	if (rd == -1) {
		// 		printErrorIo1();
		// 	}
		// 	if (rd == 0) {
		// 		buf[i] = EOF;
		// 		break;
		// 	}
		// }
		rd = read(fd, buf, buffer_size);
		if (rd == 0) {
			return;
		} else if (rd == -1) {
			printErrorIo1();
		}
		// LOG("%s", buf);
		// LOG("%d", strlen(buf));
		// if (strlen(buf) > 1025) {
		// 	LOG("%s", buf);
		// }
		if (buf[0] == EOF) {
			break;
		}
		// for (int i = 0; i < buffer_size && buf[i] != EOF; i++) {
		// 	if (write(STDOUT_FILENO, buf + i, 1) == -1) {
		// 		printErrorIo1();
		// 	}
		// }
		// int len = buffer_size;
		// for (int i = 0; i < buffer_size; i++) {
		// 	if (buf[i] == EOF) len = i - 1;
		// }
		if (write(STDOUT_FILENO, buf, rd) == -1) {
			printErrorIo1();
		}
	}
}

void closeFile(int readFromStdin) {
	if (!readFromStdin && close(fd) == -1) {
		printErrorIo1();
	}
}

void mainCat(int readFromStdin) {
	if (use_stdio) {
		/* -l 有効（課題3）*/
		// implement here
		fopenFile(readFromStdin);
		freadFile();
		fcloseFile(readFromStdin);
	} else {
		/* -l 無効（課題1）*/
		// implement here
		openFile(readFromStdin);
		readFile();
		closeFile(readFromStdin);
	}
}

int do_cat(int size, char **args) {
	int readFromStdin;

	if (args[0] == NULL) {
		LOG("buffer_size = %d", buffer_size);
		readFromStdin = 1;
		mainCat(readFromStdin);
	} else {
		int argLen;
		for (argLen = 0; args[argLen] != NULL; argLen++)
			;
		// LOG("argLen = %d", argLen);
		for (int i = 0; i < argLen; i++) {
			filename = args[i];
			LOG("filename = %s, buffer_size = %d", filename, buffer_size);
			readFromStdin = strcmp(filename, "-") == 0 ? 1 : 0;
			mainCat(readFromStdin);
		}
	}

	return 0;
}
