#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <time.h>


int main(int argc, char const *argv[])
{
    uint64_t count = 0;
    time_t end = time(NULL) + 5;

    while (time(NULL) < end) {
        count++;
    }
    
    printf("count: %lu\n", count);
    return 0;
}
