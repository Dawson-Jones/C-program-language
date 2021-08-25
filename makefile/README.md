### =, ?=, :=, += 区别

#### = 
> 使用的时候展开

```makefile
HELLO = world
HELLO_WORLD = $(HELLO) world!
#this echoes "world world!"
echo $(HELLO_WORLD)

HELLO = hello
#this echos "hello, world!"
echo $(HELLO_WORLD)
```



#### := 

> 在赋值的时候展开

```makefile
HELLO = world
HELLO_WORLD := $(HELLO) world!	# 展开为了 world world!
#this echoes "world world!"
echo $(HELLO_WORLD)

HELLO = hello
# Still echoes "world world!"
echo $(HELLO_WORLD)

HELLO_WORLD := $(HELLO) world!	# 展开为了 hello world!
# This echoes "hello world!"
echo $(HELLO_WORLD)
```



#### ?=

> 如果之前没有赋值, 则赋值
>
> 如果第一次赋值为空, 仍然不再赋值



#### +=

> 加上后面的值, 替换原有的值

```makefile
HELLO_WORLD = hello
HELLO_WORLD += world!

# This echoes "hello world!"
echo $(HELLO_WORLD)
```



### 自动变量

**（1）$@**

$@指代当前目标，就是Make命令当前构建的那个目标。比如，`make foo`的 $@ 就指代foo。

> ```makefile
> a.txt b.txt: 
>     touch $@
> ```

等同于下面的写法。

> ```makefile
> a.txt:
>     touch a.txt
> b.txt:
>     touch b.txt
> ```

**（2）$<**

$< 指代第一个前置条件。比如，规则为 t: p1 p2，那么$< 就指代p1。

> ```makefile
> a.txt: b.txt c.txt
>     cp $< $@ 
> ```

等同于下面的写法。

> ```makefile
> a.txt: b.txt c.txt
>     cp b.txt a.txt 
> ```

**（3）$^**

$^ 指代所有前置条件，之间以空格分隔。比如，规则为 t: p1 p2，那么 $^ 就指代 p1 p2 。

**（4）$?**

$? 指代比目标更新的所有前置条件，之间以空格分隔。比如，规则为 t: p1 p2，其中 p2 的时间戳比 t 新，$?就指代p2。

**（5）$\***

$* 指代匹配符 % 匹配的部分， 比如% 匹配 f1.txt 中的f1 ，$* 就表示 f1。

**（6）$(@D) 和 $(@F)**

$(@D) 和 $(@F) 分别指向 $@ 的目录名和文件名。比如，$@是 src/input.c，那么$(@D) 的值为 src ，$(@F) 的值为 input.c。

**（7）$(<D) 和 $(<F)**

$(<D) 和 $(<F) 分别指向 $< 的目录名和文件名。



#### ${:}

```makefile
CFLAGS := -Wall -Werror -std=

SRCS := program_1.c \
    program_2.c \
    program_3.c

# 后缀替换
OBJS := ${SRCS:c=o}		# 用 o 替换每个word的后缀 c
PROGS := ${SRCS:.c=}	# 用空 替换每个word的后缀 c

.PHONY: all
all: ${PROGS}

${PROGS} : % : %.o Makefile
    ${CC} $< -o $@

clean:
    rm -f ${PROGS} ${OBJS}

%.o: %.c Makefile
    ${CC} ${CFLAGS} -c $<
```

