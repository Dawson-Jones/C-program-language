#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>

#include "mytbf.h"

#define MYTBF_MAX 1024

struct mytbf_st {
	int cps;	// 每秒传输的字节数
	int token;	// 当前积攒的字节数
	int burst;	// 积攒字节数上限
	int pos;
};

static struct mytbf_st *job[MYTBF_MAX];
static int inited = 0;

static void sigalrm_handler(int s) {
	// 把这里的 1 秒涨一个令牌换成参数, 可以做到 1 个 timer
	// 控制 anytimer
	alarm(1);

	for (int i = 0; i < MYTBF_MAX; ++i) {
		if (!job[i])
			continue;

		job[i]->token += job[i]->cps;
		if (job[i]->token > job[i]->burst)
			job[i]->token = job[i]->burst;
	}
}

// 不确定对不对
static void (*sigalrm_handler_save)(int);
// static sighandler_t sigalrm_handler_save;

void module_unload() {
	signal(SIGALRM, sigalrm_handler_save);
	alarm(0);

	for (int i = 0; i < MYTBF_MAX; ++i)
		free(job[i]);
}

void module_load() {
	sigalrm_handler_save = signal(SIGALRM, sigalrm_handler);
	alarm(1);

	atexit(module_unload);
}



static int get_free_pos() {
	for (int i = 0; i < MYTBF_MAX; ++i) {
		if (!job[i]) {
			return i;
		}
	}

	return -1;
}


mytbf_t *mytbf_init(int cps, int burst) {
	struct mytbf_st *me;

	// 只能调用一次, 因为不能多次注册同一个信号, 之前的信号会被覆盖
	if (!inited) {
		module_load();
		inited = 1;
	}

	int pos = get_free_pos();
	if (pos < 0)
		return NULL;

	me = malloc(sizeof(*me));
	if (!me)
		return NULL;
	
	me->token = 0;
	me->cps = cps;
	me->burst = burst;

	job[pos] = me;
	return me;
}


static int min(int a, int b) {
	return a > b ? b : a;
}


int mytbf_fetchtoken(mytbf_t *ptr, int size) {
	int n;
	struct mytbf_st *me = ptr;

	if (size < 0)
		return -EINVAL;
	
	while (me->token <= 0)
		pause();
	
	// return me->token > size ? ({ me->token -= size; size; }) : ({ me->token = 0; me->token; });
	n = min(me->token, size);
	me->token -= n;
	return n;
}


int mytbf_returntoken(mytbf_t *ptr, int size) {
	struct mytbf_st *me = ptr;
	if (size < 0)
		return -EINVAL;
	
	me->token += size;
	if (me->token > me->burst)
		me->token = me->burst;
	
	return size;
}


int mytbf_destroy(mytbf_t *ptr) {
	struct mytbf_st *me = ptr;

	job[me->pos] = NULL;
	free(me);

	return 0;
}
