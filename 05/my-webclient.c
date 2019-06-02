#include <sys/socket.h>     /* socket, connect */
#include <string.h>         /* memset */
#include <arpa/inet.h>      /* htons, inet_addr */
#include <sys/types.h>      /* read */
#include <sys/uio.h>        /* read */
#include <unistd.h>         /* read, write, close */

int main(int argc, char *argv[]) {

    int sockfd;

    /*

     ソケットの生成，サーバへの接続部分を完成させなさい．

    */



    /*

     HTTPリクエストの送信部分を完成させなさい．

    */



    /*

     HTTPレスポンスを受信し，メッセージボディだけを標準出力へ出力しなさい．
  
    */



    close(sockfd);
    return 0;
}
