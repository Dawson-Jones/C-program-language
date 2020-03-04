#include <stddef.h>
typedef long Align;  // 按照long类型的边界对齐

union header{
    struct{
        union header *ptr;  // 空闲块链表中的下一块
        unsigned size;  // 本块的大小
    }s;
    Align x;  // 强制块的对齐
}

typedef union header Header;

static Header base;
static Header *freep = NULL;

// malloc 函数: 通用储存分配函数
malloc(unsigned nbytes){
    Header *p, *prevp;
    Header *morecore(unsigned);
    unsigned nunits;


}
