#include <stdlib.h>
#include <stdio.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#define MAXLINE 80

char *client_path = "client.socket";
char *server_path = "server.socket";

int main(int argc, char const *argv[]) {
    struct sockaddr_un cli_un, srv_un;
    char buf[100];
    int sock_fd, size, ret;

    sock_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (sock_fd < 0){
        perror("client socket error");
        exit(1);
    }

    // 一般显式调用bind函数，以便服务器区分不同客户端  
    memset(&cli_un, 0, sizeof(cli_un));
    cli_un.sun_family = AF_UNIX;
    stpcpy(cli_un.sun_path, client_path);
    size = offsetof(struct sockaddr_un, sun_path) + strlen(cli_un.sun_path);
    ret = bind(sock_fd, (struct sockaddr *) &cli_un, size);
    if (ret < 0) {
        perror("bind error");
        exit(1);
    }

    memset(&srv_un, 0, sizeof(srv_un));
    srv_un.sun_family = AF_UNIX;
    strcpy(srv_un.sun_path, server_path);
    size = offsetof(struct sockaddr_un, sun_path) + strlen(srv_un.sun_path);
    ret = connect(sock_fd, (struct sockaddr *) &srv_un, size);
    if (ret < 0) {
        perror("connect error");
        exit(1);
    }

    while(fgets(buf, MAXLINE, stdin) != NULL) {
        write(sock_fd, buf, strlen(buf));
        int n = read(sock_fd, buf, MAXLINE);
        if (n < 0) {
            printf("the other side has been closed.\n");
            break;
        } else {
            write(STDOUT_FILENO, buf, n);
        }
    }

    close(sock_fd);
    return 0;
}
