#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <sys/select.h>
#include <stdarg.h>


#define CLIENT              0
#define SERVER              1
#define REMOTE_IP_STR_SIZE  16
#define BUFSIZE             2000

static const char   *proc_name;
static int          flags = IFF_TUN;
static char         if_name[IFNAMSIZ];
static bool         debug = false;
static int          cliserv = -1;
static char         remote_ip[REMOTE_IP_STR_SIZE];
static int          port = 55555;


void do_debug(char *msg, ...)
{

    va_list argp;

    if (debug) {
        va_start(argp, msg);
        vfprintf(stderr, msg, argp);
        va_end(argp);
    }
}

int tun_alloc()
{
    struct ifreq ifr;
    int fd, err;
    char *clone_dev = "/dev/net/tun";

    if ((fd = open(clone_dev, O_RDWR)) < 0)
        return fd;

    printf("fd: %d\n", fd);
    memset(&ifr, 0, sizeof(ifr));

    ifr.ifr_flags = flags;

    if (*if_name)
        strncpy(ifr.ifr_ifrn.ifrn_name, if_name, IFNAMSIZ);
    
    if ((err = ioctl(fd, TUNSETIFF, (void *) &ifr)) < 0) {
        close(fd);
        return err;
    }

    strcpy(if_name, ifr.ifr_name);
    return fd;
}

static void usage() {
    printf("Usage:\n\n");
    printf("%s -i <interface> [-s|-c <server ip>] [-p <port>] [-u|-a] [-d]\n", proc_name);
    printf("%s -h\n", proc_name);
    printf("\n");
    printf("    -i <interface>:     Name of interface to use (mandatory)\n");
    printf("    -p <port>:          port to liten on (if run in server mode) or to connect to (in client mode)\n");
    printf("    -s|-c <server ip>:  run in server mode (-s), or specify server address (-c <server ip>) (mandatory)\n");
    printf("    -u|-a:              use tun mode (-u default) or tap mode (-a)\n");
    printf("    -d:                 outputs debug information while running\n");
    printf("    -h:                 prints this help text\n");

    exit(0);
}

static void parse_command(int argc, char **argv)
{
    int opt;
    proc_name = argv[0];

    while ((opt = getopt(argc, argv, "i:sc:p:uahd")) != -1) {
        switch (opt) {
        case 'i':
            strncpy(if_name, optarg, IFNAMSIZ - 1);
            break;
        case 'd':
            debug = true;
            break;
        case 'u':
            flags = IFF_TUN;
            break;
        case 'a':
            flags = IFF_TAP;
            break;
        case 'c':
            cliserv = CLIENT;
            strncpy(remote_ip, optarg, REMOTE_IP_STR_SIZE - 1);
            break;
        case 's':
            cliserv = SERVER;
            break;
        case 'p':
            port = atoi(optarg);
            break;

        case 'h':
        default:
            usage();
            break;
        }
    }

    argv += optind;
    argc -= optind;
    if (argc > 0) {
        fprintf(stderr, "Too many options\n");
        usage;
    }
    
    if (*if_name == 0)
        usage();
    if (cliserv < 0)
        usage();
    if (cliserv == CLIENT && (*remote_ip == '\0'))
        usage();
    
}

static void dump(char *buffer, int nread) 
{
    int i, j;

    for (i = 0; i < (nread + 15) / 16; ++i) {
        for (j = 0; j < 16 && 16 * i + j < nread; ++j)
            printf("%.2x ", buffer[i * 16 + j]);

        printf("\n");
    }
}

static int cread(int fd, char *buf, int n)
{
    int nread;

    nread = read(fd, buf, n);
    if (nread < 0) {
        perror("reading data");
        exit(EXIT_FAILURE);
    }

    return nread;
}

static int cwrite(int fd, char *buf, int n)
{
    int nwrite;

    nwrite = write(fd, buf, n);
    if (nwrite < 0) {
        perror("writint data");
        exit(EXIT_FAILURE);
    }

    return nwrite;
}

static int read_n(int fd, char *buf, int n)
{
    int nread, left = n;

    while(left > 0) {
        if ((nread = cread(fd, buf, left)) == 0) {
            return 0;
        } else {
            left -= nread;
            buf += nread;
        }
    }

    return n;
}

static void serve(int tap_fd, int net_fd)
{
    uint16_t nread, nwrite, package_len;
    int max_fd = tap_fd > net_fd ? tap_fd : net_fd;
    char buffer[BUFSIZE];

    while (1) {
        int ret;
        fd_set rd_set;

        FD_ZERO(&rd_set);
        FD_SET(tap_fd, &rd_set);
        FD_SET(net_fd, &rd_set);

        ret = select(max_fd + 1, &rd_set, NULL, NULL, NULL);
        if (ret < 0) {
            if (errno == EINTR)
                continue;

            perror("select()");
            exit(EXIT_FAILURE);
        }

        if (FD_ISSET(tap_fd, &rd_set)) {
            nread = read(tap_fd, buffer, BUFSIZE);
            // dump(buffer, nread);
            do_debug("read %d bytes from tap interface\n", nread);

            package_len = htons(nread);
            cwrite(net_fd, (char *) &package_len, sizeof(package_len));
            nwrite = cwrite(net_fd, buffer, nread);

            do_debug("written %d bytes from to network\n", nwrite);
        } 

        if (FD_ISSET(net_fd, &rd_set)) {
            nread = read_n(net_fd, (char *) &package_len, sizeof(package_len));
            if (nread == 0) {   // ctrl-c at the other end
                break;
            }

            nread = read_n(net_fd, buffer, ntohs(package_len));
            // dump(buffer, nread);
            do_debug("read %d bytes from network\n", nread);

            nwrite = cwrite(tap_fd, buffer, nread);
            do_debug("written %d bytes from to tap interface\n", nwrite);
        }         
    }
}

// TODO: 
// 1. close sock_fd
// 2. serve multi cli_fd     
int socket_alloc() {
    int sock_fd, net_fd; 
    int err, optval = 1;
    struct sockaddr_in local, remote;

    sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (sock_fd < 0) {
        return sock_fd;
    }

    if (cliserv == CLIENT) {
        memset(&remote, 0, sizeof(remote));
        remote.sin_family = AF_INET;
        remote.sin_addr.s_addr = inet_addr(remote_ip);
        remote.sin_port = htons(port);

        err = connect(sock_fd, (struct sockaddr *) &remote, sizeof(remote));
        if (err < 0) {
            return err;
        }

        net_fd = sock_fd;
        do_debug("CLIENT: Connected to server %s\n", inet_ntoa(remote.sin_addr));
    } else {
        err = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, (char *) &optval, sizeof(optval));
        if (err < 0) {
            return err;
        }

        memset(&local, 0, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        local.sin_port = htons(port);

        err = bind(sock_fd, (struct sockaddr *) &local, sizeof(local));
        if (err < 0) {
            return err;
        }

        err = listen(sock_fd, 5);
        if (err < 0) {
            return err;
        }

        int remotelen = sizeof(remote);
        memset(&remote, 0, remotelen);

        net_fd = accept(sock_fd, (struct sockaddr *) &remote, &remotelen);
        if (net_fd < 0) {
            return net_fd;
        }

        do_debug("SERVER: Client connected from %s\n", inet_ntoa(remote.sin_addr));
    }

    return net_fd;
}

int main(int argc, char **argv)
{
    int tap_fd, net_fd;

    parse_command(argc, argv);
    do_debug("interface name: %s\n", if_name);
    do_debug("type: %d\n", flags);

    tap_fd = tun_alloc();
    if (tap_fd < 0) {
        perror("Allocating interface");
        exit(EXIT_FAILURE);
    }

    net_fd = socket_alloc();
    if (net_fd < 0) {
        perror("socket_alloc");
        exit(EXIT_FAILURE);
    }


    serve(tap_fd, net_fd);

    do_debug("end....\n");
    close(tap_fd);
    close(net_fd);
    return 0;
}
