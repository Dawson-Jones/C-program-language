#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080


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
    int client_socket = accept(serv_sock, (struct sockaddr *) &client_addr, &client_addr_size);
    printf("addr: %s\n", inet_ntoa(client_addr.sin_addr));

    char buffer[1024] = {0};
    read(client_socket, buffer, 1024);
    printf("%s", buffer);
    write(client_socket, buffer, strlen(buffer));
    close(client_socket);

    close(serv_sock);
    return 0;
}
