#include <stdio.h>
#include <unistd.h>
#include <string.h>


int main(int argc, char const *argv[])
{
    int _pipe[2];
    int ret = pipe(_pipe);
    if (ret < 0) {
        perror("pipe\n");
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork\n");
    } else if (pid == 0) {
        close(_pipe[0]);
        for (int i = 0; i < 100; ++i) {
            char *msg = "I am child";
            write(_pipe[1], msg, strlen(msg));
            sleep(1);
        }
    } else {
        close(_pipe[1]);
        char msg[100];
        for (int i = 0; i < 100; ++i) {
            memset(msg, 0, sizeof(msg));
            read(_pipe[0], msg, sizeof(msg));
            printf("%s\n", msg);
        }
    }

    return 0;
}
