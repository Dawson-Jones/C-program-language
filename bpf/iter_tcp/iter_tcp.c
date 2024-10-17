#include "vmlinux.h"
#include <bpf/bpf_helpers.h>
#include <bpf/bpf_endian.h>

#define AF_INET 2

// struct tcpconn {
//     __be32 saddr;
//     __be32 daddr;
//     __be16 sport;
//     __be16 dport;
//     __u32 seq;
//     __u32 ack_seq;
// };


SEC("iter/tcp")
int list_tcp_sock(struct bpf_iter__tcp *ctx)
{
	struct sock_common *skc = ctx->sk_common;
	// struct tcpconn t = {};
	struct tcp_sock *tp;
    __u32 seq_num;
    const struct inet_connection_sock *icsk;
    const struct inet_sock *inet;




    seq_num = ctx->meta->seq_num;
    if (seq_num == 0) {
        BPF_SEQ_PRINTF(ctx->meta->seq, "  sl  "
                        "local_address  "
                        "remmo_address  "
                        "  seq    ack_seq skc_num\n");
    }

	if (!skc)
		return 0;

	tp = bpf_skc_to_tcp_sock(skc);
	if (!tp)
		return 0;

    icsk = &tp->inet_conn;
    inet = &icsk->icsk_inet;

	if (skc->skc_state != TCP_ESTABLISHED)
		return 0;
    

    BPF_SEQ_PRINTF(ctx->meta->seq, "%4d: %08X:%5d %08X:%5d %08x %08x %5d\n",
                    seq_num, 
                    bpf_ntohl(skc->skc_rcv_saddr), bpf_ntohs(inet->inet_sport),
                    bpf_ntohl(skc->skc_daddr), bpf_ntohs(skc->skc_dport),
                    tp->snd_nxt, tp->rcv_nxt, bpf_ntohs(skc->skc_num));

	// t.saddr = skc->skc_rcv_saddr;
	// t.daddr = skc->skc_daddr;
	// t.sport = skc->skc_num;
	// t.dport = bpf_ntohs(skc->skc_dport);
	// t.seq = tp->snd_nxt;
	// t.ack_seq = tp->rcv_nxt;
	// bpf_seq_write(ctx->meta->seq, &t, sizeof(t));

	return 0;
}

#define ICSK_TIME_RETRANS	1
#define ICSK_TIME_PROBE0	3
#define ICSK_TIME_LOSS_PROBE	5
#define ICSK_TIME_REO_TIMEOUT	6

static int hlist_unhashed_lockless(const struct hlist_node *h)
{
    return !(h->pprev);
}

static int timer_pending(const struct timer_list * timer)
{
	return !hlist_unhashed_lockless(&timer->entry);
}

extern unsigned CONFIG_HZ __kconfig;

#define USER_HZ		100
#define NSEC_PER_SEC	1000000000ULL

static clock_t jiffies_to_clock_t(unsigned long x)
{
	// The implementation here tailored to a particular
	// setting of USER_HZ.
	u64 tick_nsec = (NSEC_PER_SEC + CONFIG_HZ/2) / CONFIG_HZ;
	u64 user_hz_nsec = NSEC_PER_SEC / USER_HZ;

	if ((tick_nsec % user_hz_nsec) == 0) {
		if (CONFIG_HZ < USER_HZ)
			return x * (USER_HZ / CONFIG_HZ);
		else
			return x / (CONFIG_HZ / USER_HZ);
	}
	return x * tick_nsec/user_hz_nsec;
}

static clock_t jiffies_delta_to_clock_t(long delta)
{
	if (delta <= 0)
		return 0;

	return jiffies_to_clock_t(delta);
}



static long sock_i_ino(const struct sock *sk)
{
	const struct socket *sk_socket = sk->sk_socket;
	const struct inode *inode;
	unsigned long ino;

	if (!sk_socket)
		return 0;

	inode = &container_of(sk_socket, struct socket_alloc, socket)->vfs_inode;
	bpf_probe_read_kernel(&ino, sizeof(ino), &inode->i_ino);
	return ino;
}

#define TCP_INFINITE_SSTHRESH	0x7fffffff
#define TCP_PINGPONG_THRESH	3

static bool inet_csk_in_pingpong_mode(const struct inet_connection_sock *icsk)
{
	return icsk->icsk_ack.pingpong >= TCP_PINGPONG_THRESH;
}

static bool tcp_in_initial_slowstart(const struct tcp_sock *tcp)
{
	return tcp->snd_ssthresh >= TCP_INFINITE_SSTHRESH;
}

static int dump_tcp_sock(struct seq_file *seq, struct tcp_sock *tp, uid_t uid, __u32 seq_num)
{
    const struct inet_connection_sock *icsk;
    const struct fastopen_queue *fastopenq;
    const struct inet_sock *inet;
    unsigned long timer_expires;
    const struct sock *sp;
    __u16 sport, dport;
    __be32 saddr, daddr;
    int timer_active;
    int rx_queue;
    int state;

    icsk = &tp->inet_conn;
    inet = &icsk->icsk_inet;
    sp = &inet->sk;
    fastopenq = &icsk->icsk_accept_queue.fastopenq;

    daddr = inet->sk.__sk_common.skc_daddr;
    saddr = inet->sk.__sk_common.skc_rcv_saddr;
    sport = bpf_ntohs(inet->inet_sport);
    dport = bpf_ntohs(inet->sk.__sk_common.skc_dport);

    if (icsk->icsk_pending == ICSK_TIME_RETRANS ||
        icsk->icsk_pending == ICSK_TIME_REO_TIMEOUT ||
        icsk->icsk_pending == ICSK_TIME_LOSS_PROBE) {
        timer_active = 1;
        timer_expires = icsk->icsk_timeout;
    } else if (icsk->icsk_pending == ICSK_TIME_PROBE0) {
        timer_active = 4;
        timer_expires = icsk->icsk_timeout;
    } else if (timer_pending(&sp->sk_timer)) {
        timer_active = 2;
        timer_expires = sp->sk_timer.expires;
    } else {
        timer_active = 0;
        timer_expires = bpf_jiffies64();
    }

    state = sp->__sk_common.skc_state;
    if (state == TCP_LISTEN) {
        rx_queue = sp->sk_ack_backlog;
    } else {
        // rcv_nxt（下一个预期序列号）：这个值表示TCP期望接收的下一个字节的序列号。它代表了到目前为止已经正确接收到的所有数据的"边界"。
        // copied_seq（最后一个已复制数据包的序列号）：这个值表示已经被复制到用户空间（即应用程序可以读取）的最后一个字节的序列号。
        // 差值 rcv_nxt - copied_seq：
        // 这个差值代表了已经被TCP接收但还没有被应用程序读取的数据量。
        // 换句话说，这些数据已经到达了网络层，但还在内核的接收缓冲区中等待被读取。
        rx_queue = tp->rcv_nxt - tp->copied_seq;
        if (rx_queue < 0)
            rx_queue = 0;
    }

    BPF_SEQ_PRINTF(seq, "%4d: %08X:%04X %08X:%04X ",
                    seq_num, saddr, sport, daddr, dport);

    // tp->write_seq 通常表示已经写入（发送）的最后一个字节的序列号。
    // tp->snd_una 代表最早的未确认字节的序列号。(Send Unacknowledged)
    // tp->write_seq - tp->snd_una 的差值表示已发送但尚未被确认的数据量。
    BPF_SEQ_PRINTF(seq, "%02X %08X:%08X %02X:%08lX %08X %5u %8d %lu %d ",
                    state, tp->write_seq - tp->snd_una, rx_queue, timer_active,
                    jiffies_delta_to_clock_t(timer_expires - bpf_jiffies64()),
                    icsk->icsk_retransmits, uid, icsk->icsk_probes_out,
                    sock_i_ino(sp), sp->__sk_common.skc_refcnt.refs.counter);
    BPF_SEQ_PRINTF(seq, "%pK %lu %lu %u %u %d\n",
                    tp,
                    jiffies_to_clock_t(icsk->icsk_rto), //  the retransmission timeout
                    jiffies_to_clock_t(icsk->icsk_ack.ato), //  the ACK timeout
                    (icsk->icsk_ack.quick << 1) | inet_csk_in_pingpong_mode(icsk),
                    tp->snd_cwnd,   // the current send congestion window
                    state == TCP_LISTEN ? fastopenq->max_qlen
                     : (tcp_in_initial_slowstart(tp) ? -1 : tp->snd_ssthresh)
                    // Either fastopenq->max_qlen (if the socket is in LISTEN state) 
                    // or tp->snd_ssthresh (if not in initial slow-start phase) 
                    // or -1 (if in initial slow-start phase)
                    );
    
    return 0;
}

static int dump_tw_sock(struct seq_file *seq, struct tcp_timewait_sock *ttw, uid_t uid, __u32 seq_num)
{
    struct inet_timewait_sock *tw = &ttw->tw_sk;
    __u16 sport, dport;
    __be32 saddr, daddr;
    long delta;

    delta = tw->tw_timer.expires - bpf_jiffies64();
    daddr = tw->__tw_common.skc_daddr;
    saddr = tw->__tw_common.skc_rcv_saddr;
    dport = bpf_ntohs(tw->__tw_common.skc_dport);
    sport = bpf_ntohs(tw->tw_sport);

    BPF_SEQ_PRINTF(seq, "%4d: %08X:%04X %08X:%04X ",
                    seq_num, saddr, sport, daddr, dport);
    BPF_SEQ_PRINTF(seq, "%02X %08X:%08X %02X:%08lX %08X %5d %8d %d %d %pK\n",
                    tw->tw_substate, 0, 0,
                    3, jiffies_delta_to_clock_t(delta), 0, 0, 0, 0,
                    tw->__tw_common.skc_refcnt.refs.counter, tw);

    return 0;
}

static int dump_req_sock(struct seq_file *seq, struct tcp_request_sock *treq, uid_t uid, __u32 seq_num)
{
    struct inet_request_sock *irsk = &treq->req;
    struct request_sock *req = &irsk->req;
    long ttd;

    ttd = req->rsk_timer.expires - bpf_jiffies64();
    if (ttd < 0)
        ttd = 0;

    BPF_SEQ_PRINTF(seq, "%4d: %08X:%04X %08X:%04X ",
                    seq_num, 
                    // ir_num 字段通常是用来存储本地端口号的。在 Linux 内核网络栈中，skc_num （这里对应 ir_num）通常用于表示本地端口号。
                    // 使用 ir_num 而不是显式的 "source port" 可能是因为在这个阶段，这个值可能还在变化或者还没有最终确定。
                    req->__req_common.skc_rcv_saddr, req->__req_common.skc_num,
                    req->__req_common.skc_daddr, bpf_ntohs(req->__req_common.skc_dport));
    BPF_SEQ_PRINTF(seq, "%02X %08X:%08X %02X:%08lX %08X %5d %8d %d %d %pK\n",
                    TCP_SYN_RECV, 0, 0, 1, jiffies_delta_to_clock_t(ttd), 
                    req->num_timeout, uid, 0, 0, 0, req);

    return 0;
}

SEC("iter/tcp")
int dump_tcp4(struct bpf_iter__tcp *ctx)
{
    struct sock_common *sk_common = ctx->sk_common;
    struct seq_file *seq = ctx->meta->seq;
    struct tcp_timewait_sock *tw;
    struct tcp_request_sock *req;
    struct tcp_sock *tp;
    uid_t uid = ctx->uid;
    __u32 seq_num;

    if (sk_common == (void *) 0)
        return 0;
    
    // 在输出或日志记录中，这个序列号可以用来区分不同的TCP连接。
    // 它有助于保持输出的顺序，特别是在处理多个连接时。
    seq_num = ctx->meta->seq_num;
    if (seq_num == 0) {
        		BPF_SEQ_PRINTF(seq, "  sl  "
				    "local_address "
				    "rem_address   "
				    "st tx_queue rx_queue tr tm->when retrnsmt"
				    "   uid  timeout inode\n");
    }

    if (sk_common->skc_family != AF_INET)
        return 0;

    tp = bpf_skc_to_tcp_sock(sk_common);
    if (tp) {
        // BPF_SEQ_PRINTF(seq, "TCP socket: %pK\n", tp);
        return dump_tcp_sock(seq, tp, uid, seq_num);
    }
    
    tw = bpf_skc_to_tcp_timewait_sock(sk_common);
    if (tw) {
        // BPF_SEQ_PRINTF(seq, "TCP timewait: %pK\n", tw);
        return dump_tw_sock(seq, tw, uid, seq_num);
    }
    
    req = bpf_skc_to_tcp_request_sock(sk_common);
    if (req) {
        // BPF_SEQ_PRINTF(seq, "TCP request: %pK\n", req);
        return dump_req_sock(seq, req, uid, seq_num);
    }

    return 0;
}

char _license[] SEC("license") = "Dual BSD/GPL";