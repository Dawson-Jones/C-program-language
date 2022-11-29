#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>


#define QLEN 10
const char *listen_name = "foo.sock";

int serv_listen() {
    int fd, len;

    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        return -1;
    }

    // unlink(name);   // remove the specified file

    struct sockaddr_un un;
    un.sun_family = AF_UNIX;
    strcpy(un.sun_path, listen_name);

    len = offsetof(struct sockaddr_un, sun_path) + strlen(listen_name);

    if (bind(fd, (struct sockaddr *) &un, len) < 0) {
        perror("bind");
        close(fd);
        return -2;
    }

    if (listen(fd, QLEN) < 0) {
        perror("listen");
        close(fd);
        return -3;
    }

    return fd;
}


int serv_accept(int listen_fd, uid_t *uidptr) {
    int cli_fd, len, err, rval;
    time_t staletime;
    struct sockaddr_un un;
    struct stat statbuf;

    len = sizeof(un);
    if ((cli_fd = accept(listen_fd, (struct sockaddr *) &un, &len)) < 0) {
        perror("accept");
        return -1;
    }

    len -= offsetof(struct sockaddr_un, sun_path);
    un.sun_path[len] = 0;

    if (stat(un.sun_path, &statbuf) < 0) {
        perror("stat");
        close(cli_fd);
        return -2;
    }

    if (S_ISSOCK(statbuf.st_mode) == 0) {
        perror("S_ISSOCK");
        close(cli_fd);
        return -3;
    }

    if (uidptr != NULL)
        *uidptr = statbuf.st_uid;   // User ID of the file's owner
    
    unlink(un.sun_path);
    return cli_fd;
}

void sighandle(int signo) {
    unlink(listen_name);
    exit(0);
}

int main(int argc, char const *argv[])
{

    int fd = serv_listen();
    if (fd < 0) {
        return 1;
    }

    signal(SIGINT, sighandle);
    signal(SIGHUP, sighandle);
    signal(SIGTERM, sighandle);

    uid_t uid;
    int cli_fd = serv_accept(fd, &uid);

    printf("accept from [%u]\n", uid);

    char buf[1024];
    memset(buf, 0, sizeof(buf));
    int n = read(cli_fd, buf, sizeof(buf));
    if (n < 0) {
        perror("read");
        return 0;
    }
    printf("received: %s\n", buf);

    char *resp = "hello from server";
    n = write(cli_fd, resp, strlen(resp));
    if (n < 0) {
        perror("write");
        return 0;
    }

    kill(0, SIGTERM);
    return 0;
}
