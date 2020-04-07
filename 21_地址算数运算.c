#define ALLOCSIZE 10000  // 可用空间大小

static char allocbuf[ALLOCSIZE];  // alloc使用的储存区

static char *allocp = allocbuf;  // 下一个空闲位置

char *alloc(int n){
   if(allocbuf+ALLOCSIZE-allocp >= n){  // 头部指针+size-当前指针  有足够的空间
        allocp += n;
        return allocp-n;  // 分配前的指针p
   } else
      return 0;
}

// 释放p指向的储存区, 本质上就是让指针退回去覆盖
// 保证alloc与afree以栈的形式进行储存空间的管理, 仔细想一下为什么
void afree(char *p){  
    if(p>=allocbuf && p<allocbuf + ALLOCSIZE)    
        allocp = p;
}
