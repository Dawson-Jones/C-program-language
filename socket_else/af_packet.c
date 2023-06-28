#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if.h>
#include <linux/ipv6.h>
#include <arpa/inet.h>

#define BUFFER_SIZE 2048

// 打开 Raw Socket，并绑定到指定的网络接口上
int open_packet_socket(const char *interface_name)
{
    int fd = socket(AF_PACKET, SOCK_RAW, htons(ETH_P_IP));
    if (fd < 0) {
        perror("Failed to open packet socket");
        exit(EXIT_FAILURE);
    }

    // get interface index
    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, interface_name, IFNAMSIZ - 1);
    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        perror("Failed to get interface index");
        exit(EXIT_FAILURE);
    }
    printf("%s index: %d\n", interface_name, ifr.ifr_ifindex);

    struct sockaddr_ll saddr;
    memset(&saddr, 0, sizeof(saddr));
    saddr.sll_family = AF_PACKET;
    saddr.sll_ifindex = ifr.ifr_ifindex;
    saddr.sll_protocol = htons(ETH_P_IP);
    if (bind(fd, (struct sockaddr *)&saddr, sizeof(saddr)) < 0) {
        perror("Failed to bind socket to interface");
        exit(EXIT_FAILURE);
    }

    return fd;
}

// 监听数据包并进行嗅探
void sniff_packets(int fd)
{
    char buffer[BUFFER_SIZE];
    while (1) {
        ssize_t n = read(fd, buffer, sizeof(buffer));
        if (n < 0) {
            perror("Error reading from socket");
            exit(EXIT_FAILURE);
        }
        if (n == 0) {  // EOF
            break;
        }
        // 解析数据包并打印信息
        struct iphdr *ipv4_header = (struct iphdr *)(buffer + sizeof(struct ether_header));
        printf("Received packet: source = %s, destination = %s, protocol = %d\n",
                inet_ntoa((struct in_addr){ipv4_header->saddr}),
                inet_ntoa((struct in_addr){ipv4_header->daddr}),
                ipv4_header->protocol);
    }
}

int main(int argc, char *argv[])
{
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <interface>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    const char *iface_name = argv[1];
    int fd = open_packet_socket(iface_name);
    printf("Start sniffing on interface %s\n", iface_name);
    sniff_packets(fd);

    return 0;
}