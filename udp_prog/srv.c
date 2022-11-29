#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <getopt.h>
#include <signal.h>

#define BUCKET_COUNT 10000

static void usage(void)
{
    printf("Usage: \n");
    printf("        -p <server port> server port to bind\n");
    printf("        -h Display this help\n");
}

static char bucket[BUCKET_COUNT] = {};

static void handle(int sig)
{
    int counter = 0;
    for (int i = 0; i < BUCKET_COUNT; ++i) {
        if (bucket[i] != 1) {
            if (bucket[i] == 0) {
                counter++;
                printf("loss the %dth packet, %d\n", i + 1, bucket[i]);
            } else {
                printf("reduplicate packet: %dth, %d\n", i + 1, bucket[i]);
            }
        }
    }

    printf("loss %d packets\n", counter);

    exit(0);
}

int main(int argc, char **argv)
{
    int opt;
    int port = 55001;
    while ((opt = getopt(argc, argv, "p:")) != -1) {
        switch (opt) {
        case 'p':
            port = atoi(optarg);
            break;
        default:
            usage();
            return 0;
        }
    }
    int socket_desc;
    struct sockaddr_in server_addr, client_addr;
    int client_struct_length = sizeof(client_addr);
    // char server_message[2000] = {};

    // create udp socket
    socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (socket_desc < 0) {
        fprintf(stderr, "Error while creating socket\n");
        return -1;
    }
    printf("Socket created successfully\n");

    // Set port and IP
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // bind o the set port and IP
    if (bind(socket_desc, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "Counldn't bind to the port\n");
        return -1;
    }

    printf("Listening for incoming message...\n\n");

    signal(SIGINT, handle);
	signal(SIGTERM, handle);
	signal(SIGABRT, handle);

    int err;
    int number = 0;

    while (1) {
        char client_message[6] = {};
        err = recvfrom(socket_desc, client_message, sizeof(client_message), 0, (struct sockaddr *) &client_addr, &client_struct_length);
        if (err == -1) {
            perror("recvfrom\n");
            exit(EXIT_FAILURE);
        }
        
        // if (client_message[0] == 4) {
        //     printf("EOT received\n");
        //     break;
        // }

        number = atoi(client_message);
        if (!number) {
            fprintf(stderr, "can not convert to a number\n");
            continue;
        }
        printf("recv %d\n", number);

        bucket[number - 1]++;
        if (number >= BUCKET_COUNT)
            break;
    }

    close(socket_desc);
    kill(0, SIGINT);
    return 0;
}
