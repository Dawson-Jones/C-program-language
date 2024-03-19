#define MNL_ALIGNTO		4
#define MNL_ALIGN(len)		(((len)+MNL_ALIGNTO-1) & ~(MNL_ALIGNTO-1))
#define MNL_NLMSG_HDRLEN	MNL_ALIGN(sizeof(struct nlmsghdr))




int addattr32(struct nlmsghdr *n, int maxlen, int type, __u32 data);
int af_bit_len(int af);
int rtm_get_table(struct rtmsg *r, struct rtattr **tb);