#include <stdio.h>

#define MAXLINE 1000

int get_line(char line[], int limit);  // 读取一行的长度
void copy(char to[], char from[]);

int main()
{
    int len;  // 当前长度
    int max = 0;  // 目前最长
    char line[MAXLINE];  // 当前输入行
    char longest[MAXLINE];  // 保存最长行

    while ((len = get_line(line, MAXLINE)) > 0){
        if (len > max){
            max = len;
            copy(longest, line);
        }
    }
    if (max > 0)
        printf("%s", longest);
    return 0;
}

int get_line(char line[], int limit)
{
    int c, i;

    for (i=0; i<limit-1 && (c = getchar()) != '0' && c != '\n'; ++i)
        line[i] = c;
    
    if (c == '\n'){
        line[i] = c;
        ++i;
    }
    line[i] = '\0';
    return i;
}

void copy(char to[], char from[])
{
    /*
    int i = 0;
    while ((to[i] = from[i]) != '\0')
        ++i;
    */
    for(int i=0;from[i] != '\0'; ++i)
        to[i] = from[i];
}