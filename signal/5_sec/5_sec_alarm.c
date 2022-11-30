#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <signal.h>

static volatile int loop = 1;

static void sigalrm_handle(int sig) {
    loop = 0;
}

int main(int argc, char const *argv[])
{
    int64_t count = 0;

    // 信号注册在前
    signal(SIGALRM, sigalrm_handle);
    alarm(5);

    while (loop) {
        count++;
    }
    
    printf("count: %lu\n", count);
    return 0;
}
