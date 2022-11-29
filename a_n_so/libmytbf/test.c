#include <stdio.h>
#include <stddef.h>
#include <string.h>

#include "mytbf.h"


int main(int argc, char const *argv[])
{
    int token;
    mytbf_t *tbf = mytbf_init(2, 5);
    while (token = mytbf_fetchtoken(tbf, 2)) {
        printf("get %d tokens\n", token);
    }

    return 0;
}
