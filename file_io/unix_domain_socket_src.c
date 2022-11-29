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

char *socket_path = "server.socket";

int main(int argc, char const *argv[]) {
    struct sockaddr_un srv_un, cli_un;
    socklen_t cliun_len;
    int listen_fd, ret, size;
    char buf[MAXLINE];
    listen_fd = socket(AF_UNIX, SOCK_STREAM, 0);
    if (listen_fd < 0) {
        perror("socket error");
        exit(1);
    }

    memset(&srv_un, 0, sizeof(srv_un));
    srv_un.sun_family = AF_UNIX;
    strcpy(srv_un.sun_path, socket_path);
    size = offsetof(struct sockaddr_un, sun_path) + strlen(srv_un.sun_path);
    unlink(socket_path);

    ret = bind(listen_fd, (struct sockaddr *) &srv_un, size);
    if (ret < 0) {
        perror("bind error");
        exit(1);
    }
    printf("UNIX domain socket bound\n");

    ret = listen(listen_fd, 20);
    if (ret < 0) {
        perror("listen error");
        exit(1);
    }
    printf("Acceping connections ...\n");

    while (1) {
        cliun_len = sizeof(cli_un);
        int conn_fd = accept(listen_fd, (struct sockaddr *) &cli_un, &cliun_len);
        if (conn_fd < 0) {
            perror("accept error");
            continue;
        }

        while (1) {
            int n = read(conn_fd, buf, sizeof(buf));
            if (n < 0) {
                perror("read error");
                break;
            } else if (n == 0) {
                printf("EOF\n");
                break;
            }
            printf("received: %s", buf);

            for (int i = 0; i < n; i++)
                buf[i] = toupper(buf[i]);

            write(conn_fd, buf, n);
        }
        
        close(conn_fd);
    }
    
    close(listen_fd);
    return 0;
}
