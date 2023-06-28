#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/eventfd.h>
#include <errno.h>

static inline 
void die(const char *msg) {
    fprintf(stderr, "error: %s: %s(%d)\n", msg, strerror(errno), errno);
    exit(EXIT_FAILURE);
}



int main(int argc, char const *argv[]) {
    char buf[BUFSIZ];
    int event_fd, ec_fd, oom_fd;
    uint64_t u;
    if ((event_fd = eventfd(0, 0)) == -1)
        die("eventfd");
    snprintf(buf, BUFSIZ, "%s/%s", argv[1], "cgroup.event_control");
    if ((ec_fd = open(buf, O_WRONLY)) == -1)
        die("cgroup.event_control");
    snprintf(buf, BUFSIZ, "%s/%s", argv[1], "memory.oom_control");
    if ((oom_fd = open(buf, O_WRONLY)) == -1)
        die("memory.oom_control");

    snprintf(buf, BUFSIZ, "%d %d", event_fd, oom_fd);
    if (write(ec_fd, buf, strlen(buf)) == -1)
        die("write cgroup.event_control");
    
    if (close(ec_fd) == -1)
        die("close cgroup.event_control");
    if (close(oom_fd) == -1)
        die("close memory.oom_control");
    
    for (;;) {
        if (read(event_fd, &u, sizeof(uint64_t)) != sizeof(uint64_t))
            die("read eventfd");

        printf("mem_cgroup oom event received\n");
    }
    return 0;
}

