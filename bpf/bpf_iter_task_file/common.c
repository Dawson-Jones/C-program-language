#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <libgen.h>
#include <errno.h>

#include <sys/mount.h>
#include <sys/vfs.h>


#include "common.h"

void p_err(const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);
		fprintf(stderr, "Error: ");
		vfprintf(stderr, fmt, ap);
		fprintf(stderr, "\n");
	va_end(ap);
}

static bool is_bpffs(const char *path) {
    struct statfs st_fs;

    if (statfs(path, &st_fs) < 0)
        return false;

    return (unsigned long) st_fs.f_type == BPF_FS_MAGIC;
}

static int mnt_fs(const char *target, const char *type, char *buff, size_t buff_len) {
    bool bind_done = false;

    while(mount("", target, "none", MS_PRIVATE | MS_REC, NULL)) {
        if (errno == EINVAL && !bind_done) {
            snprintf(buff, buff_len,
                "mount --make-private %s failed: %s",
                target, strerror(errno));
            return -1;
        }

        if (mount(target, target, "none", MS_BIND, NULL)) {
            snprintf(buff, buff_len,
                "mount --bind %s %s failed: %s",
                target, target, strerror(errno));
            return -1;
        }

        bind_done = true;
    }

    if (mount(type, target, type, 0, "mode=0700")) {
        snprintf(buff, buff_len,
            "mount -t %s %s %s failed: %s",
            type, type, target, strerror(errno));
        return -1;
    }

    return 0;
}

int mount_bpffs_for_file(const char *file_name) {
    char err_str[ERR_MAX_LEN];
    char *temp_name;
    char *dir;
    int err = 0;

    if (access(file_name, F_OK) != -1) {
        p_err("can't pin BPF object: path '%s' already exists", file_name);
        return -1;
    }

    temp_name = strdup(file_name);
    if (!temp_name) {
        p_err("mem alloc failed");
        return -1;
    }

    dir = dirname(temp_name);

    if (is_bpffs(dir)) // nothing to do if already mounted
        goto out_free;

    if (access(dir, F_OK) == -1) {
        p_err("can't pin BPF object: path '%s' doesn't exist", dir);
        err = -1;
        goto out_free;
    }

    err = mnt_fs(dir, "bpf", err_str, ERR_MAX_LEN);
    if (err) {
        err_str[ERR_MAX_LEN - 1] = '\0';
        p_err("can't mount BPF file system to pin the object: '%s': %s", file_name, err_str);
    }

out_free:
    free(temp_name);
    return err;
}