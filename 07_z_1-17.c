// 打印成都大于20个字符的所有输入行
#include <stdio.h>

int main(int argc, char const *argv[]){
    int c;
    int lt80[100] = {0};
    int index = 0;

    int count = 0;
    while ((c = getchar()) != EOF) {
        if (c != '\n')
            count++;
        else {
            if (count > 20)
                lt80[index] = count;

            count = 0;
            index++;
        }
    }
    
    for (int i = 0; i < 100; i++)
        if (lt80[i] != 0)
            printf("the %d rows chars lt 20\n", i);

    printf("over\n");

    return 0;
}
