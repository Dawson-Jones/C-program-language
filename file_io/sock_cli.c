#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    struct sockaddr_in sin_addr;
    char str[1024] = "hello, world";

    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    sin_addr.sin_family = AF_INET;
    sin_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    sin_addr.sin_port = htons(9734);
    size_t serv_addr_size = sizeof(sin_addr);
    int result = connect(sockfd, (struct sockaddr *) &sin_addr, serv_addr_size);

    if (result == -1) {
        perror("oops: client1");
        exit(1);
    }
    write(sockfd, str, strlen(str));
    printf("--------\n");
    size_t receive_len = read(sockfd, str, 1024);
    str[receive_len] = '\0';
    printf("char from server = %s\n", str);
    close(sockfd);
    exit(0);
}
