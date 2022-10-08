#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

struct segroute {
    int vxlan_flag;
    // __be32 vx_vni: 24;
    // __u8 reserverd;
    int vx_vni;
    int seg_list[3];
};

#define SERVER_PORT 55001
#define BUFF_LEN 1024

void usage() {
    
}

int main(int argc, char **argv) {
    if (argc != 3) {
        fprintf(stderr, "prog <serverip> <seglist>\n");
        exit(EXIT_FAILURE);
    }

    char *srv_ip = argv[1];
    char *seg_list = argv[2];
    struct sockaddr_in dst_addr = {};

    int sock_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock_fd < 0) {
        fprintf(stderr, "create socket fail\n");
        exit(EXIT_FAILURE);
    }

    dst_addr.sin_family = AF_INET;
    // dst_addr.sin_addr.s_addr = INADDR_ANY;
    if (inet_pton(AF_INET, srv_ip, &dst_addr.sin_addr) != 1) {
        fprintf(stderr, "error: inet_pron\n");
        exit(EXIT_FAILURE);
    }
    dst_addr.sin_port = htons(SERVER_PORT);

    char buf[BUFF_LEN] = {};
    struct segroute *sr = (struct segroute *) buf;
    sr->vxlan_flag = 8;
//    sr->vx_vni = 1 << 16;
    sr->vx_vni = htonl(1 << 8);
    if (inet_pton(AF_INET, seg_list, &sr->seg_list[0]) != 1) {
        fprintf(stderr, "error: inet_pron\n");
        exit(EXIT_FAILURE);
    } else {
        printf("seg list: %x\n", sr->seg_list[0]);
    }

    memcpy(sr + 1, "helo, world", sizeof("hello, wordld"));
    printf("client: -----\n");
    sendto(sock_fd, buf, BUFF_LEN, 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));
    printf("sent-------\n");

    memset(buf, 0, sizeof(buf));
    int len, n;
    n = recvfrom(sock_fd,(char *) buf, BUFF_LEN, MSG_WAITALL, (struct sockaddr *) &dst_addr, &len);
    buf[n] = '\0';
    printf("server: %s\n", buf);

    close(sock_fd);
    return 0;
}
