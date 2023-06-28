#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <net/if.h>
#include <unistd.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <errno.h>
#include <linux/types.h>

struct segroute {
    __be32 vxlan_flag;
    // __be32 vxlan_flag: 8,
    //        reserved: 24;
    __be32 vx_vni;
    // __be32 vx_vni: 24,
    //        reserved2: 8;
    __be32 seg_list[3];
};

#define SERVER_PORT 55001
#define BUFF_LEN 1024

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

int main(int argc, char **argv) {
    char *srv_ip = argv[1];
    char *seg_list = argv[2];
    struct sockaddr_in dst_addr = {};
    int qos_value = 0;

    if (argc < 3) {
        fprintf(stderr, "prog <serverip> <seglist> [qos]\n");
        exit(EXIT_FAILURE);
    }
    if (argc == 4) {
        qos_value = atoi(argv[3]);
    }

    int sock_fd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP/* or 0*/);
    if (sock_fd < 0) {
        perror("create socket failed\n");
        exit(EXIT_FAILURE);
    }
    if (qos_value != 0) {
        if (setsockopt(sock_fd, IPPROTO_IP, IP_TOS, (void *) &qos_value, sizeof(qos_value)) < 0) {
            perror("setsockopt");
            exit(EXIT_FAILURE);
        }
    }

    dst_addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, srv_ip, &dst_addr.sin_addr) != 1) {
        fprintf(stderr, "error: inet_pron\n");
        exit(EXIT_FAILURE);
    }
    dst_addr.sin_port = htons(SERVER_PORT);

    char buf[BUFF_LEN] = {};
    struct segroute *sr = (struct segroute *) buf;
    sr->vxlan_flag = 0x08;
    sr->vx_vni = htonl(1 << 8);
    if (inet_pton(AF_INET, seg_list, &sr->seg_list[0]) != 1) {
        fprintf(stderr, "error: inet_pron\n");
        exit(EXIT_FAILURE);
    } else {
        printf("seg list: %x\n", sr->seg_list[0]);
    }

    
    for (int i = 0; i < 10; ++i) {
        memcpy(sr + 1, "helo, world", sizeof("hello, world"));
        printf("client: -----\n");
        sendto(sock_fd, buf, BUFF_LEN, 0, (struct sockaddr *) &dst_addr, sizeof(dst_addr));
        printf("sent-------\n");
        msleep(200);
    }

    // memset(buf, 0, sizeof(buf));
    // int len, n;
    // n = recvfrom(sock_fd,(char *) buf, BUFF_LEN, MSG_WAITALL, (struct sockaddr *) &dst_addr, &len);
    // buf[n] = '\0';
    // printf("server: %s\n", buf);

    close(sock_fd);
    return 0;
}

