#include <stdio.h>
#include <string.h>

#define MAXLINES 5000 // 排序的最大文本行数

char *lineptr[MAXLINES];

int readlines(char *lineptr[], int nlines);
void writelines(char *lineptr[], int nlines);

void qsort(char *lineptr[], int left, int right);

int main()
{
    int nlines;

    if ((nlines = readlines(lineptr, MAXLINES)) >= 0)
    {
        qsort(lineptr, 0, nlines - 1);
        writelines(lineptr, nlines);
        return 0;
    }
    else
    {
        printf("errot: input too big to sort\n");
        return 1;
    }
}

#define MAXLEN 1000 // 每个输入文本行的最大长度
int getline(char *, int);
char *alloc(int);

/* readlines 函数: 读取输入行 */
int readlines(char *lineptr[], int maxlines)
{
    int len, nlines;
    char *p, line[MAXLEN];
    nlines = 0;
    while ((len = getline(line, MAXLEN)) > 0)
    {
        if (nlines >= maxlines || (p = alloc(len)) == NULL)
        {
            return -1;
        }
        else
        {
            line[len - 1] = '\0';
            strcpy(p, line);
            lineptr[nlines++] = p;
        }
    }
    return nlines;
}

/* writelines 函数: 写输出行 */
void writelines(char *lineptr[], int nlines)
{
    int i;
    for (i=0;i<nlines;i++){
        printf("%s\n", lineptr[i]);
    }
}


/* qsort function: 按照递增的顺序对v[left]...v[right]进行排序 */
void qsort(char *v[], int left, int right)
{
    int i, last;
    void swap(char *v[], int i, int j);
    if (left >= right)
        return;
    swap(v, left, (left + right)/2);
    last = left;
    for (i = left + 1; i <= right; i++)
    {
        if (strcmp())
    }
}