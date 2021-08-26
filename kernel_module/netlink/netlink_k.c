#include <linux/init.h>
#include <linux/module.h>
#include <linux/types.h>
#include <net/sock.h>
#include <linux/netlink.h>


#define NETLINK_PRIVATE 30
#define USER_PORT		100

MODULE_LICENSE("GPL");
MODULE_AUTHOR("dawson");
MODULE_DESCRIPTION("netlink example");


struct sock *nlsk = NULL;
extern struct net init_net;

int send_usermsg(char *msg, uint16_t len) {
	struct sk_buff *nl_skb;
	struct nlmsghdr *nlh;

	/* 创建sk_buff 空间 */
	nl_skb = nlmsg_new(len, GFP_ATOMIC);
	if (!nl_skb) {
		printk("netlink alloc failure\n");
		return -1;
	}

	// nlmsg_put 设置 netlink 消息头部
	nlh = nlmsg_put(nl_skb, 0, 0, NETLINK_PRIVATE, len, 0);
	if (nlh == NULL) {
		printk("nlmsg_put failaure\n");
		nlmsg_free(nl_skb);
		return -1;
	}

	// void *nlmsg_data(const struct nlmsghdr *)
	// (unsigned char *) nlh + NLMSG_HDRLEN;
	memcpy(nlmsg_data(nlh), msg, len);
	return netlink_unicast(nlsk, nl_skb, USER_PORT, MSG_DONTWAIT);
}

static void netlink_rcv_msg(struct sk_buff *skb) {
	struct nlmsghdr *nlh = NULL;
	char *umsg = NULL;
	char *kmsg = "hello, users";

	if (skb->len >= nlmsg_total_size(0)) {
		nlh = nlmsg_hdr(skb);	// (struct nlmsghdr *)skb->data;
		umsg = NLMSG_DATA(nlh);
		if (umsg) {
			printk("kernel recv from user: %s\n", umsg);
			send_usermsg(kmsg, strlen(kmsg));
		}
	}
}

// 处理函数注册
struct netlink_kernel_cfg cfg = {
	.input = netlink_rcv_msg,
};

int test_netlink_init(void) {
	/* create netlink socket */
	nlsk = (struct sock *) netlink_kernel_create(&init_net, NETLINK_PRIVATE, &cfg);
	if (nlsk == NULL) {
		printk("netlink_kernal_create error\n");
		return -1;
	}

	printk("test_netlink_init\n");
	return 0;
}

void test_netlink_exit(void) {
	if (nlsk) {
		netlink_kernel_release(nlsk);
		nlsk = NULL;
	}

	printk("test_netlink exit\n");
}

module_init(test_netlink_init);
module_exit(test_netlink_exit);
