#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[]) {
    int sockfd, b, c;
    struct sockaddr_in src, dst;
    char str[1024];

    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    // /** ---- not nessary ------------
    src.sin_family = AF_INET;
    src.sin_port = htons(1314);
    inet_pton(AF_INET, "127.0.0.1", &src.sin_addr);
    // src.sin_addr.s_addr = inet_addr("127.0.0.1");    // same effect as above
    b = bind(sockfd, (struct sockaddr *) &src, sizeof(src));
    if (b == -1)
        perror("oops bind");
    // --------------------------------- */

    dst.sin_family = AF_INET;
    dst.sin_port = htons(8001);
    inet_pton(AF_INET, "127.0.0.1", &dst.sin_addr);
    c = connect(sockfd, (struct sockaddr *) &dst, sizeof(dst));
    if (c == -1) {
        perror("oops: client1");
        exit(1);
    }

    while (1) {
        scanf("%s", str);
        write(sockfd, str, strlen(str) + 1);    // +1 means '\0'
        printf("------ transmit to srv: %s\n", str);
        size_t receive_len = read(sockfd, str, 1024);
        if (!receive_len) {
            break;
        }
        // str[receive_len] = '\0';
        printf("------ receive from srv: %s\n", str);
    }

    printf("----- no data received, bye\n");
    close(sockfd);
    exit(0);
}
