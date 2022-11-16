#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <linux/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include <net/if.h>
#include <errno.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <time.h>

struct rtnl_handle {
	int			fd;
	struct sockaddr_nl	local;
	struct sockaddr_nl	peer;
	__u32			seq;
	__u32			dump;
	int			proto;
	FILE		       *dump_fp;
#define RTNL_HANDLE_F_LISTEN_ALL_NSID		0x01
#define RTNL_HANDLE_F_SUPPRESS_NLERR		0x02
#define RTNL_HANDLE_F_STRICT_CHK		0x04
	int			flags;
};




typedef struct
{
	__u16 flags;
	__u16 bytelen;
	__s16 bitlen;
	/* These next two fields match rtvia */
	__u16 family;
	__u32 data[64];
} inet_prefix;


struct link_filter {
	int ifindex;
	int family;
	int oneline;
	int showqueue;
	inet_prefix pfx;
	int scope, scopemask;
	int flags, flagmask;
	int up;
	char *label;
	int flushed;
	char *flushb;
	int flushp;
	int flushe;
	int group;
	int master;
	char *kind;
	char *slave_kind;
	int target_nsid;
};

// static struct link_filter filter;

int parse_rtattr_flags(struct rtattr *tb[], int max, struct rtattr *rta,
		       int len, unsigned short flags)
{
	unsigned short type;

	memset(tb, 0, sizeof(struct rtattr *) * (max + 1));
	while (RTA_OK(rta, len)) {
		type = rta->rta_type & ~flags;
		if ((type <= max) && (!tb[type]))
			tb[type] = rta;
		rta = RTA_NEXT(rta, len);
	}
	if (len)
		fprintf(stderr, "!!!Deficit %d, rta_len=%d\n",
			len, rta->rta_len);
	return 0;
}

int print_linkinfo(struct nlmsghdr *n, void *arg)
{
	FILE *fp = (FILE *)arg;
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct rtattr *tb[IFLA_MAX+1];
	int len = n->nlmsg_len;
	const char *name;
	unsigned int m_flag = 0;
	char b1[64];
	bool truncated_vfs = false;

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	// len 是减去了 nlmsghdr 和 ifinfo msg 的总长度
	len -= NLMSG_LENGTH(sizeof(*ifi));
	if (len < 0)
		return -1;

	// 								 rtattr 的指针
	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(ifi), len, NLA_F_NESTED);

	// printf("    link/%s ", ll_type_n2a(ifi->ifi_type, b1, sizeof(b1)));
	// if (tb[IFLA_ADDRESS]) {
	// 	fprintf(stdout, "%s", inet_ntop(AF_INET, RTA_DATA(tb[IFLA_ADDRESS]), b1, sizeof(b1)));
	// }

	if (tb[IFLA_BROADCAST] && (ifi->ifi_flags&IFF_POINTOPOINT)) {
		// if (ifi->ifi_flags&IFF_POINTOPOINT) {
		// 	printf(" peer ");
		// }
		fprintf(stdout, "%s\n", inet_ntop(AF_INET, RTA_DATA(tb[IFLA_BROADCAST]), b1, sizeof(b1)));
	}

	fflush(fp);
	return 1;
}

#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data, int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
		fprintf(stderr,
			"addattr_l ERROR: message exceeded bound of %d\n",
			maxlen);
		return -1;
	}

	rta = NLMSG_TAIL(n);	// rta 指向了 buf
	rta->rta_type = type;
	rta->rta_len = len;
	if (alen)
		memcpy(RTA_DATA(rta), data, alen);

	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
	return 0;
}

static int __rtnl_recvmsg(int fd, struct msghdr *msg, int flags)
{
	int len;

	do {
		len = recvmsg(fd, msg, flags);
	} while (len < 0 && (errno == EINTR || errno == EAGAIN));

	if (len < 0) {
		fprintf(stderr, "netlink receive error %s (%d)\n",
			strerror(errno), errno);
		return -errno;
	}

	if (len == 0) {
		fprintf(stderr, "EOF on netlink\n");
		return -ENODATA;
	}

	return len;
}

static int rtnl_recvmsg(int fd, struct msghdr *msg, char **answer)
{
	struct iovec *iov = msg->msg_iov;
	char *buf;
	int len;

	iov->iov_base = NULL;
	iov->iov_len = 0;

	len = __rtnl_recvmsg(fd, msg, MSG_PEEK | MSG_TRUNC);
	if (len < 0)
		return len;

	if (len < 32768)
		len = 32768;
	buf = malloc(len);
	if (!buf) {
		fprintf(stderr, "malloc error: not enough buffer\n");
		return -ENOMEM;
	}

	iov->iov_base = buf;
	iov->iov_len = len;

	len = __rtnl_recvmsg(fd, msg, 0);
	if (len < 0) {
		free(buf);
		return len;
	}

	if (answer)
		*answer = buf;
	else
		free(buf);

	return len;
}

static int rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n, struct nlmsghdr **answer)
{
	struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
	struct iovec iov = {
		.iov_base = n,
		.iov_len = n->nlmsg_len
	};
	// size_t iovlen = 1;

	struct iovec riov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	unsigned int seq = 0;
	int i, status;
	char *buf;

	n->nlmsg_seq = seq = ++rtnl->seq;
	if (answer == NULL)
		n->nlmsg_flags |= NLM_F_ACK;

	status = sendmsg(rtnl->fd, &msg, 0);
	if (status < 0) {
		perror("Cannot talk to rtnetlink");
		return -1;
	}

	/* change msg to use the response iov */
	msg.msg_iov = &riov;
	msg.msg_iovlen = 1;

	// for (i = 0; i < iovlen; ++i) {
	status = rtnl_recvmsg(rtnl->fd, &msg, &buf);
	if (status < 0)
		return status;

	if (msg.msg_namelen != sizeof(nladdr)) {
		fprintf(stderr,
			"sender address length == %d\n",
			msg.msg_namelen);
		exit(1);
	}

	for (struct nlmsghdr *h = (struct nlmsghdr *)buf; status >= sizeof(*h); ) {
		int len = h->nlmsg_len;
		int l = len - sizeof(*h);

		if (l < 0 || len > status) {
			if (msg.msg_flags & MSG_TRUNC) {
				fprintf(stderr, "Truncated message\n");
				free(buf);
				return -1;
			}
			fprintf(stderr,
				"!!!malformed message: len=%d\n",
				len);
			exit(1);
		}

		if (nladdr.nl_pid != 0 ||
			h->nlmsg_pid != rtnl->local.nl_pid ||
			h->nlmsg_seq > seq || h->nlmsg_seq < seq - 1 /*iovlen*/) {
			/* Don't forget to skip that message. */
			status -= NLMSG_ALIGN(len);
			h = (struct nlmsghdr *)((char *)h + NLMSG_ALIGN(len));
			continue;
		}

		if (h->nlmsg_type == NLMSG_ERROR) {
			struct nlmsgerr *err = (struct nlmsgerr *)NLMSG_DATA(h);
			int error = err->error;

			if (l < sizeof(struct nlmsgerr)) {
				fprintf(stderr, "ERROR truncated\n");
				free(buf);
				return -1;
			}

			if (answer)
				*answer = (struct nlmsghdr *)buf;
			else
				free(buf);

			return error ? -i : 0;
		}

		if (answer) {
			*answer = (struct nlmsghdr *)buf;
			return 0;
		}

		fprintf(stderr, "Unexpected reply!!!\n");

		status -= NLMSG_ALIGN(len);
		h = (struct nlmsghdr *)((char *)h + NLMSG_ALIGN(len));
	}
	free(buf);

	// if (msg.msg_flags & MSG_TRUNC) {
	// 	fprintf(stderr, "Message truncated\n");
	// 	continue;
	// }

	if (status) {
		fprintf(stderr, "!!!Remnant of size %d\n", status);
		exit(1);
	}
	// }
}



struct iplink_req {
	struct nlmsghdr		n;
	struct ifinfomsg	i;
	char			buf[1024];
};

int iplink_get(const char *name, __u32 filt_mask, struct rtnl_handle *rth)
{
	struct iplink_req req = {
		{
			.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
			.nlmsg_type = RTM_GETLINK,
			.nlmsg_flags = NLM_F_REQUEST,
		},
		{ 
			.ifi_family = AF_UNSPEC,
			.ifi_index = 
		},
	};

	addattr_l(&req.n, sizeof(req), IFLA_IFNAME, name, strlen(name) + 1);
	// addattr_l(&req.n, sizeof(req), IFLA_EXT_MASK, &filt_mask, sizeof(__u32));
	// 经过这两步, 有两个 rtattr 放在了 buf 中

	struct nlmsghdr *answer;

	if (rtnl_talk(rth, &req.n, &answer) < 0)
		return -2;

	print_linkinfo(answer, stdout);

	free(answer);
	return 0;
}

int rcvbuf = 1024 * 1024;

void rtnl_close(struct rtnl_handle *rth)
{
	if (rth->fd >= 0) {
		close(rth->fd);
		rth->fd = -1;
	}
}

int rtnl_open(struct rtnl_handle *rth, unsigned int subscriptions)
{
	socklen_t addr_len;
	int sndbuf = 32768;
	int one = 1;

	memset(rth, 0, sizeof(*rth));

	rth->proto = NETLINK_ROUTE;
	rth->fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, NETLINK_ROUTE);
	if (rth->fd < 0) {
		perror("Cannot open netlink socket");
		return -1;
	}

	if (setsockopt(rth->fd, SOL_SOCKET, SO_SNDBUF,
		       &sndbuf, sizeof(sndbuf)) < 0) {
		perror("SO_SNDBUF");
		goto err;
	}

	if (setsockopt(rth->fd, SOL_SOCKET, SO_RCVBUF,
		       &rcvbuf, sizeof(rcvbuf)) < 0) {
		perror("SO_RCVBUF");
		goto err;
	}

	/* Older kernels may no support extended ACK reporting */
	setsockopt(rth->fd, SOL_NETLINK, NETLINK_EXT_ACK,
		   &one, sizeof(one));

	memset(&rth->local, 0, sizeof(rth->local));
	rth->local.nl_family = AF_NETLINK;
	rth->local.nl_groups = subscriptions;

	if (bind(rth->fd, (struct sockaddr *)&rth->local,
		 sizeof(rth->local)) < 0) {
		perror("Cannot bind netlink socket");
		goto err;
	}
	addr_len = sizeof(rth->local);
	if (getsockname(rth->fd, (struct sockaddr *)&rth->local,
			&addr_len) < 0) {
		perror("Cannot getsockname");
		goto err;
	}
	if (addr_len != sizeof(rth->local)) {
		fprintf(stderr, "Wrong address length %d\n", addr_len);
		goto err;
	}
	if (rth->local.nl_family != AF_NETLINK) {
		fprintf(stderr, "Wrong address family %d\n",
			rth->local.nl_family);
		goto err;
	}
	rth->seq = time(NULL);
	return 0;
err:
	rtnl_close(rth);
	return -1;
}

int main(int argc, char const *argv[])
{
	const char *filter_dev = argv[1];

	struct rtnl_handle rth = { .fd = -1 };

	if (rtnl_open(&rth, 0) < 0)
		exit(1);

	iplink_get(filter_dev, RTEXT_FILTER_VF, &rth);

	rtnl_close(&rth);
}
