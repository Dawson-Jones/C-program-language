#include <stdbool.h>
#include <stdio.h>
#include <setjmp.h>

static jmp_buf getNext_caller_jmp;
static jmp_buf getNext_gen_jmp;

static _Bool getNext_continue = 0;
static int getNext_ret;

void getNext__real(int n);

int getNext(int n) { 
    if (!getNext_continue) { 
        getNext_continue = 1; 
        if (setjmp(getNext_caller_jmp) == 0) {
            getNext__real(n); 
        } else { 
            return getNext_ret; 
        } 
    } else { 
        longjmp(getNext_gen_jmp, 1); 
    } return 0; 
} 

void getNext__real(int n) {
    static int counter;

    counter = n;
    while (true) {
        counter += 1;
        if (setjmp(getNext_gen_jmp) == 0) { 
            getNext_ret = counter; 
            longjmp(getNext_caller_jmp, 1); 
        }
    }
}

int main(int argc, char const *argv[])
{
    while (true) {
        int n = getNext(1);
        if (n > 42) {
            break;
        }

        printf("%d\n", n);
    }

    return 0;
}
