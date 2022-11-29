# C-program-language
## 二维指针	

```c
int *arr[3];
// 和
int (*arr)[3];
// 的区别
```

第一个是 `3` 个 `int` 类型的的指针. (可以是数组指针)

第二个是指针指向的是一个具有 `3` 个 `int` 类型的数组. (数组指针的每个指向都是 `3` 个 `int` 类型的数组)
```c
    int arr[3][3] = {
            {1, 2, 3},
            {4, 5, 6},
            {7, 8, 9},
    };
    int (*tmp)[3] = &arr[1];
```

> 一个 2D 的数组在传递的时候是不能使用 `int**`来传递的, 这是因为

```c
int arr[3][3] = {
        {0, 0, 0},
        {0, 1, 0},
        {0, 0, 0},
};
// 这是一块连续的 9 * sizeof(int) 的内存块

/* 当传递的是 (int **)arr 的时候
 * 调用的函数在计算比如: arr
 * 只知道这是个指针, 却并不知道这个指针的列数
 * 所以当调用的 arr[row][col] 的时候只能找到 row, 但是找不到 col
 */


int l1[3] = {0, 0, 0};
int l2[3] = {0, 1, 0};
int l3[3] = {0, 0, 0};
int *arr[3] = {l1, l2, l3};
// 当传递 arr 的时候, 这里是可以通过 arr[m][n] 找到对应的值的
```



## 位运算技巧

- 寻找最低位为 1 的 位置

    `num & ~(num - 1)`

- 按 N 位进行对齐 (N进制进1法)

    (len + N - 1) & ~(N - 1)



## 宏container_of(ptr, type, member)

> 通过结构体 *成员变量* 地址获取 *该结构体* 的地址

```c
#define container_of(ptr, type, member) ({				\
	void *__mptr = (void *)(ptr);					\
	BUILD_BUG_ON_MSG(!__same_type(*(ptr), ((type *)0)->member) &&	\
			 !__same_type(*(ptr), void),			\
			 "pointer type mismatch in container_of()");	\
	((type *)(__mptr - offsetof(type, member))); })
```

### 参数

- ptr

    成员变量的地址

- type

    该结构体类型

- member

    该成员在结构体中的名字
    
    

### 知识点

#### ({}) 的作用

> 返回最后一个表达式的值。比如`x = ({a;b;c;d;})`，最终x的值应该是d。

```c
#include<stdio.h>

void main(void)
{
    int a=({1;2;4;})+10;
    printf("%d\n",a);  //a=14
}
```



#### typeof: 根据变量获取变量类型

```c
void main(void)
{
    int a = 6;
    typeof(a) b =9;	// typeof(a) is int
    printf("%d %d\n",a, b);
}
```



#### (struct st *) 0 的作用

**看例子**

```c
struct st{
    int a;
    int b;
} *p_st, n_st;

void main(void)
{
    printf("%p\n", &((struct st*)0)->b);	// 0x4
}
```



把 0 地址强制转换为该类型, 然后获取成员相较于0的地址

相当于尺子



### offset 的实现

```c
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE*)0)->MEMBER)
```

>获取成员的在结构体中的偏移量

```c
struct st{
    int a;
    int b;
} *p_st, n_st;

void main(void)
{
    printf("%lu\n", offsetof(struct st, b));	// 4
}
```

