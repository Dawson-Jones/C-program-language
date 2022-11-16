#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <string.h>
#include <linux/netlink.h>
#include <stdint.h>
#include <unistd.h>
#include <errno.h>

#define NETLINK_PRIVATE 30
#define MSG_LEN			125
#define MAX_PAYLOAD		125

struct user_msg_info {
	struct nlmsghdr hdr;
	char msg[MSG_LEN];
};


void send_msg(int nlfd, struct nlmsghdr *nlh, struct sockaddr_nl *daddr_p) {
	int ret;
	char *msg;

	// Send N bytes of BUF on socket FD to peer at address ADDR (which is ADDR_LEN bytes long)
	// Returns the number sent, or -1 for errors.
	ret = sendto(nlfd, nlh, nlh->nlmsg_len, 0, (struct sockaddr *) daddr_p, sizeof(struct sockaddr_nl));
	if (!ret) {
		perror("sendto error\n");
		close(nlfd);
		exit(-1);
	}

	printf("send kernel %s\n", (char *) NLMSG_DATA(nlh));
}

void recv_msg(int nlfd, struct sockaddr_nl *daddr_p) {
	int len, ret;
	struct user_msg_info u_info = {};

	// Read N bytes into BUF through socket FD.
   	// If ADDR is not NULL, fill in *ADDR_LEN bytes of it with tha address of
   	// the sender, and store the actual size of the address in *ADDR_LEN.
   	// Returns the number of bytes read or -1 for errors.
	ret = recvfrom(nlfd, &u_info, sizeof(struct user_msg_info), 0, (struct sockaddr *) daddr_p, &len);
	if (!ret) {
		perror("recv from kernel error\n");
		close(nlfd);
		exit(-1);
	}

	printf("recv from kernel: %s", u_info.msg);
	printf("\n");
}

struct nlmsghdr *make_nlhdr(int nl_pid) {
	struct nlmsghdr *nlh;
	char *umsg = "hello, netlink";

	nlh = (struct nlmsghdr *) malloc(NLMSG_SPACE(MAX_PAYLOAD));
	memset(nlh, 0, sizeof(struct nlmsghdr));
	nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	nlh->nlmsg_pid = nl_pid;

	memcpy(NLMSG_DATA(nlh), umsg, strlen(umsg));

	return nlh;
}

void nlhdr_release(struct nlmsghdr *nlh) {
	free(nlh);
}


int main() {
	int ret, nlfd;
	struct nlmsghdr *nlh;

	struct sockaddr_nl saddr = {
		.nl_family = AF_NETLINK,
		.nl_pad	= 0,
		.nl_pid = 100,
		.nl_groups = 0
	}, daddr = {0};

	daddr.nl_family = AF_NETLINK;

	// create netlink socket
	if ((nlfd = socket(AF_NETLINK, SOCK_RAW, NETLINK_PRIVATE)) == -1) {
		perror("create socket error\n");
		return -1;
	}

	if (bind(nlfd, (struct sockaddr *) &saddr, sizeof(saddr)) != 0) {
		perror("bink() error\n");
		close(nlfd);
		return -1;
	}

	nlh = make_nlhdr(saddr.nl_pid);
	send_msg(nlfd, nlh, &daddr);
	exit(EXIT_FAILURE)

	recv_msg(nlfd, &daddr);
	close(nlfd);
	nlhdr_release(nlh);

	return 0;
}
