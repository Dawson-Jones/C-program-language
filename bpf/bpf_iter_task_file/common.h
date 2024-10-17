#define ERR_MAX_LEN 1024

#define BPF_FS_MAGIC 0xcafe4a11

void p_err(const char *fmt, ...);
int mount_bpffs_for_file(const char *file_name);
