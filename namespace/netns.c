#define _GNU_SOURCE

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sched.h>
#include <string.h>
#include <stdlib.h>

#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/if.h>
#include <linux/in.h>
#include <linux/in6.h>
#include <linux/limits.h>
#include <linux/if_tun.h>


struct nstoken {
    int orig_netns_fd;
};

struct nstoken *open_netns(const char *name)
{
    int nsfd;
    char nspath[PATH_MAX];
    int err;
    struct nstoken *token;

    token = calloc(1, sizeof(struct nstoken));
    if (!token) {
        perror("calloc error");
        return NULL;
    }

    token->orig_netns_fd = open("/proc/self/ns/net", O_RDONLY);
    if (token->orig_netns_fd < 0) {
        perror("open error");
        free(token);
        return NULL;
    }

    snprintf(nspath, sizeof(nspath), "%s/%s", "/var/run/netns", name);
    nsfd = open(nspath, O_RDONLY|O_CLOEXEC);
    if (nsfd < 0) {
        perror("open netns fd");
        free(token);
        return NULL;
    }

    err = setns(nsfd, CLONE_NEWNET);
    close(nsfd);
    if (err) {
        perror("setns");
        free(token);
        return NULL;
    }

    return token;
}

void close_netns(struct nstoken *token)
{
	setns(token->orig_netns_fd, CLONE_NEWNET);
	close(token->orig_netns_fd);
	free(token);
}


static int tun_open(char *name)
{
	struct ifreq ifr;
	int fd, err;

	fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        perror("open tun");
        return -1;
    }
	memset(&ifr, 0, sizeof(ifr));

	ifr.ifr_flags = IFF_TUN | IFF_NO_PI;
	if (*name)
		strncpy(ifr.ifr_name, name, IFNAMSIZ);

	err = ioctl(fd, TUNSETIFF, &ifr);
    if (err) {
        perror("ioctl TUNSETIFF");
        goto fail;
    }

    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "ip link set dev %s up", name);
    if ((err = system(cmd)) != 0) {
        goto fail;
    } 

    printf("----------up\n");
	return fd;

fail:
	close(fd);
	return -1;
}

int main(int argc, char const *argv[])
{
    struct nstoken *nstoken = open_netns("veth0");
    if (!nstoken) {
        return EXIT_FAILURE;
    }

    int src_fd = tun_open("tun_src");
    if (src_fd < 0) {
        return EXIT_FAILURE;
    }

    // close_netns(nstoken);

    return 0;
}
