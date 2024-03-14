#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>    
#include <time.h>
#include <stdbool.h>
#include <net/if.h>
#include <linux/lwtunnel.h>
#include <linux/if.h>
#include <ctype.h>


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

#define NLMSG_TAIL(nmsg) \
	((struct rtattr *) (((void *) (nmsg)) + NLMSG_ALIGN((nmsg)->nlmsg_len)))



struct rtnl_handle rth = { .fd = -1 };
int rcvbuf = 1024 * 1024;

int addattr_l(struct nlmsghdr *n, int maxlen, int type, const void *data,
	      int alen)
{
	int len = RTA_LENGTH(alen);
	struct rtattr *rta;

	if (NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len) > maxlen) {
		fprintf(stderr,
			"addattr_l ERROR: message exceeded bound of %d\n",
			maxlen);
		return -1;
	}
	rta = NLMSG_TAIL(n);
	rta->rta_type = type;
	rta->rta_len = len;
	if (alen)
		memcpy(RTA_DATA(rta), data, alen);
	n->nlmsg_len = NLMSG_ALIGN(n->nlmsg_len) + RTA_ALIGN(len);
	return 0;
}


int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data)
{
	return addattr_l(n, maxlen, type, &data, sizeof(__u32));
}




void rtnl_close(struct rtnl_handle *rth)
{
	if (rth->fd >= 0) {
		close(rth->fd);
		rth->fd = -1;
	}
}

int rtnl_open_byproto(struct rtnl_handle *rth, unsigned int subscriptions,
		      int protocol)
{
	socklen_t addr_len;
	int sndbuf = 32768;
	int one = 1;

	memset(rth, 0, sizeof(*rth));

	rth->proto = protocol;
	rth->fd = socket(AF_NETLINK, SOCK_RAW | SOCK_CLOEXEC, protocol);
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



int rtnl_open(struct rtnl_handle *rth, unsigned int subscriptions)
{
	return rtnl_open_byproto(rth, subscriptions, NETLINK_ROUTE);
}


typedef struct
{
	__u16 flags;
	__u16 bytelen;
	__s16 bitlen;
	/* These next two fields match rtvia */
	__u16 family;
	__u32 data[64];
} inet_prefix;


static struct
{
	unsigned int tb;
	int cloned;
	int flushed;
	char *flushb;
	int flushp;
	int flushe;
	int protocol, protocolmask;
	int scope, scopemask;
	__u64 typemask;
	int tos, tosmask;
	int iif, iifmask;
	int oif, oifmask;
	int mark, markmask;
	int realm, realmmask;
	__u32 metric, metricmask;
	inet_prefix rprefsrc;
	inet_prefix rvia;
	inet_prefix rdst;
	inet_prefix mdst;
	inet_prefix rsrc;
	inet_prefix msrc;
} filter;


static int iproute_dump_filter(struct nlmsghdr *nlh, int reqlen)
{
	struct rtmsg *rtm = NLMSG_DATA(nlh);
	int err;

	rtm->rtm_protocol = filter.protocol;
	printf("filter protocol: %d\n", filter.protocol);
	if (filter.cloned) {
		printf("filter cloned: %d\n", filter.cloned);
		rtm->rtm_flags |= RTM_F_CLONED;
	}

	if (filter.tb) {
		printf("filter tb: %d\n", filter.tb);
		err = addattr32(nlh, reqlen, RTA_TABLE, filter.tb);
		if (err)
			return err;
	}

	if (filter.oif) {
		printf("filter oif: %d\n", filter.oif);
		err = addattr32(nlh, reqlen, RTA_OIF, filter.oif);
		if (err)
			return err;
	}

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


int rtnl_routedump_req(struct rtnl_handle *rth, int family)
{
	struct {
		struct nlmsghdr nlh;
		struct rtmsg rtm;
		char buf[128];
	} req = {
		.nlh.nlmsg_len = NLMSG_LENGTH(sizeof(struct rtmsg)),
		.nlh.nlmsg_type = RTM_GETROUTE,
		.nlh.nlmsg_flags = NLM_F_DUMP | NLM_F_REQUEST,
		.nlh.nlmsg_seq = rth->dump = ++rth->seq,
		.rtm.rtm_family = family,
	};

    int err = iproute_dump_filter(&req.nlh, sizeof(req));
    if (err)
        return err;

	printf("req payload\n");
	print_debug(&req.nlh);
	return send(rth->fd, &req, sizeof(req), 0);
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
	printf("recv len: %d\n", len);
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

int af_bit_len(int af)
{
	switch (af) {
	case AF_INET6:
		return 128;
	case AF_INET:
		return 32;
	case AF_MPLS:
		return 20;
	}

	return 0;
}

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


int parse_rtattr(struct rtattr *tb[], int max, struct rtattr *rta, int len)
{
	return parse_rtattr_flags(tb, max, rta, len, 0);
}

static inline __u32 rta_getattr_u32(const struct rtattr *rta)
{
	return *(__u32 *)RTA_DATA(rta);
}

static inline int rtm_get_table(struct rtmsg *r, struct rtattr **tb)
{
	__u32 table = r->rtm_table;

	if (tb[RTA_TABLE])
		table = rta_getattr_u32(tb[RTA_TABLE]);
	return table;
}

enum {
	PREFIXLEN_SPECIFIED	= (1 << 0),
	ADDRTYPE_INET		= (1 << 1),
	ADDRTYPE_UNSPEC		= (1 << 2),
	ADDRTYPE_MULTI		= (1 << 3),

	ADDRTYPE_INET_UNSPEC	= ADDRTYPE_INET | ADDRTYPE_UNSPEC,
	ADDRTYPE_INET_MULTI	= ADDRTYPE_INET | ADDRTYPE_MULTI
};


int get_real_family(int rtm_type, int rtm_family)
{
	if (rtm_type != RTN_MULTICAST)
		return rtm_family;

	if (rtm_family == RTNL_FAMILY_IPMR)
		return AF_INET;

	if (rtm_family == RTNL_FAMILY_IP6MR)
		return AF_INET6;

	return rtm_family;
}


#define SPRINT_BSIZE 64
#define SPRINT_BUF(x)	char x[SPRINT_BSIZE]

int print_route(struct nlmsghdr *n)
{
	FILE *fp = (FILE *)stdout;
	struct rtmsg *r = NLMSG_DATA(n);
	int len = n->nlmsg_len;
	printf("response length: %d\n", len);
	struct rtattr *tb[RTA_MAX+1];
	int family, color, host_len;
	__u32 table;
	int ret;

	SPRINT_BUF(b1);
	printf("type: %d\n", n->nlmsg_type);
	if (n->nlmsg_type != RTM_NEWROUTE && n->nlmsg_type != RTM_DELROUTE) {
		fprintf(stderr, "Not a route: %08x %08x %08x\n",
			n->nlmsg_len, n->nlmsg_type, n->nlmsg_flags);
		return -1;
	}
	if (filter.flushb && n->nlmsg_type != RTM_NEWROUTE)
		return 0;
	len -= NLMSG_LENGTH(sizeof(*r));
	if (len < 0) {
		fprintf(stderr, "BUG: wrong nlmsg len %d\n", len);
		return -1;
	}

	host_len = af_bit_len(r->rtm_family);

	parse_rtattr(tb, RTA_MAX, RTM_RTA(r), len);
	table = rtm_get_table(r, tb);
	if (tb[RTA_ENCAP]) {
		printf("RTA_ENCAP yes\n");
	} else {
		printf("RTA_ENCAP no\n");
	}

/*
	if (!filter_nlmsg(n, tb, host_len))
		return 0;

	if (tb[RTA_DST]) {
		printf("RTA_DST\n");
		family = get_real_family(r->rtm_type, r->rtm_family);
		// color = ifa_family_color(family);

		if (r->rtm_dst_len != host_len) {
			snprintf(b1, sizeof(b1),
				 "%s/%u", rt_addr_n2a_rta(family, tb[RTA_DST]),
				 r->rtm_dst_len);
		} else {
			const char *hostname = format_host_rta_r(family, tb[RTA_DST],
					  b1, sizeof(b1));
			if (hostname)
				strncpy(b1, hostname, sizeof(b1) - 1);
		}
	} else if (r->rtm_dst_len) {
		printf("rtm_dst_len\n");
		snprintf(b1, sizeof(b1), "0/%d ", r->rtm_dst_len);
	} else {
		printf("default\n");
		strncpy(b1, "default", sizeof(b1));
	}
	print_color_string(PRINT_ANY, color,
			   "dst", "%s ", b1);

	if (tb[RTA_SRC]) {
		family = get_real_family(r->rtm_type, r->rtm_family);
		color = ifa_family_color(family);

		if (r->rtm_src_len != host_len) {
			snprintf(b1, sizeof(b1),
				 "%s/%u",
				 rt_addr_n2a_rta(family, tb[RTA_SRC]),
				 r->rtm_src_len);
		} else {
			const char *hostname = format_host_rta_r(family, tb[RTA_SRC],
					  b1, sizeof(b1));
			if (hostname)
				strncpy(b1, hostname, sizeof(b1) - 1);
		}
		print_color_string(PRINT_ANY, color,
				   "from", "from %s ", b1);
	} else if (r->rtm_src_len) {
		snprintf(b1, sizeof(b1), "0/%u", r->rtm_src_len);

		print_string(PRINT_ANY, "src", "from %s ", b1);
	}

	if (tb[RTA_NH_ID])
		print_uint(PRINT_ANY, "nhid", "nhid %u ",
			   rta_getattr_u32(tb[RTA_NH_ID]));

	if (tb[RTA_NEWDST])
		print_rta_newdst(fp, r, tb[RTA_NEWDST]);

	if (tb[RTA_ENCAP])
		lwt_print_encap(fp, tb[RTA_ENCAP_TYPE], tb[RTA_ENCAP]);

	if (r->rtm_tos && filter.tosmask != -1) {
		print_string(PRINT_ANY, "tos", "tos %s ",
			     rtnl_dsfield_n2a(r->rtm_tos, b1, sizeof(b1)));
	}

	if (tb[RTA_GATEWAY] && filter.rvia.bitlen != host_len)
		print_rta_gateway(fp, r->rtm_family, tb[RTA_GATEWAY]);

	if (tb[RTA_VIA])
		print_rta_via(fp, tb[RTA_VIA]);

	if (tb[RTA_OIF] && filter.oifmask != -1)
		print_rta_ifidx(fp, rta_getattr_u32(tb[RTA_OIF]), "dev");

	if (table && (table != RT_TABLE_MAIN || show_details > 0) && !filter.tb)
		print_string(PRINT_ANY,
			     "table", "table %s ",
			     rtnl_rttable_n2a(table, b1, sizeof(b1)));

	if (!(r->rtm_flags & RTM_F_CLONED)) {
		if ((r->rtm_protocol != RTPROT_BOOT || show_details > 0) &&
		    filter.protocolmask != -1)
			print_string(PRINT_ANY,
				     "protocol", "proto %s ",
				     rtnl_rtprot_n2a(r->rtm_protocol,
						     b1, sizeof(b1)));

		if ((r->rtm_scope != RT_SCOPE_UNIVERSE || show_details > 0) &&
		    filter.scopemask != -1)
			print_string(PRINT_ANY,
				     "scope", "scope %s ",
				     rtnl_rtscope_n2a(r->rtm_scope,
						      b1, sizeof(b1)));
	}

	if (tb[RTA_PREFSRC] && filter.rprefsrc.bitlen != host_len) {
		const char *psrc
			= rt_addr_n2a_rta(r->rtm_family, tb[RTA_PREFSRC]);

		// Do not use format_host(). It is our local addr
		// and symbolic name will not be useful.
		
		if (is_json_context())
			print_string(PRINT_JSON, "prefsrc", NULL, psrc);
		else {
			fprintf(fp, "src ");
			print_color_string(PRINT_FP,
					   ifa_family_color(r->rtm_family),
					   NULL, "%s ", psrc);
		}

	}

	if (tb[RTA_PRIORITY] && filter.metricmask != -1)
		print_uint(PRINT_ANY, "metric", "metric %u ",
			   rta_getattr_u32(tb[RTA_PRIORITY]));

	print_rt_flags(fp, r->rtm_flags);

	if (tb[RTA_MARK]) {
		unsigned int mark = rta_getattr_u32(tb[RTA_MARK]);

		if (mark) {
			if (is_json_context())
				print_uint(PRINT_JSON, "mark", NULL, mark);
			else if (mark >= 16)
				print_0xhex(PRINT_FP, NULL,
					    "mark 0x%llx ", mark);
			else
				print_uint(PRINT_FP, NULL,
					   "mark %u ", mark);
		}
	}

	if (tb[RTA_FLOW] && filter.realmmask != ~0U)
		print_rta_flow(fp, tb[RTA_FLOW]);

	if (tb[RTA_UID])
		print_uint(PRINT_ANY, "uid", "uid %u ",
			   rta_getattr_u32(tb[RTA_UID]));

	if (r->rtm_family == AF_INET) {
		if (r->rtm_flags & RTM_F_CLONED)
			print_cache_flags(fp, r->rtm_flags);

		if (tb[RTA_CACHEINFO])
			print_rta_cacheinfo(fp, RTA_DATA(tb[RTA_CACHEINFO]));
	} else if (r->rtm_family == AF_INET6) {
		if (tb[RTA_CACHEINFO])
			print_rta_cacheinfo(fp, RTA_DATA(tb[RTA_CACHEINFO]));
	}

	if (tb[RTA_METRICS])
		print_rta_metrics(fp, tb[RTA_METRICS]);

	if (tb[RTA_IIF] && filter.iifmask != -1)
		print_rta_ifidx(fp, rta_getattr_u32(tb[RTA_IIF]), "iif");

	if (tb[RTA_PREF])
		print_rt_pref(fp, rta_getattr_u8(tb[RTA_PREF]));

	if (tb[RTA_TTL_PROPAGATE]) {
		bool propagate = rta_getattr_u8(tb[RTA_TTL_PROPAGATE]);

		if (is_json_context())
			print_bool(PRINT_JSON, "ttl-propogate", NULL,
				   propagate);
		else
			print_string(PRINT_FP, NULL,
				     "ttl-propogate %s",
				     propagate ? "enabled" : "disabled");
	}

	if (tb[RTA_NH_ID] && show_details)
		print_cache_nexthop_id(fp, "\n\tnh_info ", "nh_info",
				       rta_getattr_u32(tb[RTA_NH_ID]));

	if (tb[RTA_MULTIPATH])
		print_rta_multipath(fp, r, tb[RTA_MULTIPATH]);

	// If you are adding new route RTA_XXXX then place it above
	// the RTA_MULTIPATH else it will appear that the last nexthop
	// in the ECMP has new attributes
	

	print_string(PRINT_FP, NULL, "\n", NULL);
	close_json_object();
	fflush(fp);
    */
	return 0;
}


static int rtnl_dump_filter_l(struct rtnl_handle *rth)
{
	struct sockaddr_nl nladdr;
	struct iovec iov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = &iov,
		.msg_iovlen = 1,
	};
	char *buf;
	int dump_intr = 0;

	while (1) {
		int status;
		// const struct rtnl_dump_filter_arg *a;
		int found_done = 0;
		int msglen = 0;

		status = rtnl_recvmsg(rth->fd, &msg, &buf);
		if (status < 0)
			return status;

		if (rth->dump_fp) {
			printf("rth dump fp---\n");
			fwrite(buf, 1, NLMSG_ALIGN(status), rth->dump_fp);
		}

        struct nlmsghdr *h = (struct nlmsghdr *)buf;

        msglen = status;

        while (NLMSG_OK(h, msglen)) {
			print_debug(h);
            int err = 0;

            // h->nlmsg_flags &= ~a->nc_flags;

            if (nladdr.nl_pid != 0 ||
                h->nlmsg_pid != rth->local.nl_pid ||
                h->nlmsg_seq != rth->dump)
                return -1;

            if (h->nlmsg_flags & NLM_F_DUMP_INTR)
                dump_intr = 1;

            if (h->nlmsg_type == NLMSG_DONE) {
                printf("NLMSG_DONE\n");
                // err = rtnl_dump_done(h, a);
                // if (err < 0) {
                // 	free(buf);
                // 	return -1;
                // }

                found_done = 1;
                break; /* process next filter */
            }


            err = print_route(h);
            if (err < 0) {
                free(buf);
                return err;
            }
            h = NLMSG_NEXT(h, msglen);
		}
		free(buf);

		if (found_done) {
			if (dump_intr)
				fprintf(stderr,
					"Dump was interrupted and may be inconsistent.\n");
			return 0;
		}

		if (msg.msg_flags & MSG_TRUNC) {
			fprintf(stderr, "Message truncated\n");
			continue;
		}
		if (msglen) {
			fprintf(stderr, "!!!Remnant of size %d\n", msglen);
			exit(1);
		}
	}
}


// #define SPRINT_BSIZE 64
// #define SPRINT_BUF(x)	char x[SPRINT_BSIZE]

static int __check_ifname(const char *name)
{
	if (*name == '\0')
		return -1;
	while (*name) {
		if (*name == '/' || isspace(*name))
			return -1;
		++name;
	}
	return 0;
}

int check_ifname(const char *name)
{
	/* These checks mimic kernel checks in dev_valid_name */
	if (strlen(name) >= IFNAMSIZ)
		return -1;
	return __check_ifname(name);
}
static int __rtnl_talk_iov(struct rtnl_handle *rtnl, struct iovec *iov,
			   size_t iovlen, struct nlmsghdr **answer)
{
	struct sockaddr_nl nladdr = { .nl_family = AF_NETLINK };
	struct iovec riov;
	struct msghdr msg = {
		.msg_name = &nladdr,
		.msg_namelen = sizeof(nladdr),
		.msg_iov = iov,
		.msg_iovlen = iovlen,
	};
	unsigned int seq = 0;
	struct nlmsghdr *h;
	int i, status;
	char *buf;

	for (i = 0; i < iovlen; i++) {
		h = iov[i].iov_base;
		h->nlmsg_seq = seq = ++rtnl->seq;
		if (answer == NULL)
			h->nlmsg_flags |= NLM_F_ACK;
	}

	status = sendmsg(rtnl->fd, &msg, 0);
	if (status < 0) {
		perror("Cannot talk to rtnetlink");
		return -1;
	}
	printf("talk to rtnetlink\n");

	/* change msg to use the response iov */
	msg.msg_iov = &riov;
	msg.msg_iovlen = 1;
	i = 0;
	while (1) {
next:
		status = rtnl_recvmsg(rtnl->fd, &msg, &buf);
		++i;

		if (status < 0)
			return status;

		if (msg.msg_namelen != sizeof(nladdr)) {
			fprintf(stderr,
				"sender address length == %d\n",
				msg.msg_namelen);
			exit(1);
		}
		for (h = (struct nlmsghdr *)buf; status >= sizeof(*h); ) {
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
			    h->nlmsg_seq > seq || h->nlmsg_seq < seq - iovlen) {
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

				if (!error) {
					/* check messages from kernel */
					// nl_dump_ext_ack(h, errfn);
				} else {
					errno = -error;

					// if (rtnl->proto != NETLINK_SOCK_DIAG &&
					//     show_rtnl_err)
					// 	rtnl_talk_error(h, err, errfn);
				}

				if (i < iovlen) {
					free(buf);
					goto next;
				}

				if (error) {
					free(buf);
					return -i;
				}

				if (answer)
					*answer = (struct nlmsghdr *)buf;
				return 0;
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

		if (msg.msg_flags & MSG_TRUNC) {
			fprintf(stderr, "Message truncated\n");
			continue;
		}

		if (status) {
			fprintf(stderr, "!!!Remnant of size %d\n", status);
			exit(1);
		}
	}
}

static int __rtnl_talk(struct rtnl_handle *rtnl, struct nlmsghdr *n,
		       struct nlmsghdr **answer)
{
	struct iovec iov = {
		.iov_base = n,
		.iov_len = n->nlmsg_len
	};

	printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
	return __rtnl_talk_iov(rtnl, &iov, 1, answer);
}

int rtnl_talk_suppress_rtnl_errmsg(struct rtnl_handle *rtnl, struct nlmsghdr *n,
				   struct nlmsghdr **answer)
{
	return __rtnl_talk(rtnl, n, answer);
}

static int ll_link_get(const char *name, int index)
{
	struct {
		struct nlmsghdr		n;
		struct ifinfomsg	ifm;
		char			buf[1024];
	} req = {
		.n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
		.n.nlmsg_flags = NLM_F_REQUEST,
		.n.nlmsg_type = RTM_GETLINK,
		.ifm.ifi_index = index,
	};
	__u32 filt_mask = RTEXT_FILTER_VF | RTEXT_FILTER_SKIP_STATS;
	struct rtnl_handle rth = {};
	struct nlmsghdr *answer;
	int rc = 0;

	if (rtnl_open(&rth, 0) < 0)
		return 0;

	addattr32(&req.n, sizeof(req), IFLA_EXT_MASK, filt_mask);
	if (name)
		addattr_l(&req.n, sizeof(req),
			  !check_ifname(name) ? IFLA_IFNAME : IFLA_ALT_IFNAME,
			  name, strlen(name) + 1);

	printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
	if (rtnl_talk_suppress_rtnl_errmsg(&rth, &req.n, &answer) < 0)
		goto out;

	/* add entry to cache */
	// rc  = ll_remember_index(answer, NULL);
	// if (!rc) {
	// 	struct ifinfomsg *ifm = NLMSG_DATA(answer);

	// 	rc = ifm->ifi_index;
	// }

	free(answer);
out:
	rtnl_close(&rth);
	return rc;
}

unsigned ll_name_to_index(const char *name)
{
	// const struct ll_cache *im;
	unsigned idx;

	if (name == NULL)
		return 0;

	// im = ll_get_by_name(name);
	// if (im)
	// 	return im->index;

	// printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
	idx = ll_link_get(name, 0);
	if (idx == 0)
		idx = if_nametoindex(name);
	// if (idx == 0)
	// 	idx = ll_idx_a2n(name);
	return idx;
}

int main(int argc, char const *argv[])
{
    const char *ifname = argv[1];

    if (rtnl_open(&rth, 0) < 0)
		exit(1);

    // int idx = if_nametoindex(ifname);
    int idx = ll_name_to_index(ifname);
	filter.tb = RT_TABLE_MAIN;
    filter.oif = idx;
    filter.oifmask = -1;

	if (rtnl_routedump_req(&rth, AF_INET) < 0) {
		perror("Cannot send dump request");
		return -2;
	}

    rtnl_dump_filter_l(&rth);

    return 0;
}
