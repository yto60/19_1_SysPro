#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>

char *not_found_body = "<html><body><center><h1>404 Not Found!</h1></center></body></html>\n";

int find_file(char *path) {
    struct stat info;
    if (stat(path, &info) == -1) {
        return -1;
    }
    return (int)info.st_size;        // file size
}

char *get_content_type(char *filename) {
    if (filename[strlen(filename) - 1] == 'g') {        // jpeg or jpg
        return "image/jpeg";

    } else if (filename[strlen(filename) - 1] == 'f') {        // gif
        return "image/gif";

    } else if (filename[strlen(filename) - 1] == 'l' || filename[strlen(filename) - 1] == 'm') {        // html or htm
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

void session(int fd, char *cli_addr, int cli_port) {

    FILE *fin, *fout;
    fin = fdopen(fd, "r"); fout = fdopen(fd, "w");

    /* read request line */
    char request_buf[BUFSIZ];
    if (fgets(request_buf, sizeof(request_buf), fin) == NULL) {
        fflush(fout);
        fclose(fin);
        fclose(fout);
        close(fd);
        return;        // disconnected
    }

    /* parse request line */
    char method[BUFSIZ];
    char uri[BUFSIZ], *path;
    char version[BUFSIZ];
    sscanf(request_buf, "%s %s %s", method, uri, version);
    path = &(uri[1]);

    printf("HTTP Request: %s %s %s %s\n", method, uri, path, version);
    fflush(stdout);

    /* read/parse header lines */
    while (1) {

        /* read header lines */
        char headers_buf[BUFSIZ];
        if (fgets(headers_buf, sizeof(headers_buf), fin) == NULL) {
            fclose(fin);
            fclose(fout);
            close(fd);
            return;            // disconnected from client
        }

        /* check header end */
        if (strcmp(headers_buf, "\r\n") == 0) {
            break;
        }

        /* parse header lines */
        char header[BUFSIZ];
        char value[BUFSIZ];
        sscanf(headers_buf, "%s %s", header, value);

        printf("Header: %s %s\n", header, value);
        fflush(stdout);

    } // while


    /*

      送信部分を完成させなさい．

     */


    /* close connection */
    fclose(fin);
    fclose(fout);
    close(fd);
    printf("Connection closed.\n");
    fflush(stdout);
}

int main(int argc, char *argv[]) {

    int listfd, connfd, optval = 1, port = 10000;
    unsigned int addrlen;
    struct sockaddr_in saddr, caddr;

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

    while (1) {
        addrlen = sizeof(caddr);
        connfd = accept(listfd, (struct sockaddr *)&caddr, (socklen_t*)&addrlen);
        session(connfd, inet_ntoa(caddr.sin_addr), ntohs(caddr.sin_port));
    }

    return 0;
}
