#include <stdlib.h>

#define NALLOC 1024

typedef long Align;

typedef union header Header;
union header {
    struct {
        union header *next_block_header;
        unsigned size;      // size of this block, unit is sizeof(Header), 这个size包括了header本身
    } s;
    Align x;        // will not use forever, use to align block
};

static Header base;             // start with null link
static Header *freep = NULL;    // 上次找到空闲块的前面一个块

void *my_malloc(unsigned nbytes)
{
    Header *p, *prevp;
    Header *my_morecore(unsigned);
    unsigned nunits;

    // 向上取整 再加1, nunits的意思是: 需要 n + 1 个 Header 大小的 byte
    nunits = (nbytes + sizeof(Header) - 1) / sizeof(Header) + 1;    // n 个小单元

    // 初始化?
    if ((prevp = freep) == NULL) {      // 无空闲链表, 同时把 prev 指向了上次找到的空闲块
        base.s.next_block_header = freep = prevp = &base;      // 指向了自己
        base.s.size = 0;
    }

    for (p = prevp->s.next_block_header; ; prevp = p, p = p->s.next_block_header) {
        if (p->s.size >= nunits) {      // 满足要求
            if (p->s.size == nunits)    // 刚好满足要求
                prevp->s.next_block_header = p->s.next_block_header;
            else {                      // 分配该块末尾的部分
                p->s.size -= nunits;    // size 减去了比分配出去多一个单元到 size
                p += p->s.size;         // 把 p 移向准备返回的内存指针的前一单元
                p->s.size = nunits;     // 建了一个新的头，记录着此时 header 到后面分配出去到 total_size
            }
            freep = prevp;
            return (void *)(p+1);       // 去掉 Header 部分, 把后面的部分返回
        }

        /*
         * 回到了上次 malloc 的地方, 形成了闭环
         * 表示当前已经没有合适的块满足申请的大小要求
         */
        if (p == freep)
            if ((p = my_morecore(nunits)) == NULL)
                return NULL;
    }
}

static Header *my_morecore(unsigned nu)
{
    char *cp;
    char *sbrk(int);
    Header *up;

    if (nu <NALLOC)
        nu = NALLOC;
    cp = sbrk(nu * sizeof(Header));
    if (cp == (char *) -1)  // 没有向系统申请到空间
        return NULL;
    up = (Header *) cp;
    up->s.size = nu;

    // 释放 up 后面的内存, 这里有一个header没有被释放
    my_free((void *) (up + 1));
    return freep;
}

void my_free(void *ap)
{
   Header *bp, *p;

   bp = (Header *)ap - 1;    // 指向块头

   /*
    * bp <= p || bp >= p->s.next_block_header
    */
   for (p = freep; !(bp > p && bp < p->s.next_block_header); p = p->s.next_block_header) {
       if (p >= p->s.next_block_header && (bp > p || bp < p->s.next_block_header))
           break;   // 被释放的块在链表的开头或末尾
   }

   /* 与上个相邻块合并 */
   if (bp + bp->s.size == p->s.next_block_header) {
       bp->s.size += p->s.next_block_header->s.size;
       bp->s.next_block_header = p->s.next_block_header->s.next_block_header;
   } else {
       bp->s.next_block_header = p->s.next_block_header;
   }
   /* 与下个相邻块合并 */
    if (p + p->s.size == bp) {
        p->s.size += bp->s.size;
        p->s.next_block_header = bp->s.next_block_header;       // 合并到了p的后面
    } else {
        p->s.next_block_header = bp;
    }
    freep = p;
}