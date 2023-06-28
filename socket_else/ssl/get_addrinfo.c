#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


int main(int argc, char const *argv[])
{
    if (argc != 3) {
        // hostname port
        fprintf(stderr, "Usage: %s <hostname> <service>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct addrinfo hints;
    struct addrinfo *result, *rp;
    int sfd, s;

    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM; /* TCP socket */
    hints.ai_flags = 0;
    hints.ai_protocol = 0; /* Any protocol */

    s = getaddrinfo(argv[1], argv[2], &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }

    for (rp = result; rp != NULL; rp = rp->ai_next) {
        sfd = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (sfd == -1) {
            continue;
        }

        if (connect(sfd, rp->ai_addr, rp->ai_addrlen) != -1) {
            break; /* Success */
        }

        close(sfd);
    }

    if (rp == NULL) {
        fprintf(stderr, "Could not connect\n");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);

    // 使用 sfd 通信
    void communacate(int fd);
    communacate(sfd);

    return 0;
}

void communacate(int fd) {
    char buf[1024];
    while (1) {
        scanf("%s", buf);
        write(fd, buf, strlen(buf));
        printf("------ transmit to srv: %s\n", buf);

        size_t receive_len = read(fd, buf, 1024);
        if (!receive_len) {
            break;
        }
        buf[receive_len] = '\0';
        printf("------ receive from srv: %s\n", buf);
    }
    
}