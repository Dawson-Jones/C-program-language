#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "mytbf.h"


int main(int argc, char const *argv[])
{
    int token;
    mytbf_t *tbf = mytbf_init(1, 10);

    while (1) {
        token = mytbf_fetchtoken(tbf, 10);
        if (token < 0) {
            fprintf(stderr, "mytbf_fetchtoken: %s", strerror(-token));
        }

        printf("*\n");
    }

    mytbf_destroy(tbf);
    return 0;
}
