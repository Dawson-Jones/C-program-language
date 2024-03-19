#include <stdio.h>
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <net/if.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <linux/if_link.h>
#include <bpf/bpf.h>
#include <bpf/libbpf.h>
#include <linux/bpf.h>
#define PCAP_DONT_INCLUDE_PCAP_BPF_H
#include <pcap/pcap.h>
#include <pcap/dlt.h>
#include <time.h>

const char *pin_dir = "/sys/fs/bpf";

pcap_t *pcap;
pcap_dumper_t *dumper;
struct perf_buffer *pb;

int open_pcap_file(const char *filename) {
    char errbuf[PCAP_ERRBUF_SIZE];

    pcap = pcap_open_dead(DLT_RAW, 65535);
    if (!pcap) {
        fprintf(stderr, "Failed to open pcap_t\n");
        return -1;
    }

    dumper = pcap_dump_open(pcap, filename);
    if (!dumper) {
        fprintf(stderr, "Failed to open pcap_dumper_t\n");
        return -1;
    }

    return 0;
}

void write_packet_to_pcap_file(const void *data, size_t size)
{
    struct pcap_pkthdr header;

    header.ts.tv_sec = time(NULL);
    header.ts.tv_usec = 0;
    header.caplen = size;
    header.len = size;

    pcap_dump((unsigned char *) dumper, &header, data);
}

void close_pcap_file()
{
    pcap_dump_close(dumper);
    pcap_close(pcap);
}


void print_bpf_output(void *ctx, int cpu, void *data, __u32 size)
{
    struct {
    __u16 pkt_len;
    __u8 pkt_data[0];
    } __attribute__((packed)) *e = data;

    printf("pkt len: %-5d\n", e->pkt_len);

    int i = 0;
    while (i < e->pkt_len) {
        printf("%02x ", e->pkt_data[i]);

        if (!(++i % 16))
            printf("\n");
    }

    printf("\n");

    write_packet_to_pcap_file(e->pkt_data, e->pkt_len);
}

static void sig_handler(int signo)
{
    close_pcap_file();
	perf_buffer__free(pb);
	exit(0);
}

int main(int argc, const char *argv[]) {
    int err;
    const char *file_path = argv[1];
    const char *pcap_file = argv[2];

    open_pcap_file(pcap_file);

    int fd = bpf_obj_get(file_path);
    if (fd < 0) {
        perror("bpf_obj_get");
        exit(EXIT_FAILURE);
    }

    pb = perf_buffer__new(fd, 8, print_bpf_output, NULL, NULL, NULL);
    err = libbpf_get_error(pb);
    if (err) {
        perror("perf_buffer__new");
        exit(EXIT_FAILURE);
    }

    signal(SIGINT, sig_handler);
    signal(SIGHUP, sig_handler);
    signal(SIGTERM, sig_handler);

    while (perf_buffer__poll(pb, 1000) >= 0) { }
}