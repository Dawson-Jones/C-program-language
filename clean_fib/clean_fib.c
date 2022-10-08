#include <bpf/libbpf.h>
#include <poll.h>
#include <sys/socket.h>
#include <linux/rtnetlink.h>



int main(int argc, char const *argv[])
{
    char buf[1024];
    struct pollfd fds_route, fds_arp;

    struct nlmsghdr *nh;
    int sock_rt = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    int sock_arp = socket(AF_NETLINK, SOCK_RAW, NETLINK_ROUTE);
    struct sockaddr_nl la = {}, lr = {};

    lr.nl_family = AF_NETLINK;
    lr.nl_groups = RTMGRP_IPV6_ROUTE | RTMGRP_IPV4_ROUTE | RTMGRP_NOTIFY;

    bind(sock_rt, (struct sockaddr *) &lr, sizeof(lr));
    fds_route.fd = sock_rt;
    fds_route.events = POLLIN;

    la.nl_family = AF_NETLINK;
    la.nl_groups = RTMGRP_NEIGH | RTMGRP_NOTIFY;
    bind(sock_arp, (struct sockaddr *) &la, sizeof(la));
    fds_arp.fd = sock_arp;
    fds_arp.events = POLLIN;

    while (1) {
        memset(buf, 0, sizeof(buf));
        if (poll(&fds_route, 1, 3) == POLLIN) {
            recv(sock_rt, buf, sizeof(buf), 0);
            nh = (struct nlmsghdr *) buf;
        } else if (poll(&fds_arp, 1, 3) == POLLIN) {
            recv(sock_arp, buf, sizeof(buf), 0);
            nh = (struct nlmsghdr *) buf;
        } else {
            continue;
        }
		if (nh->nlmsg_type == RTM_NEWNEIGH || nh->nlmsg_type == RTM_DELNEIGH ||
			nh->nlmsg_type == RTM_NEWROUTE || nh->nlmsg_type == RTM_DELROUTE) {

            printf("netlink type: %d\n", nh->nlmsg_type);
        }
    }
    return 0;
}
