#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>


int main(int argc, char const *argv[])
{
    // 忽略 SIGINT 信号
    signal(SIGINT, SIG_IGN);

    for (int i = 0; i < 10; i++) {
        write(1, "*", 1);
        sleep(1);
    }
    
    return 0;
}
