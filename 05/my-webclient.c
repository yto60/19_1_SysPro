#include <arpa/inet.h>	/* htons, inet_addr */
#include <stdio.h>			/* FILE, BUFSIZE */
#include <string.h>			/* memset */
#include <sys/socket.h> /* socket, connect */
#include <sys/types.h>	/* read */
#include <sys/uio.h>		/* read */
#include <unistd.h>			/* read, write, close */

int main(int argc, char *argv[]) {

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);

	//  ソケットの生成，サーバへの接続部分を完成させなさい．
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(10000);
	serveraddr.sin_addr.s_addr = inet_addr("127.0.0.1");

	connect(sockfd, (struct sockaddr *)&serveraddr, sizeof(serveraddr));

	//  HTTPリクエストの送信部分を完成させなさい．
	write(sockfd, "GET / HTTP/1.0\r\n\r\n", 30);

	//  HTTPレスポンスを受信し，メッセージボディだけを標準出力へ出力しなさい．
	FILE *fin, *fout;
	char buf[BUFSIZ];

	fin = fdopen(sockfd, "r");
	fout = fdopen(STDOUT_FILENO, "w");
	fgets(buf, sizeof(buf), fin);
	while (fgets(buf, sizeof(buf), fin) != NULL) {
		// 最初の \r\n を探す
		if (strcmp(buf, "\r\n") == 0) {
			break;
		}
	}
	while (fgets(buf, sizeof(buf), fin) != NULL) {
		// 2回目の \r\n が見つかるまで出力
		if (strcmp(buf, "\r\n") == 0) {
			break;
		}
		fprintf(fout, "%s", buf);
	}

	close(sockfd);
	return 0;
}
