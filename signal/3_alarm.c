#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char const *argv[])
{
    alarm(20);
    while (1)
        pause();

    
    return 0;
}
