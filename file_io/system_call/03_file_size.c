#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/syscall.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/uio.h>
#include "dirent.h"

void fsize(char *);

int main(int argc, char const *argv[])
{
    if (argc == 1)
        fsize(".");
    else
        while (--argc > 0)
            fsize(*++argv);
        
    return 0;
}

