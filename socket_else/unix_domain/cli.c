#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/un.h>

#define CLI_PATH "/var/tmp/"   // +5 for pid = 14 chars


int cli_conn(const char *name) {
    int fd, len;
    struct sockaddr_un un;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    un.sun_family = AF_UNIX;
    sprintf(un.sun_path, "%s%05d", CLI_PATH, getpid());
    printf("sub_path: %s\n", un.sun_path);

    len = offsetof(struct sockaddr_un, sun_path) + strlen(un.sun_path);
    unlink(un.sun_path); // in case it already exist

    if (bind(fd, (struct sockaddr *) &un, len) < 0) {
        perror("bind");
        close(fd);
        return -2;
    }
	printf("UNIX domain socket bound\n");

    struct sockaddr_un srv_un;
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, name);
    len = offsetof(struct sockaddr_un, sun_path) + strlen(name);
    if (connect(fd, (struct sockaddr *) &un, len) < 0) {
        perror("connect");
        close(fd);
        return -3;
    }

    return fd;
}

int main(int argc, char const *argv[]) {
    int fd = cli_conn("foo.sock");
    int len;

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    strcpy(buf, "hello, unix socket");

    len = strlen(buf);
    printf("send %s, len: %d\n", buf, len);
    write(fd, buf, len);

    memset(buf, 0, sizeof(buf));
    int n = read(fd, buf, sizeof(buf));

    printf("received %d bytes from endpoint: %s\n", n, buf);

    sleep(10);
	exit(0);
}
