#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <fnmatch.h>

#include <linux/netdevice.h>
#include <linux/if_arp.h>
#include <linux/if_infiniband.h>
#include <linux/sockios.h>
#include <linux/net_namespace.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>

#include "ip_common.h"
#include "libnetlink.h"

struct rtnl_handle rth = { .fd = -1 };


static int store_nlmsg(struct nlmsghdr *n, void *arg)
{
    struct nlmsg_chain *lchain = (struct nlmsg_chain *) arg;
    struct nlmsg_list *h;

    h = malloc(n->nlmsg_len + sizeof(void *));
    if (h == NULL)
        return -1;
    
    memcpy(&h->h, n, n->nlmsg_len);
    h->next = NULL;

    if (lchain->tail)
        lchain->tail->next = h;
    else 
        lchain->head = h;
    lchain->tail = h;

    ll_remember_index(n, NULL);
}


static int ipaddr_link_get(int index, struct nlmsg_chain *linfo)
{
    struct iplink_req req = {
        .n.nlmsg_len = NLMSG_LENGTH(sizeof(struct ifinfomsg)),
        .n.nlmsg_flags = NLM_F_REQUEST,
        .n.nlmsg_type = RTM_GETLINK,
        .i.ifi_family = AF_UNSPEC,
        .i.ifi_index = index,
    };

    __u32 filt_mask = RTEXT_FILTER_VF;
    struct nlmsghdr *answer;

    addattr32(&req.n, sizeof(req), IFLA_EXT_MASK, filt_mask);

    if (rtnl_talk(&rth, &req.n, &answer) < 0) {
        perror("Cannot send link request");
        return 1;
    }

    if (store_nlmsg(answer, linfo) < 0) {
        fprintf(stderr, "Failed to process link information\n");
        free(answer);
        return 1;
    }
    free(answer);

    return 0;
}
