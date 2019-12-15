#include <stdio.h>

//#define 后面无分号
#define LOWER 0
#define UPPER 300
#define STEP 20

void main()
{
//    int fahr, celsius;
   for(int fahr = LOWER;fahr <= UPPER; fahr += STEP) 
   {
       /*
       %d       按照十进制整型数打印
       %6d      整型, 至少6个字宽  
       %f       浮点数
       %6f      浮点, 至少6个字宽
       %.2f     浮点, 小数点2位小数
       %6.2f    浮点, 至少6个字宽, 小数点2位小数
       */
       printf("%3d %6.1f\n", fahr, (5.0/9.0)*(fahr-32));
   }
}