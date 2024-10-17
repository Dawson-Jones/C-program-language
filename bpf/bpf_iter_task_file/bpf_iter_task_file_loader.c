#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <bpf/bpf.h>
#include <pthread.h>
#include "bpf_iter_task_file.skel.h"

static volatile bool exiting = false;

static void sig_handler(int sig) {
    exiting = true;
}

static void do_read_opts(struct bpf_program *prog, struct bpf_iter_attach_opts *opts) {
    struct bpf_link *link;
    char buf[16] = {};
    int iter_fd, len;

    link = bpf_program__attach_iter(prog, opts);   
    if (!link) {
        printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
        fprintf(stderr, "Failed to attach BPF program\n");
        return;
    }

    iter_fd = bpf_iter_create(bpf_link__fd(link));
    if (iter_fd < 0) {
        printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
        fprintf(stderr, "Failed to create BPF iterator\n");
        goto free_link;
    }

    // while ((len = read(iter_fd, buf, sizeof(buf))) > 0) {
    //     buf[len] = '\0';
    //     printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
    //     printf("%s\n", buf);
    // }
    while ((len = read(iter_fd, buf, sizeof(buf))) > 0)
    ;

    close(iter_fd);

free_link:
    printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
    bpf_link__destroy(link);
}

static void check_bpf_link_info(const struct bpf_program *prog)
{
	LIBBPF_OPTS(bpf_iter_attach_opts, opts);
	union bpf_iter_link_info linfo;
	struct bpf_link_info info = {};
	struct bpf_link *link;
	__u32 info_len;
	int err;

	memset(&linfo, 0, sizeof(linfo));
	linfo.task.tid = getpid();
	opts.link_info = &linfo;
	opts.link_info_len = sizeof(linfo);

	link = bpf_program__attach_iter(prog, &opts);
    if (!link) {
        fprintf(stderr, "Failed to attach BPF program\n");
        return;
    }

	info_len = sizeof(info);
	err = bpf_link_get_info_by_fd(bpf_link__fd(link), &info, &info_len);
    if (err) {
        fprintf(stderr, "Failed to get BPF link info: %d\n", err);
    }
    printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
    printf("check_task_tid: %d == pid: %d\n", info.iter.task.tid, getpid());

	bpf_link__destroy(link);
}

static pthread_mutex_t do_nothing_mutex;

static void *do_nothing_wait(void *arg) {
    pthread_mutex_lock(&do_nothing_mutex);
    printf("another thread locked\n");
    printf("another pid: %d\n", getpid());
    printf("another thread unlocked\n");
    pthread_mutex_unlock(&do_nothing_mutex);

    pthread_exit(arg);
}

int main(int argc, char const *argv[]) {
    LIBBPF_OPTS(bpf_iter_attach_opts, opts);
    struct bpf_iter_task_file *skel;
    union bpf_iter_link_info link_info;
    pthread_t thread_id;
    void *ret;
    int err;

    printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
    printf("current pid: %d\n", getpid());

    skel = bpf_iter_task_file__open_and_load();
    if (!skel) {
        fprintf(stderr, "Failed to open and load BPF skeleton\n");
        return 1;
    }

    skel->bss->tgid = getpid();

    pthread_mutex_lock(&do_nothing_mutex);

    pthread_create(&thread_id, NULL, &do_nothing_wait, NULL);

    memset(&link_info, 0, sizeof(link_info));
    link_info.task.tid = getpid();
    opts.link_info = &link_info;
    opts.link_info_len = sizeof(link_info);

    do_read_opts(skel->progs.dump_task_file, &opts);

    printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
    printf("check_count: %d, should be 0\n", skel->bss->count);
    printf("check_unique_tgid_count: %d, should be 1\n", skel->bss->unique_tgid_count);
    /*
        等于 1 的原因是 opts link_info 里面的 task.tid 被固定是当前进程的 pid
    */

    skel->bss->last_tgid = 0;
    skel->bss->count = 0;
    skel->bss->unique_tgid_count = 0;

    do_read_opts(skel->progs.dump_task_file, NULL);

    printf("%s(%s:%d)\n", __func__, __FILE__, __LINE__);
    printf("check_count: %d, should be 0\n", skel->bss->count);
    printf("check_unique_tgid_count: %d, should gt 1\n", skel->bss->unique_tgid_count);

    check_bpf_link_info(skel->progs.dump_task_file);

    pthread_mutex_unlock(&do_nothing_mutex);
    pthread_join(thread_id, &ret);

cleanup:
    bpf_iter_task_file__destroy(skel);
    return err != 0;
}
