// 创建一个新的进程并把他放到新的namespace中

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

#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child 1M*/


static int child_func(void *hostname)
{
    struct utsname uts;

    // 子进程的 hostname
    if (sethostname(hostname, strlen(hostname)) == -1)
        errExit("sethostname");

    // get name and information about current kernel
    if (uname(&uts) == -1)   
        errExit("uname");
    printf("uts.nodename in child:  %s\n", uts.nodename);

    // sleep(120);
    // 用 bash 替换当前子进程
    // 执行完execlp后，子进程没有退出，也没有创建新的进程,
    // 只是当前子进程不再运行自己的代码，而是去执行bash的代码,
    execlp("bash", "bash", (char *) NULL);

    /* 从这里开始的代码将不会被执行到，因为当前子进程已经被上面的bash替换掉了 */

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

    // 申请一块内存, 用来当作新的进程的栈空间
    stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if (stack == MAP_FAILED)
        errExit("mmap");
    
    // 栈空间从高地址到低地址
    stackTop = stack + STACK_SIZE;

    child_pid = clone(child_func,   // 执行函数
                        stackTop,   // 栈地址
                        CLONE_NEWUTS | SIGCHLD, // UTS ns | 子进程退出返回给父进程的信息
                        argv[1]     // 函数参数
                );
    if (child_pid == -1)    // 父进程继续执行
        errExit("clone");
    printf("clone() returned %jd\n", (intmax_t) child_pid);
    sleep(1);

    if (uname(&uts) == -1)
        errExit("uname");
    printf("uts.nodename in parent: %s\n", uts.nodename);

    if (waitpid(child_pid, NULL, 0) == -1)
        errExit("waitpid");
    
    munmap(stack, STACK_SIZE);
    printf("child has terninated\n");

    return 0;
}
