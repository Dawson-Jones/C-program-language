#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>    
#include <time.h>
#include <errno.h>
#include <stdbool.h>
#include <net/if.h>
#include <linux/lwtunnel.h>

#include "mnl.h"


struct buf_s {
    void *buf;
    int len;
};

void print_debug(struct nlmsghdr *n);

unsigned int get_port_id(int sock_fd) {
    int ret;
    struct sockaddr_nl addr;
    socklen_t addr_len = sizeof(addr);

    ret = getsockname(sock_fd, (struct sockaddr *)&addr, &addr_len);
    if (ret == -1) {
        perror("getsockname");
        exit(EXIT_FAILURE);
    }
    printf("port_id: %d\n", addr.nl_pid);
    return addr.nl_pid;
}


int send_req_msg(int sock_fd, struct nlmsghdr *nlh) {
    int ret;
    struct sockaddr_nl nladdr;
    nladdr.nl_family = AF_NETLINK;
    printf("send payload: \n");
    print_debug(nlh);
    // ret = sendto(sock_fd, (void *) nlh, nlh->nlmsg_len, 0, (struct sockaddr *)&nladdr, sizeof(nladdr));
    ret = send(sock_fd, (void *) nlh, nlh->nlmsg_len, 0);
    if (ret < 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    return ret;
}

int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len) {
    memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
    while (RTA_OK(rta, len)) {
        if (rta->rta_type == RTA_ENCAP) {
            printf("RTA_ENCAP yes\n");
        }
        tb[rta->rta_type] = rta;
        rta = RTA_NEXT(rta, len);
    }

    if (len)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n", len, rta->rta_len);
	return 0;
}

void lwt_print_encap(FILE *fp, struct rtattr *encap_type, struct rtattr *encap) {
    unsigned short et = *(__u16 *) RTA_DATA(encap_type);
    switch (et) {
    case LWTUNNEL_ENCAP_MPLS:
        printf("encap: LWTUNNEL_ENCAP_MPLS\n");
        break;
    case LWTUNNEL_ENCAP_IP:
        printf("encap: LWTUNNEL_ENCAP_IP\n");
        break;
    case LWTUNNEL_ENCAP_ILA:
        printf("encap: LWTUNNEL_ENCAP_ILA\n");
        break;
    case LWTUNNEL_ENCAP_IP6:
        printf("encap: LWTUNNEL_ENCAP_IP6\n");
        break;
    default:
        printf("encap: %d\n", et);
        break;
    }
}

int print_route(struct nlmsghdr *nlh) {
    FILE *fp = stdout;
    struct rtmsg *r = NLMSG_DATA(nlh);
    int len = nlh->nlmsg_len;
    struct rtattr *tb[RTA_MAX + 1] = {};

    len -= NLMSG_LENGTH(sizeof(*r));
    if (len < 0) {
        fprintf(fp, "invalid route nlmsg\n");
        return -1;
    }

    int host_len = af_bit_len(r->rtm_family);
    parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
    __u32 table = rtm_get_table(r, tb);
    printf("table: %d\n", table);

    if (tb[RTA_ENCAP]) {
        lwt_print_encap(fp, tb[RTA_ENCAP_TYPE], tb[RTA_ENCAP]);
    } else {
        printf("encap: NULL\n");
    }

    return 0;
}


int recv_resp_msg(int sock_fd, struct buf_s buf) {
    int ret;
    struct sockaddr_nl nladdr;
    struct iovec iov = {
        .iov_base = buf.buf,
        .iov_len = buf.len,
    };
    struct msghdr msg = {
        .msg_name = &nladdr,
        .msg_namelen = sizeof(nladdr),
        .msg_iov = &iov,
        .msg_iovlen = 1,
    };

    ret = recvmsg(sock_fd, &msg, 0);
    printf("recvmsg: %d\n", ret);
    if (ret == -1) {
        return ret;
    }
    if (msg.msg_flags & MSG_TRUNC) {
        errno = ENOSPC;
        perror("recvmsg, msg_flags & MSG_TRUNC");
        return -errno;
    }

    return ret;
}

int recv_msg(int sock_fd, struct buf_s buf) {
    int len;
    struct nlmsghdr *nlh;

    while ((len = recv_resp_msg(sock_fd, buf)) > 0) {
        printf("recv_resp_msg len: %d\n", len);
        nlh = buf.buf;
        int i = 0;
        while (NLMSG_OK(nlh, len)) {
            printf("recv payload: \n");
            print_debug(nlh);
            // printf("----------%d-----------\n", ++i);
            if (nlh->nlmsg_type == NLMSG_DONE) {
                printf("NLMSG_DONE\n");
                goto ret;
            }

            // printf("nlh type: %d\n", nlh->nlmsg_type);
            struct rtmsg *rtm = (void *) nlh + MNL_ALIGN(sizeof(struct nlmsghdr));
            if (rtm->rtm_table != RT_TABLE_MAIN) {
                nlh = NLMSG_NEXT(nlh, len);
                continue;
            }

            // printf("rtm_family: %d\n", rtm->rtm_family);
            print_route(nlh);
            nlh = NLMSG_NEXT(nlh, len);
        }
    }

ret:
    return len;
}

int main(int argc, char const *argv[]) {
    int ret;
    int len;

    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ifname>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    const char *ifname = argv[1];

    // int sock_fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
    int sock_fd = socket(AF_NETLINK, SOCK_DGRAM | SOCK_CLOEXEC, NETLINK_ROUTE);
    if (sock_fd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_nl addr;
    addr.nl_family = AF_NETLINK;
    addr.nl_groups = 0;
    printf("pid: %d\n", getpid());
    addr.nl_pid = 0; // getpid();
    ret = bind(sock_fd, (struct sockaddr *)&addr, sizeof(addr));
    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
    printf("pid: %d\n", addr.nl_pid);

    unsigned int port_id = get_port_id(sock_fd);
    // rtnl_set_strict_dump(&rth);
    int one = 1;
    setsockopt(sock_fd, SOL_NETLINK, NETLINK_GET_STRICT_CHK, &one, sizeof(one));

    // make req msg
    char buffer[32768];
    struct buf_s buf = {
        .buf = buffer,
        .len = sizeof(buffer) / sizeof(char),
    };
    len = MNL_ALIGN(sizeof(struct nlmsghdr));
    struct nlmsghdr *nlh = buf.buf;
    memset((void *) nlh, 0, len);
    nlh->nlmsg_len = len;
    nlh->nlmsg_type = RTM_GETROUTE;
    nlh->nlmsg_flags = NLM_F_REQUEST | NLM_F_DUMP;
    nlh->nlmsg_seq = time(NULL);

    struct rtmsg *rtm = (void *) nlh + nlh->nlmsg_len;
    len = MNL_ALIGN(sizeof(struct rtmsg));
    memset((void *) rtm, 0, len);
    nlh->nlmsg_len += len;
    rtm->rtm_family = AF_INET;
    struct rtattr *rtattr = (void *) rtm + nlh->nlmsg_len;
    // rtattr->rta_type = RTA_TABLE;
    // rtattr->rta_len = RTA_LENGTH(sizeof(__u32));
    // // rtattr->rta_len = sizeof(struct rtattr) + sizeof(__u32);
    // __u32 table = RT_TABLE_MAIN;
    // memcpy(RTA_DATA(rtattr), &table, sizeof(__u32));
    // nlh->nlmsg_len += RTA_ALIGN(rtattr->rta_len);
    addattr32(nlh, 32768, RTA_TABLE, RT_TABLE_MAIN);
    __u32 ifindex = if_nametoindex(ifname);
    printf("ifindex: %d\n", ifindex);
    addattr32(nlh, 32768, RTA_OIF, ifindex);

    // send req msg
    send_req_msg(sock_fd, nlh);

    // recv resp msg
    ret = recv_msg(sock_fd, buf);
    if (ret == -1) {
        perror("recv_resp_msg");
        exit(EXIT_FAILURE);
    }
    printf("recv_resp_msg: %d\n", ret);

    close(sock_fd);

    return 0;
}



void print_debug(struct nlmsghdr *n) {
	printf("len: %d\n", n->nlmsg_len);
	char *x = (char *) n;
	int j = 0;
	for (int i = 0; i < n->nlmsg_len; i++) {
		printf("%02x ", x[i] & 0xff);
		j++;
		if (j == 16) {
			printf("\n");
			j = 0;
		}
	}
	printf("\n");
}