#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct file_obj_s file_obj_t;
struct file_obj_s {
   FILE *fd;
   file_obj_t *(*seek)(file_obj_t *self, long offset, int where);
   long (*tell)(file_obj_t *self);
   int (*close)(file_obj_t *self);
};

file_obj_t *
seek(file_obj_t *self, long offset, int where) {
    fseek(self->fd, offset, where);
    return self;
}

long
tell(file_obj_t *self) {
    return ftell(self->fd);
}

int
close(file_obj_t *self) {
    int res = fclose(self->fd);             // 成功关闭返回 0
    free(self);
    return res;
}

file_obj_t *
open(char *fn, const char *mode) {
    file_obj_t *obj = (file_obj_t *) malloc(sizeof(file_obj_t));
    if (!obj) exit(1);
    obj->fd = fopen(fn, mode);
    obj->seek = seek;
    obj->tell = tell;
    obj->close = close;
    return obj;
}


// 一次性读取文件所有内容
char *
get_file_all(char *fn) 
{
    FILE *fd_r;                         // 读取的文件描述符
    char *str;
    char txt[1000];
    long int file_size;

    fd_r = fopen(fn, "r");
    if (!fd_r) {
        printf("打开文件%s错误\n",fn);
        exit(1);
    }
    fseek(fd_r, 0, SEEK_END);          // 从文件最后移动一个偏移量
    file_size = ftell(fd_r);           // 获取文件指针到文件头的字节数
    str = (char *) calloc(file_size, sizeof(char));
    rewind(fd_r);                      // 文件指针重新指向开头

    while (fgets(txt, 1000, fd_r) != 0)
        strcat(str, txt);

    fclose(fd_r);
    printf("%s", str);
    return str;
};

int
main()
{
    file_obj_t *fd = open("test.txt", "r");
    long res = fd->seek(fd, 3, SEEK_SET)->tell(fd);
    printf("res: %ld\n", res);
    fd->close(fd);
    return 0;
}