//
// Created by Dawson-Jones on 2020/8/6.
//

/*
 * string.h 标准库函数测试样例
 */
#include <stdio.h>
#include <string.h>

int main() {
    // 1.
    char s[13] = "hello ";      // 7个字符
    char *t = "world\n";        // 7个字符, 合并去掉 s 最后的'\0'共13个字符
    strcat(s, t);       // 拼接 s 和 t， s 必须由足够的内存存放 t 指向的字符串
    printf("%s", s);    // hello world

    // 2.
    char s[13] = "hello ";
    char *t = "world\n";
    return 0;
}

