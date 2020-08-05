### extern 和 include  

> 当使用 `include` 的时候, 会将另一个文件的全部包含引用到此文件中, 这样做有可能是不安全的  
> 所以如果只是希望使用另一文件的某个变量, 使用 `extern`关键字是更好的选择

`gcc a.c b.c` or  `make`  

清空： `make clean`

### makefile
- 在Makefile中的命令，必须要以 Tab 键开始。
