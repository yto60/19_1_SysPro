#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <unistd.h>

char *not_found_body =
		"<html><body><center><h1>404 Not Found!</h1></center></body></html>\n";
char *index_file = "index.html";

int find_file(char *path) {
	struct stat info;
	if (stat(path, &info) == -1) {
		return -1;
	}
	return (int)info.st_size; // file size
}

char *get_content_type(char *filename) {
	if (filename[strlen(filename) - 1] == 'g') { // jpeg or jpg
		return "image/jpeg";

	} else if (filename[strlen(filename) - 1] == 'f') { // gif
		return "image/gif";

	} else if (filename[strlen(filename) - 1] == 'l' ||
						 filename[strlen(filename) - 1] == 'm') { // html or htm
		return "text/html";

		/* others */
	} else {
		return "text/plain";
	}
}

void send_content_data(FILE *from, FILE *to) {
	char buf[BUFSIZ];
	int counter = 0;
	while (1) {
		int size = fread(buf, 1, BUFSIZ, from);
		if (size > 0) {
			fwrite(buf, size, 1, to);
		} else {
			break;
		}
		counter++;
	}
}

int session(int fd, char *cli_addr, int cli_port) {

	FILE *fin, *fout;
	fout = fdopen(fd, "w");
	fin = fdopen(fd, "r");

	int connection_close = 0;

	/* read request line */
	char request_buf[BUFSIZ];
	while (fgets(request_buf, sizeof(request_buf), fin) == NULL)
		;
	// if (fgets(request_buf, sizeof(request_buf), fin) == NULL) {
	// 	fflush(fout);
	// 	fclose(fin);
	// 	fclose(fout);
	// 	close(fd);
	// 	printf("1 Connection closed (child).\n");
	// 	fflush(stdout);
	// 	return 1; // disconnected
	// }

	/* parse request line */
	char method[BUFSIZ];
	char uri[BUFSIZ], *path;
	char version[BUFSIZ];

	sscanf(request_buf, "%s %s %s", method, uri, version);
	path = strlen(uri) > 1 ? &(uri[1]) : index_file;
	// pathが指定されていない場合は index_file (index.html) を返すように

	printf("HTTP Request: %s %s %s %s\n", method, uri, path, version);
	fflush(stdout);

	/* read/parse header lines */
	while (1) {

		/* read header lines */
		char headers_buf[BUFSIZ];
		while (fgets(headers_buf, sizeof(headers_buf), fin) == NULL)
			;
		// if (fgets(headers_buf, sizeof(headers_buf), fin) == NULL) {
		// 	fclose(fin);
		// 	fclose(fout);
		// 	close(fd);
		// 	printf("2 Connection closed (child).\n");
		// 	fflush(stdout);
		// 	return 1; // disconnected from client
		// }

		/* check header end */
		if (strcmp(headers_buf, "\r\n") == 0) {
			printf("check header end\n");
			break;
		}

		/* parse header lines */
		char header[BUFSIZ];
		char value[BUFSIZ];
		sscanf(headers_buf, "%s %s", header, value);

		printf("Header: %s %s\n", header, value);
		fflush(stdout);

		if (strcmp(header, "Connection:") == 0 && strcmp(value, "close") == 0) {
			connection_close = 1;
		}

	} // while

	//   送信部分を完成させなさい．

	// get content type and status code
	char *content_type;
	char *status_code;
	int content_length = find_file(path);
	int notfound = content_length == -1;
	if (notfound) {
		content_type = get_content_type(".html");
		status_code = "404 Not Found";
		content_length = strlen(not_found_body);
	} else {
		content_type = get_content_type(path);
		status_code = "200 OK";
	}

	// send header
	fprintf(fout, "HTTP/1.1 %s\r\n", status_code);
	fprintf(fout, "Content-Type: %s\r\n", content_type);
	fprintf(fout, "Content-Length: %d\r\n", content_length);
	fprintf(fout, "\r\n");

	// send body
	if (notfound) {
		fprintf(fout, "%s", not_found_body);
	} else {
		FILE *fd = fopen(path, "r");
		send_content_data(fd, fout);
	}
	fflush(fout);

	/* close connection */
	if (connection_close) {
		fclose(fin);
		fclose(fout);
		close(fd);
		printf("Connection closed (child).\n");
		fflush(stdout);
		return 1;
	} else {
		fflush(fout);
		fflush(fin);
		return 0;
	}
}

int while_listening(int listfd) {
	struct sockaddr_in caddr;
	unsigned int addrlen = sizeof(caddr);
	int connfd = accept(listfd, (struct sockaddr *)&caddr, (socklen_t *)&addrlen);
	printf("accept\n");
	fflush(stdout);

	// fork
	printf("fork\n");
	fflush(stdout);
	pid_t pid = fork();

	if (pid == -1) {
		// エラー処理
		perror("fork");
		exit(errno);
	}

	if (pid == 0) {
		// child process
		while (1) {
			if (session(connfd, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port)) ==
					1) {
				break;
			}
		}
		return 0;
	} else {
		// parent process
		while_listening(listfd);

		int st;
		wait(&st);
		printf("child process finished\n");
		if (WIFEXITED(st)) {
			printf("exit status=%d\n", WEXITSTATUS(st));
		}

		// close connection (parent)
		close(connfd);
		printf("Connection closed (parent).\n");
		fflush(stdout);
		return 0;
	}
}

int main(int argc, char *argv[]) {

	int listfd, optval = 1, port = 10000;
	// unsigned int addrlen;
	struct sockaddr_in saddr;

	if (argc >= 2) {
		port = atoi(argv[1]);
	}

	listfd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(listfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(port);
	saddr.sin_addr.s_addr = htonl(INADDR_ANY);

	bind(listfd, (struct sockaddr *)&saddr, sizeof(saddr));

	listen(listfd, 10);

	while_listening(listfd);

	return 0;
}
