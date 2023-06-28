#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv)
{

    char theLink[128];
    char thePath[128];

    strcpy(thePath, "/sys/class/net/");

    memset(theLink, 0, 128);

    if (argc > 1) {
        strcat(thePath, argv[1]);
    } else {
        printf("Gimme device\n");
        return 1;
    }

    if (readlink(thePath, theLink, 127) == -1) {
        perror(argv[1]);
    } else {
        if (strstr(theLink, "/virtual")) {
            printf("%s is a virtual device\n", argv[1]);
        }
        else {
            printf("%s is a physical device\n", argv[1]);
        }
    }
}