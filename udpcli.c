#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <unistd.h>

static void usage(void)
{
    printf("Usage: \n");
    printf("        -s <server ip>  ip for send message to\n");
    printf("        -p <server port> server port\n");
    printf("        -h Display this help\n");
}


static char *server_addr_str;
static int port   = 55001;
static int pps    = 10;
static int total  = 1 << 16;


int main(int argc, char **argv)
{
    int opt;
    int count = 0;

    while ((opt = getopt(argc, argv, "s:p:P:t:l:")) != -1) {
        switch (opt) {
        case 's':
            server_addr_str = optarg;
            break;
        case 'p':
            port = atoi(optarg);
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

    char buf[1024] = {};
    sprintf(buf, "%d", ++count);

    printf("send %s\n", buf);
    if (sendto(socket_desc, buf, strlen(buf), 0, 
            (struct sockaddr *) &server_addr, server_struct_length) < 0) {
        fprintf(stderr, "unable to send message\n");
        return -1;
    }

    struct sockaddr_in dst_addr = {};
    memset(buf, 0, sizeof(buf));
    int len = 0;
    int n = recvfrom(socket_desc, (char *) buf, sizeof(buf), MSG_WAITALL, (struct sockaddr *) &dst_addr, &len);
    buf[n] = '\0';
    printf("server: %s\n", buf);

    close(socket_desc);
    return 0;
}
