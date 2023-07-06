/*
sudo ip tuntap add dev tun0 mode tun
sudo ip addr add 10.1.2.3/24 dev tun0
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <fcntl.h>

void sockaddr_to_ipv4(const struct sockaddr *sa) {
    if (sa->sa_family != AF_INET) {
        printf("Not an IPv4 address\n");
        return;
    }

    struct sockaddr_in *sin = (struct sockaddr_in *)sa;
    char ip[INET_ADDRSTRLEN];
    const char *addr = inet_ntop(AF_INET, &(sin->sin_addr), ip, INET_ADDRSTRLEN);
    if (addr == NULL) {
        perror("inet_ntop");
        return;
    }

    printf("IPv4 Address: %s\n", addr);
}

int main(int argc, char const *argv[])
{
    struct ifreq ifr;
    /* 不 work */
    // char *clone_dev = "/dev/net/tun";
    // int fd = open(clone_dev, O_RDWR);
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    strncpy(ifr.ifr_ifrn.ifrn_name, "tun0", strlen("tun0"));
    // ifr.ifr_ifru.ifru_addr = 
    ioctl(fd, SIOCGIFADDR, (void *) &ifr);

    sockaddr_to_ipv4(&ifr.ifr_ifru.ifru_addr);

    return 0;
}
