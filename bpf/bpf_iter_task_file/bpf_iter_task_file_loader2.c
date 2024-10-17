#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <bpf/libbpf.h>
#include <errno.h>

#include "common.h"


int main(int argc, char const *argv[]) {
    // DECLARE_LIBBPF_OPTS(bpf_iter_attach_opts, iter_opts);

    int err = -1;
    int map_fd = -1;

    struct bpf_object *obj;
    struct bpf_program *prog;
    struct bpf_link *link;
    union bpf_iter_link_info link_info;
    

    if (argc < 3) {
        fprintf(stderr, "Usage: %s <obj> <pin-path> ...\n", argv[0]);
        return err;
    }

    const char *objfile = argv[1];
    const char *pinpath = argv[2];

    obj = bpf_object__open(objfile);
    if (!obj) {
        err = -errno;
        p_err("bpf_object__open");
        goto close_map_fd; 
    }

    err = bpf_object__load(obj);
    if (err) {
        p_err("bpf_object__load %s", objfile);
        goto close_obj;
    }

    prog = bpf_object__next_program(obj, NULL);
    if (!prog) {
        err = -errno;
        p_err("can't find bpf program in objfile %s", objfile);
    }

    link = bpf_program__attach_iter(prog, NULL);
    if (!link) {
        err = -errno;
        p_err("attach_iter failed for program %s", bpf_program__name(prog));
        goto close_obj;
    }

    err = mount_bpffs_for_file(pinpath);
    if (err) {
        goto close_link;
    }

    err = bpf_link__pin(link, pinpath);
    if (err) {
        p_err("pin_iter failed for program %s to path %s", bpf_program__name(prog));
        goto close_link;
    }

close_link:
    bpf_link__destroy(link);
close_obj:
    bpf_object__close(obj);
close_map_fd:
    if (map_fd >= 0)
        close(map_fd);
    return err;
}
