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
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));  // 初始化 sock_addr
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(PORT);

    // 让套接字和特定的 ip 端口绑定起来, 使此 ip 端口传过来的消息能够被该套接字接收到
    bind(serv_sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr));
    listen(serv_sock, 20);

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    char buffer[BUF_LEN] = {0};
    while (1) {
        int client_socket = accept(serv_sock, (struct sockaddr *) &client_addr, &client_addr_size);
        printf("addr: %s, port: %d, fd: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), client_socket);
        // 非常奇怪， 到这里阻塞，下面的直到消息发送过来不会执行

        int bytes_len;
        while ((bytes_len = read(client_socket, buffer, BUF_LEN))) {
        // while ((bytes_len = recv(client_socket, buffer, BUF_LEN, 0))) {
            printf("%s\n", buffer);
            write(client_socket, buffer, strlen(buffer));
            // send(client_socket, buffer, strlen(buffer), 0);
            if (!bytes_len){
                break;
            }
        }
        printf("client close connect.\n");
        close(client_socket);
    }
    close(serv_sock);
    return 0;
}
