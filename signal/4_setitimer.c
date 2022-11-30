#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <sys/time.h>


static void sigalrm_handle(int sig) {
    printf("timer trigger\n");
}

int main(int argc, char const *argv[])
{
    struct itimerval itv;

    // 信号注册在前
    signal(SIGALRM, sigalrm_handle);

    // 如果把这个值写为 1, 会产生一个周期性的timer
    itv.it_interval.tv_sec = 1;
    itv.it_interval.tv_usec = 0;
    itv.it_value.tv_sec = 1;
    itv.it_value.tv_usec = 0;
    if (setitimer(ITIMER_REAL, &itv, NULL) < 0) {
        perror("setitimer");
        exit(1);
    }

    while (1) {
        pause();
        printf("after signal handle\n");
    }
    
    return 0;
}
