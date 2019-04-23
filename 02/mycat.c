#include "mycat.h"
#include "main.h"

#include <fcntl.h>	// open
#include <unistd.h> // write, close

// エラー出力用
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int printErrorIo1() {
	perror("io1");
	exit(errno);
}

/**
 * @param size: オプション処理後の引数の個数
 * @param args: オプション処理後の引数群
 */

int do_cat(int size, char **args) {
	char *filename = args[0];
	LOG("filename = %s, buffer_size = %d", filename, buffer_size);
	if (use_stdio) {
		/* -l 有効（課題3）*/
		// implement here
	} else {
		/* -l 無効（課題1）*/
		// implement here

		int fd, rd;

		// file open
		fd = open(filename, O_RDONLY);
		if (fd == -1) {
			printErrorIo1();
		}

		// read and write
		while (1) {
			char buf[buffer_size];
			for (int i = 0; i < buffer_size; i++) buf[i] = '\0';
			rd = read(fd, buf, buffer_size);
			if (rd == 0) {
				break;
			} else if (rd == -1) {
				printErrorIo1();
			}
			if (write(STDOUT_FILENO, buf, strlen(buf)) == -1) {
				printErrorIo1();
			}
		}

		// file close
		if (close(fd) == -1) {
			printErrorIo1();
		}
	}
	return 0;
}
