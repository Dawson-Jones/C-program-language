#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/time.h>

#include "mytbf.h"

#define MYTBF_MAX 1024

// 使用另一个进程, 给当前进程发信号
// while true ; do kill -ALRM <pid>; done

struct mytbf_st {
	int cps;	// 每秒传输的字节数
	int token;	// 当前积攒的字节数
	int burst;	// 积攒字节数上限
	int pos;
};

static struct mytbf_st *job[MYTBF_MAX];
static int inited = 0;
static struct sigaction alrm_sa_save;

static void alrm_action(int s, siginfo_t *sig_info, void *unused) {
	if (sig_info->si_code != SI_KERNEL)
		return;

	for (int i = 0; i < MYTBF_MAX; ++i) {
		if (!job[i])
			continue;

		job[i]->token += job[i]->cps;
		if (job[i]->token > job[i]->burst)
			job[i]->token = job[i]->burst;
	}
}


void module_unload() {
	struct itimerval itv;
	
	itv.it_interval.tv_sec = 0;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 0;
	itv.it_value.tv_usec = 0;

	sigaction(SIGALRM, &alrm_sa_save, NULL);
	setitimer(ITIMER_REAL, &itv, NULL);

	for (int i = 0; i < MYTBF_MAX; ++i)
		free(job[i]);
}

void module_load() {
	struct sigaction sa;
	struct itimerval itv;
	
	sa.sa_sigaction = alrm_action;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_SIGINFO;
	sigaction(SIGALRM, &sa, &alrm_sa_save);

	itv.it_interval.tv_sec = 1;
	itv.it_interval.tv_usec = 0;
	itv.it_value.tv_sec = 1;
	itv.it_value.tv_usec = 0;
	setitimer(ITIMER_REAL, &itv, NULL);

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
