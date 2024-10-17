// UTS namespace用来隔离系统的hostname以及NIS domain name。

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdint.h>
#include <unistd.h>
#include <sched.h>
#include <sys/wait.h>
#include <sys/mman.h>
#include <sys/utsname.h>

#define errExit(msg) do {perror(msg); exit(EXIT_FAILURE);} while (0)

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */


static int child_func(void *hostname)
{
    struct utsname uts;

    if (sethostname(hostname, strlen(hostname)) == -1)
        errExit("sethostname");

    execlp("bash", "bash", (char *) NULL);
    return 0;
}


int main(int argc, char *argv[]) {
    pid_t child_pid;
    char *stack;
    char *stackTop;
    struct utsname uts;
    

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <child-hostname>\n", argv[0]);
        return -1;
    }

    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        errExit("mmap");
    
    // 栈是从高位向低位增长，所以这里要指向高位地址
    stackTop = stack + STACK_SIZE;

    child_pid = clone(child_func, stackTop, CLONE_NEWUTS | SIGCHLD, argv[1]);
    if (child_pid == -1)
        errExit("clone");
    printf("clone() returned %jd\n", (intmax_t) child_pid);
    sleep(1);

    if (waitpid(child_pid, NULL, 0) == -1)
        errExit("waitpid");
    printf("child has terninated\n");

    return 0;
}

/*
- compile 
make clone_uts_2

- run the program
sudo ./clone_uts_2 container001

```bash
$ sudo ./clone_uts_2 container001
[sudo] password for dawson: 
clone() returned 316391
# 进入到了 子进程的 bash 中
root@container001:~# hostname
container001
# pstree -pl
systemd(1)─┬
           ├─sh(314825)───node(314829)─┬─node(314879)─┬─bash(315968)───sudo(316378)───sudo(316389)───clone_uts_2(316390)───bash(316391)───pstree(316581)

# echo $$
316391

# 验证 子进程 和 父进程 是否在不同的 UTS namespace
root@container001:# readlink /proc/$$/ns/uts
uts:[4026533446]
root@container001:# readlink /proc/self/ns/uts 
uts:[4026533446]
root@container001:# readlink /proc/316391/ns/uts
uts:[4026533446]
root@container001:# readlink /proc/316390/ns/uts
uts:[4026531838]
root@container001:# readlink /proc/1/ns/uts 
uts:[4026531838]
# 再次创建一个新的 UTS namespace
root@container001:# ./clone_uts_2 container002
clone() returned 321948
root@container002:# hostname
container002
```
*/