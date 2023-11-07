#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <linux/if.h>

void parse_addr(char *addr, char **ip_str_p, char **port_str_p) {
    *ip_str_p = addr;
    if ((*port_str_p = strchr(addr, ':')) != NULL) {
        **port_str_p = '\0';
        (*port_str_p)++;
        return;
    }
    *port_str_p = NULL;
    return;
}

int main(int argc, char *argv[]) {
    int sockfd, b, c;
    struct sockaddr_in src, dst;
    char str[1024];

    int port;
    char *ip_str, *port_str;
    if (argc != 2) {
        perror("lack of params");
        return 1;
    }
    char *addr = argv[1];

    parse_addr(addr, &ip_str, &port_str);
    if (!(port_str && (port = atoi(port_str)))) {
        perror("input wrong");
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /* bind to a specific interface */
    // struct ifreq ifr;
    // char ifname[] = "enp0s5";
    // memset(&ifr, 0, sizeof(ifr));
    // strncpy(ifr.ifr_name, ifname, sizeof(ifname) / sizeof(char));
    // if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *) &ifr, sizeof(ifr)) != 0) {
    //     perror("setsockopt");
    //     exit(EXIT_FAILURE);
    // }

    /** ---- not nessary ------------
    src.sin_family = AF_INET;
    src.sin_port = htons(1314);
    inet_pton(AF_INET, "0.0.0.0", &src.sin_addr);
    // src.sin_addr.s_addr = inet_addr("127.0.0.1");    // same effect as above
    b = bind(sockfd, (struct sockaddr *) &src, sizeof(src));
    if (b == -1)
        perror("oops bind");
    // --------------------------------- */

    struct msghdr msg

    dst.sin_family = AF_INET;
    dst.sin_port = htons(port);
    if (inet_pton(AF_INET, ip_str, &dst.sin_addr) != 1) {
        perror("input ip wrong");
        return 1;
    }

    c = connect(sockfd, (struct sockaddr *) &dst, sizeof(dst));
    if (c == -1) {
        perror("oops: client1");
        exit(1);
    }

    while (1) {
        scanf("%s", str);
        write(sockfd, str, strlen(str) + 1);    // +1 means '\0'
        printf("------ transmit to srv: %s\n", str);
        // size_t receive_len = read(sockfd, str, 1024);
        // if (!receive_len) {
        //     break;
        // }
        // // str[receive_len] = '\0';
        // printf("------ receive from srv: %s\n", str);
    }

    printf("----- no data received, bye\n");
    close(sockfd);
    exit(0);
}
