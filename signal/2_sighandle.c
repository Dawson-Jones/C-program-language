// 信号会打断阻塞的系统调用
// 如果一直按着 ^c, 会直接退出
// 这里阻塞的系统调用就是 write


#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sig_handle(int sig) {
    write(1, "!", 1);
}

int main(int argc, char const *argv[])
{
    // 忽略 SIGINT 信号
    signal(SIGINT, sig_handle);

    for (int i = 0; i < 10; i++) {
        write(1, "*", 1);
        slee1p(1);
    }
    
    return 0;
}
