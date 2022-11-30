#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

void sig_handle(int sig) {
    write(1, "!", 1);
}

int main(int argc, char const *argv[])
{
    sigset_t set;
    sigset_t old_set;
    sigemptyset(&set);
    sigaddset(&set, SIGINT);

    signal(SIGINT, sig_handle);

    for (int j = 0; j < 1000; j++) {
        // sigprocmask(SIG_BLOCK, &set, NULL);
        sigprocmask(SIG_SETMASK, &set, &old_set);
        for (int i = 0; i < 5; i++) {
            write(1, "*", 1);
            sleep(1);
        }
        write(1, "\n", 1);
        // sigprocmask(SIG_UNBLOCK, &set, NULL);
        sigprocmask(SIG_SETMASK, &old_set, NULL);
    }

    
    return 0;
}
