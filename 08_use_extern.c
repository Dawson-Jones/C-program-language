#include <stdio.h>
#define MAXLINE 1000

int max = 0;           // 目前最长
char line[MAXLINE];    // 当前输入行
char longest[MAXLINE]; // 保存最长行
int get_line(void);    // 读取一行的长度
void copy();

int main()
{
    int len;
    extern int max;
    extern char longest[];
    max = 0;
    while ((len = get_line()) > 0)
        if (len > max)
        {
            max = len;
            copy();
        }
    if (max > 0)
        printf("%s", longest);
    return 0;
}

int get_line(void)
{
    int c, i;
    extern char line[];
    for (i = 0; i < MAXLINE - 1 && (c = getchar()) != EOF && c != '\n'; i++)
        line[i] = c;
    if (c == '\n')
    {
        line[i] = c;
        ++i;
    }
    line[i] = '\0';
    return i;
}

void copy(void)
{
    int i = 0;
    extern char line[], longest[];
    while ((longest[i] = line[i]) != '\0')
        ++i;
}