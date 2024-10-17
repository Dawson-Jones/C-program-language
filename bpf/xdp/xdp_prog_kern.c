// #include <linux/bpf.h>
#include "vmlinux.h"
#include <bpf/bpf_helpers.h>

SEC("xdp")
int  xdp_drop_func(struct xdp_md *ctx)
{
    bpf_printk("xdp ---------\n");
	return XDP_PASS;
}

char _license[] SEC("license") = "GPL";