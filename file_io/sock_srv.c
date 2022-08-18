#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define BUF_LEN 1024
#define PORT 8001


int main() {
    /*
     * sock_stream 是有保障的(即能保证数据正确传送到对方)面向连接的SOCKET，多用于资料(如文件)传送。
     * sock_dgram 是无保障的面向消息的socket ， 主要用于在网络上发广播信息。
     */
    //                     ipv4                  
    int sock_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in cli_addr, srv_addr;
    memset(&srv_addr, 0, sizeof(srv_addr));  // 初始化 sock_addr
    srv_addr.sin_family = AF_INET;
    // inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);
    srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    srv_addr.sin_port = htons(PORT);

    // 让套接字和特定的 ip 端口绑定起来, 使此 ip 端口传过来的消息能够被该套接字接收到
    if (bind(sock_fd, (struct sockaddr *) &srv_addr, sizeof(srv_addr)) != 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    listen(sock_fd, 20);

    socklen_t cli_addr_size = sizeof(cli_addr);

    while (1) {
        int conn_fd = accept(sock_fd, (struct sockaddr *) &cli_addr, &cli_addr_size);
        printf("addr: %s, port: %d, fd: %d\n", inet_ntoa(cli_addr.sin_addr), ntohs(cli_addr.sin_port), conn_fd);
        // 到这里阻塞，下面的直到消息发送过来不会执行

        int bytes_len;
        char buffer[BUF_LEN] = {0};
        while ((bytes_len = read(conn_fd, buffer, BUF_LEN))) {
        // while ((bytes_len = recv(client_socket, buffer, BUF_LEN, 0))) {
            printf("%s\n", buffer);
            write(conn_fd, buffer, strlen(buffer));
            // send(client_socket, buffer, strlen(buffer), 0);
            if (!bytes_len){
                break;
            }
        }
        printf("client close connect.\n");
        close(conn_fd);
    }

    close(sock_fd);
    return 0;
}
