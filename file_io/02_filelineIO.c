#include <stdio.h>
#include <stdlib.h>

char *my_fgets(char *s, int n, FILE *iop)
{
    register int c;
    register char *cs;
    cs = s;

    while (--n > 0 && (c = getc(iop)) != EOF)
        if ((*cs++ = c) == '\n')
            break;

    *cs = '\0';

    // c == EOF && cs == s 说明没有读取到
    return (c == EOF && cs == s) ? NULL : s;
}

int my_fputs(char *s, FILE *iop)
{
    int c;

    while ((c = *s++))
        putc(c, iop);
    return ferror(iop) ? EOF : 1;
}

int main() {
    char str[100];
    my_fgets(str, 100, stdin);
    my_fputs(str, stdout);
    // printf("%s", str);
    return 0;
}