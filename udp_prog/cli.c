#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

#include "mytbf.h"

static void usage(void)
{
    printf("Usage: \n");
    printf("        -s <server ip>  ip for send message to\n");
    printf("        -t <total packet> total packet received\n");
    printf("        -p <server port> server port\n");
    printf("        -l <packet length>\n");
    printf("        -P <pps> send pps\n");
    printf("        -h Display this help\n");
}


static char *server_addr_str;
static int port   = 55001;
static int pps    = 10;
static int total  = 1 << 16;
static int length = 64;


int main(int argc, char **argv)
{
    int opt;
    int count = 0;

    while ((opt = getopt(argc, argv, "s:p:P:t:l:")) != -1) {
        switch (opt) {
        case 's':
            server_addr_str = optarg;
            break;
        case 't':
            total = atoi(optarg);
            break;
        case 'l':
            length = atoi(optarg);
            break;
        case 'p':
            port = atoi(optarg);
            break;
        case 'P':
            pps = atoi(optarg);
            break;
        default:
            usage();
            return 0;
        }
    }

    if (!server_addr_str) {
        usage();
        return -1;
    }

    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_desc < 0) {
        fprintf(stderr, "Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");
    
    // set IP and port;
    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(server_addr_str),
        .sin_port = htons(port),
    };
    int server_struct_length = sizeof(server_addr);

    // struct timeval tv;
    // tv.tv_sec = 0;
    // tv.tv_usec = 100000;
    // if (setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO,&tv,sizeof(tv)) < 0) {
    //     perror("Error");
    // }

    mytbf_t *tbf = mytbf_init(pps, pps * 5);
    // char client_message[length];
    while (count < total && mytbf_fetchtoken(tbf, 1)) {
        // memset(client_message, 0, sizeof(client_message));
        char client_message[6] = {};
        sprintf(client_message, "%d", ++count);

        printf("send %s\n", client_message);
        if (sendto(socket_desc, client_message, strlen(client_message), 0, 
                (struct sockaddr *) &server_addr, server_struct_length) < 0) {
            fprintf(stderr, "unable to send message\n");
            return -1;
        }
    }

    // for (int i = 0; i < 3; ++i) {
    //     // memset(client_message, 0, sizeof(client_message));
    //     client_message[0] = 4;
    //     sendto(socket_desc, client_message, strlen(client_message), 0, (struct sockaddr *) &server_addr, server_struct_length);
    // }

    close(socket_desc);
    mytbf_destroy(tbf);
    return 0;
}
