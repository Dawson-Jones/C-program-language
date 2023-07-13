#include <sys/statfs.h>
#include <string.h>
#include <stdio.h>

#define BPF_FS_MAGIC 0xcafe4a11

int main() {
    char *bpf_sys_path = "/sys/fs/bpf";

    struct statfs fs_info;
    memset(&fs_info, 0, sizeof(fs_info));

    if (statfs(bpf_sys_path, &fs_info) == -1) {
        perror("statfs");
        return 1;
    }

    if (fs_info.f_type == BPF_FS_MAGIC) {
        printf("%s is already mounted with bpf filesystem\n", bpf_sys_path);
    } else {
        printf("mounting bpf filesystem to %s\n", bpf_sys_path);

        // if (mount("none", bpf_sys_path, "bpf", 0, NULL) != 0) {
        //     perror("mount");
        //     return 1;
        // }

        // printf("%s is mounted with bpf filesystem\n", bpf_sys_path);
    }

    return 0;
}