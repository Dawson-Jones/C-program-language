#include <stdio.h>
#define MAXLINE 1000

int get_line(char s[], int lim)
{ // 把输入的字符(一行)放在 s 数组中, 并返回这一行的长度, 如果长度超过1000, s只保存前998个字符
    int c, i, j;
    j = 0;
    for (i = 0; (c = getchar()) != EOF && c != '\n'; ++i)
        if (i < lim - 2)
        {
            s[j] = c;
            ++j;
        }
    if (c == '\n')
    {
        s[j] = c;
        ++j;
        ++i;
    }
    s[j] = '\0';
    return i;

    if (c == '\n')
    {
        s[i] = '\0';
        ++i;
    }
    return i;
}

void reverse(char s[])
{
    int i, j;
    char temp;
    i = 0;
    while (s[i] != '\0')
        ++i; // 测量长度
    --i;     // 长度-1, 索引
    if (s[i] == '\n')
        --i; // 去掉换行符
    j = 0;
    while (j < i)
    {
        // swap  你真牛逼
        temp = s[j];
        s[j] = s[i];
        s[i] = temp;
        --i;
        ++j;
    }
}
int main(int argc, char const *argv[])
{
    char line[MAXLINE];
    while (get_line(line, MAXLINE) > 0)
    {
        reverse(line);
        printf("%s", line);
    }

    return 0;
}
