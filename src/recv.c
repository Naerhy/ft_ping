#include "ft_ping.h"

static uint16_t sw16(uint16_t v)
{
	return (v << 8) | (v >> 8);
}

static int valid_icmp_type(uint8_t type)
{
	if (type == ICMP_DEST_UNREACH || type == ICMP_SOURCE_QUENCH || type == ICMP_REDIRECT
			|| type == ICMP_TIME_EXCEEDED || type == ICMP_PARAMETERPROB)
		return 1;
	return 0;
}

void recvping(void)
{
	char buf[IP_MAXPACKET];
	struct sockaddr_in recvaddr;
	socklen_t recvaddrlen;
	ssize_t nbrecv;
	struct ip* ip;
	struct icmp* icmp;
	uint16_t icmplen;
	double ts_diff;
	double* new_list;

	while (!info->close)
	{
		recvaddrlen = sizeof(recvaddr);
		memset(buf, 0, sizeof(buf));
		nbrecv = recvfrom(info->sockfd, buf, sizeof(buf), MSG_DONTWAIT,
				(struct sockaddr*)&recvaddr, &recvaddrlen);
		if (nbrecv != -1)
		{
			ip = (struct ip*)buf;
			if (nbrecv < IP_HEADER_MIN_SIZE || ip->ip_p != IPPROTO_ICMP)
				continue;
			icmplen = sw16(ip->ip_len) - ip->ip_hl * 4;
			if (icmplen < ICMP_MIN_SIZE)
				continue;
			icmp = (struct icmp*)(buf + ip->ip_hl * 4);
			if (icmp->icmp_type == ICMP_ECHOREPLY)
			{
				if (icmp->icmp_id == info->pid)
				{
					ts_diff = timestamp_diff((struct timeval*)icmp->icmp_data);
					if (!info->pcktrecv)
					{
						info->min_ts = ts_diff;
						info->max_ts = ts_diff;
						info->avg_ts = ts_diff;
					}
					else
					{
						if (ts_diff < info->min_ts)
							info->min_ts = ts_diff;
						if (ts_diff > info->max_ts)
							info->max_ts = ts_diff;
						info->avg_ts = (info->avg_ts + ts_diff) / 2;
					}
					print_icmp_reply(ip, icmp, icmplen, ts_diff);
					info->list_ts[info->pcktrecv] = ts_diff;
					info->pcktrecv++;
					if (info->pcktrecv == info->list_max_items)
					{
						new_list = realloc(info->list_ts, info->list_max_items * 2 * sizeof(double));
						if (new_list)
							info->list_ts = new_list;
						else
							info->close = -1;
						info->list_max_items *= 2;
					}
				}
			}
			else if (valid_icmp_type(icmp->icmp_type))
			{
				if (*((uint16_t*)((uint8_t*)icmp + 8 + (ip->ip_hl * 4) + 4)) == info->pid)
					print_icmp_error(ip, icmp, icmplen);
			}
			else
				continue;
		}
		else
		{
			if (errno != EAGAIN || errno != EWOULDBLOCK)
				info->close = -1;
		}
	}
}
