#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <sys/types.h>


int main(int argc, char *argv[]) {
    pid_t pid;

    pid = fork();
    if (pid == -1) {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid != 0) { // parent
        printf("Parent (PID=%ld) created child with PID %ld\n",
            (long) getpid(), (long) pid);
        printf("Parent (PID=%ld; PPID=%ld) terminating\n",
            (long) getpid(), (long) getppid());
        
        exit(EXIT_SUCCESS);
    }

    do {
        usleep(100000);
    } while (getppid() != 1);   // getppid = 1, is orphan adopt by init process

    printf("\nChild (PID=%ld) now an orphan (parent PID=%ld)\n",
        (long) getpid(), (long) getppid());
    
    sleep(1);
    
    printf("Child  (PID=%ld) terminating\n", (long) getpid());
    _exit(EXIT_SUCCESS);
}
