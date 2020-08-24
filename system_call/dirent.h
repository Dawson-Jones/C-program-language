#define NAME_MAX    14      // max length of file name

typedef struct {    // 可移植的目录项
    long info;      // i 结点编号
    char name[NAME_MAX+1];      // 文件名 + 结束符 '\0'
} Dirent;

typedef struct {
    int fd;
    Dirent d;
} DIR;

DIR *my_opendir(char *dirname);
Dirent *my_readdir(DIR *dirfd);
void closedir();

char *name;
struct stat stbuf;
int stat(char *, struct stat *);
