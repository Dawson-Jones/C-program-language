#ifndef MYTBF_H__
#define MYTBF_H__


typedef void mytbf_t;

// cps: 每秒传输字符个数
// burst 上限
mytbf_t *mytbf_init(int cps, int burst);

int mytbf_fetchtoken(mytbf_t *, int);

int mytbf_returntoken(mytbf_t *, int);

int mytbf_destroy(mytbf_t *);


#endif