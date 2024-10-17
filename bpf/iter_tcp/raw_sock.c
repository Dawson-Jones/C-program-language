#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <linux/tcp.h>

struct iphdr {
	uint8_t ihl: 4;
	uint8_t version: 4;
	uint8_t tos;
	uint16_t tot_len;
	uint16_t id;
	uint16_t frag_off;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t check;
	uint32_t saddr;
	uint32_t daddr;
};

// struct tcphdr {
// 	uint16_t source;
// 	uint16_t dest;
// 	uint32_t seq;
// 	uint32_t ack_seq;
// 	uint16_t res1: 4;
// 	uint16_t doff: 4;
// 	uint16_t fin: 1;
// 	uint16_t syn: 1;
// 	uint16_t rst: 1;
// 	uint16_t psh: 1;
// 	uint16_t ack: 1;
// 	uint16_t urg: 1;
// 	uint16_t ece: 1;
// 	uint16_t cwr: 1;
// 	uint16_t window;
// 	uint16_t check;
// 	uint16_t urg_ptr;
// };

struct tcp4_pseudohdr {
	uint32_t saddr;
	uint32_t daddr;
	uint8_t pad;
	uint8_t protocol;
	uint16_t len;
};

static uint16_t checksum(const uint8_t *pkt, int len)
{
	uint32_t csum = 0;
	int i;

	for (i = 0; i < len - 1; i += 2)
		csum += *(uint16_t *)&pkt[i];

	if (len & 1)
		csum += pkt[i];

	while (csum > 0xffff)
		csum = (csum & 0xffff) + (csum >> 16);

	return ~csum;
}

static int build_packet(struct sockaddr_in *saddr, struct sockaddr_in *daddr,
			uint8_t **pkt, int *len, unsigned int seq_number, unsigned int ack_seq)
{
	uint8_t data[sizeof(struct tcp4_pseudohdr) + sizeof(struct tcphdr)];
	struct tcp4_pseudohdr *pseudo_header;
	struct tcphdr *tcp_header;
	struct iphdr *ip_header;
	uint8_t *buf;

	*len = sizeof(struct iphdr) + sizeof(struct tcphdr);

	buf = calloc(*len, sizeof(uint8_t));
	if (!buf) {
		perror("calloc");
		return -1;
	}

	ip_header = (struct iphdr *)buf;
	tcp_header = (struct tcphdr *)(buf + sizeof(struct iphdr));

	ip_header->ihl = 5;
	ip_header->version = 4;
	ip_header->tos = 0;
	ip_header->tot_len = *len;
	ip_header->id = 0xeBaF;
	ip_header->frag_off = 0;
	ip_header->ttl = 0xff;
	ip_header->protocol = IPPROTO_TCP;
	ip_header->check = 0;
	ip_header->saddr = saddr->sin_addr.s_addr;
	ip_header->daddr = daddr->sin_addr.s_addr;

	tcp_header->source = saddr->sin_port;
	tcp_header->dest = daddr->sin_port;
	tcp_header->seq = htonl(seq_number);
	tcp_header->ack_seq = htonl(ack_seq);
	tcp_header->res1 = 0;
	tcp_header->doff = 5;
	tcp_header->fin = 0;
	tcp_header->syn = 0;
	tcp_header->rst = 1;
	tcp_header->psh = 0;
	tcp_header->ack = 0;
	tcp_header->urg = 0;
	tcp_header->ece = 0;
	tcp_header->cwr = 0;
	tcp_header->window = htons(0xCafe);
	tcp_header->check = 0;
	tcp_header->urg_ptr = 0;

	memset(data, 0, sizeof(data));
	pseudo_header = (struct tcp4_pseudohdr *)data;
	pseudo_header->saddr = saddr->sin_addr.s_addr;
	pseudo_header->daddr = daddr->sin_addr.s_addr;
	pseudo_header->pad = 0;
	pseudo_header->protocol = IPPROTO_TCP;
	pseudo_header->len = htons(sizeof(struct tcphdr));
	memcpy(data + sizeof(struct tcp4_pseudohdr), tcp_header, sizeof(struct tcphdr));

	tcp_header->check = checksum((void *)data, sizeof(data));
	ip_header->check = checksum((void *)ip_header, sizeof(struct iphdr));

	*pkt = buf;
	return 0;
}

int main(int argc, char const *argv[]) {
	const int one = 1, flags = 0;
	int raw_sock_fd, ret = -1, len = 0;
	struct sockaddr_in saddr, daddr;
	uint8_t *pkt = NULL;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s <port> <seq number> [<ack seq>]\n", argv[0]);
		return -1;
	}

	int port = atoi(argv[1]);
	int seq_number = atoi(argv[2]);
	int ack_seq = 0;
	if (argc > 3)
		ack_seq = atoi(argv[3]);
	printf("port: %d, seq number: %u ack seq: %u\n", port, seq_number, ack_seq);

	raw_sock_fd = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
	if (raw_sock_fd < 0) {
		perror("socket");
		return -1;
	}

	ret = setsockopt(raw_sock_fd, IPPROTO_IP, IP_HDRINCL, &one, sizeof(one));
	if (ret < 0) {
		perror("setsockopt");
		goto cleanup;
	}

	memset(&saddr, 0, sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(5046);
	ret = inet_pton(AF_INET, "9.134.112.138", &saddr.sin_addr);
	if (ret != 1) {
		perror("inet_pton");
		goto cleanup;
	}

	memset(&daddr, 0, sizeof(daddr));
	daddr.sin_family = AF_INET;
	daddr.sin_port = htons(port);
	ret = inet_pton(AF_INET, "10.211.55.3", &daddr.sin_addr);
	if (ret != 1) {
		perror("inet_pton");
		goto cleanup;
	}

	ret = build_packet(&saddr, &daddr, &pkt, &len, seq_number, ack_seq);
	if (ret < 0)
		goto cleanup;
	
	for (int i = 0; i < len; i++) {
		printf("%02x, ", pkt[i]);
	}
	printf("\n");

	ret = sendto(raw_sock_fd, pkt, len, flags,
	// ret = sendto(raw_sock_fd, pkt + sizeof(struct iphdr), len - sizeof(struct iphdr), flags,
		     (struct sockaddr *)&daddr, sizeof(struct sockaddr));
	if (ret < 0) {
		perror("sendto");
		goto cleanup;
	}

cleanup:
	free(pkt);
	close(raw_sock_fd);
	return ret <= 0;
}