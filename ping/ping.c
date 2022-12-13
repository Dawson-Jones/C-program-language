#include <stdio.h>
#include <string.h>
#include <unistd.h>
//#include <stddef.h>
//#include <stdbool.h>
//#include <sys/types.h>
#include <errno.h>
#include <sys/time.h>
#include <arpa/inet.h>

#define MTU 1500
#define RECV_TIMEOUT_USEC 100000

const char *magic = "1234567890";
#define MAGIC_LEN 11


struct icmp_echo {
    u_int8_t type;
    u_int8_t code;
    u_int16_t checksum;

    u_int16_t identifier;
    u_int16_t sequence_number;

    double sending_ts;
    char magic[MAGIC_LEN];
};

double get_timestamp() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return tv.tv_sec + ((double) tv.tv_usec) / 1000000;
}

u_int16_t calculate_checksum(unsigned char *buffer, int bytes) {
    u_int32_t checksum = 0;
    unsigned char *end = buffer + bytes;

    // odd bytes add last byte and reset end
    if (bytes % 2 == 1) {
        end = buffer + bytes - 1;
        checksum += (*end) << 8;
    }

    // add words of two bytes, one by one
    while (buffer < end) {
        checksum += buffer[0] << 8;
        checksum += buffer[1];
        buffer += 2;
    }

    // add carry if any
    u_int32_t carray = checksum >> 16;
    while (carray) {
        checksum = (checksum & 0xffff) + carray;
        carray = checksum >> 16;
    }

    // negate it
    checksum = ~checksum;

    return checksum & 0xffff;
}

int send_echo_request(int sock, struct sockaddr_in *addr, int ident, int seq) {
    struct icmp_echo icmp;
    bzero(&icmp, sizeof(icmp));

    icmp.type = 8;
    icmp.code = 0;
    icmp.identifier = htons(ident);
    icmp.sequence_number = htons(seq);

    strncpy(icmp.magic, magic, MAGIC_LEN);
    icmp.sending_ts = get_timestamp();
    icmp.checksum = htons(calculate_checksum((unsigned char *) &icmp, sizeof(icmp)));

    int len = sendto(sock, &icmp, sizeof(icmp), 0, (struct sockaddr *) addr, sizeof(*addr));
    if (len == -1)
        return -1;

    return 0;
}

int recv_echo_reply(int sock, int ident) {
    char buffer[MTU];
    struct sockaddr_in peer_addr;

    int addr_len = sizeof(peer_addr);
    int buf_len = recvfrom(sock, buffer, sizeof(buffer), 0, (struct sockaddr *) &peer_addr, &addr_len);
    if (buf_len == -1) {
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            return 0;
        }

        return -1;
    }

    struct icmp_echo *icmp = (struct icmp_echo *) (buffer + 20);
    if (icmp->type != 0 || icmp->code != 0)
        return 0;

    if (ntohs(icmp->identifier) != ident)
        return 0;

    printf("%s seq=%d %5.2fms\n", inet_ntoa(peer_addr.sin_addr), ntohs(icmp->sequence_number),
           (get_timestamp() - icmp->sending_ts) * 1000);

    return 0;
}


int ping(const char *ip) {
    int ret;
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_port = 0;
    if (inet_aton(ip, (struct in_addr *) &addr.sin_addr.s_addr) == 0) {
        perror("inet_aton");
        return -1;
    }

    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    struct timeval tv = {0, RECV_TIMEOUT_USEC};
    ret = setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
    if (ret == -1) {
        perror("setsockopt");
        return -1;
    }

    double next_ts = get_timestamp();
    int ident = getpid();
    int seq = 1;

    for (;;) {
        if (get_timestamp() >= next_ts) {
            ret = send_echo_request(sock, &addr, ident, seq);
            if (ret == -1) {
                perror("Send failed");
            }

            next_ts += 1;
            seq += 1;
        }

        ret = recv_echo_reply(sock, ident);
        if (ret == -1) {
            perror("Receive failed");
        }
    }

    return 0;
}

int main(int argc, const char *argv[]) {
    if (ping(argv[1]) == -1) {
        fprintf(stderr, "ping: %d, %s", errno, strerror(errno));
    }
    return 0;
}

