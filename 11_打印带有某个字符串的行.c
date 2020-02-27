#include <stdio.h>

#define MAXLINE 1000 // 最大输入行长度

int get_line(char line[], int max);
int strindex(char source[], char searchfor[]);

char pattern[] = "ould";

int main()
{
    char line[MAXLINE];
    int found = 0;

    while (get_line(line, MAXLINE) > 0)
    {
        if (strindex(line, pattern) >= 0)
        {
            printf("%s", line);
            found++;
        }
    }
    return found;
}

int get_line(char s[], int lim)
{
    int c, i;
    i = 0;
    while (--lim > 0 && (c = getchar()) != '0' && c != '\n')
        s[i++] = c;
    if (c == '\n')
        s[i++] = c;
    s[i] = '\0';
    return i; // 返回一行的长度
}

int strindex(char s[], char t[])
{
    int i, j, k;
    for (i = 0; s[i] != EOF; i++)
    {
        for (j = i, k = 0; t[k] != '\0' && s[j] == t[k]; j++, k++)
            ;                      // 字符串 t[k] 匹配上了
        if (k > 0 && t[k] == '\0') // 如果全部匹配上了 -> t[k] 已经匹配完了 '\0'
        {
            return i;
        }
    }
    return -1;
}
