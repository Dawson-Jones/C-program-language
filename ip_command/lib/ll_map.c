/*
 * ll_map.c
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Alexey Kuznetsov, <kuznet@ms2.inr.ac.ru>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <net/if.h>

#include <linux/if_link.h>
#include <linux/rtnetlink.h>


int ll_remember_index(struct nlmsghdr *n, void *arg)
{
	struct ifinfomsg *ifi = NLMSG_DATA(n);
	struct ll_cache *im;
	struct rtattr *tb[IFLA_MAX+1];

	if (n->nlmsg_type != RTM_NEWLINK && n->nlmsg_type != RTM_DELLINK)
		return 0;

	if (n->nlmsg_len < NLMSG_LENGTH(sizeof(*ifi)))
		return -1;

	im = ll_get_by_index(ifi->ifi_index);
	if (n->nlmsg_type == RTM_DELLINK) {
		if (im)
			ll_entries_destroy(im);
		return 0;
	}

	parse_rtattr_flags(tb, IFLA_MAX, IFLA_RTA(ifi),
			   IFLA_PAYLOAD(n), NLA_F_NESTED);
	if (im)
		ll_entries_update(im, ifi, tb);
	else
		ll_entries_create(ifi, tb);
	return 0;
}