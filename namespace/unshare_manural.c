// 使当前进程退出指定类型的namespace，并加入到新创建的namespace（相当于创建并加入新的namespace）
// 使当前进程加入新的namespace

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sched.h>
#include <fcntl.h>

#define errExit(msg)    do { perror(msg); exit(EXIT_FAILURE); } while (0)


static void usage(char *pname)
{
    fprintf(stderr, "Usage: %s [options] program [arg...]\n", pname);
    fprintf(stderr, "Options can be: \n");
    fprintf(stderr, "    -C unshare cgroup namespace\n");
    fprintf(stderr, "    -i unshare IPC namespace\n");
    fprintf(stderr, "    -m unshare mount namespace\n");
    fprintf(stderr, "    -n unshare network namespace\n");
    fprintf(stderr, "    -p unshare PID namespace\n");
    fprintf(stderr, "    -t unshare time namespace\n");
    fprintf(stderr, "    -u unshare UTS namespace\n");
    fprintf(stderr, "    -U unshare user namespace\n");
    
    exit(0);
}

int main(int argc, char *argv[]) {
    int flags = 0;
    int opt;

    pid_t pid =  getpid();
    printf("pid: %jd\n", (intmax_t) pid);

    while ((opt = getopt(argc, argv, "CimnptuU")) != -1) {
        switch (opt) {
        case 'C': flags |= CLONE_NEWCGROUP;  break;
        case 'i': flags |= CLONE_NEWIPC;     break;
        case 'm': flags |= CLONE_NEWNS;      break;
        case 'n': flags |= CLONE_NEWNET;     break;
        case 'p': flags |= CLONE_NEWPID;     break;
        case 'u': flags |= CLONE_NEWUTS;     break;
        case 'U': flags |= CLONE_NEWUSER;    break;
        default: usage(argv[0]);
        }
    }
    if (optind >= argc)
        usage(argv[0]);
    
    if (unshare(flags) == -1)
        errExit("unshare");
    
    execvp(argv[optind], &argv[optind]);
    errExit("evecvp");

    return 0;
}


/*
    $ readlink /proc/$$/ns/mnt
    mnt:[4026531840]
    $ sudo ./unshare -m /bin/bash
    # readlink /proc/$$/ns/mnt
    mnt:[4026532325]
*/
