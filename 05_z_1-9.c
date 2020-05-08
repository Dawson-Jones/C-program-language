/*
cp input to output
    if mutil space use one instead
*/
#include <stdio.h>
#define NONBLANK 'a'

int main(){
    int c, last_char;
    last_char = NONBLANK;
    while ((c = getchar())!= EOF){
        if (last_char != ' ' || c != ' ')
            putchar(c);
        last_char =c;
    }

    return 0;
}