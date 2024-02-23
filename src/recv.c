#include "ft_ping.h"

static uint16_t sw16(uint16_t v)
{
	return (v << 8) | (v >> 8);
}

static double timestamp_diff(struct timeval* before)
{
	struct timeval now;
	double ms;

	// TODO: handle error
	gettimeofday(&now, NULL);
	ms = (((double)now.tv_sec * 1000) + ((double)now.tv_usec / 1000)) -
			(((double)before->tv_sec * 1000) + ((double)before->tv_usec / 1000));
	return ms;
}

static Timestamp convert_ts(double ms)
{
	Timestamp ts;

	ts.whole = (uint64_t)ms;
	ts.fractional = (uint64_t)((ms - ts.whole) * 1000);
	return ts;
}

void recvping(void)
{
	char buf[IP_MAXPACKET];
	struct sockaddr_in recvaddr;
	socklen_t recvaddrlen;
	ssize_t nbrecv;
	struct ip* ip;
	struct icmp* icmp;
	Timestamp ts;
	uint16_t icmplen;

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
			if (icmp->icmp_id != info->pid)
				continue;
			switch (icmp->icmp_type)
			{
				case ICMP_ECHOREPLY:
					ts = convert_ts(timestamp_diff((struct timeval*)icmp->icmp_data));
					printf("%u bytes from %s: icmp_seq=%u ttl=%u time=%lu,%lu ms\n", icmplen,
							info->ipstr, icmp->icmp_seq, ip->ip_ttl, ts.whole, ts.fractional);
					break;
				default:
					printf("invalid ICMP code\n");
					break;
			}
			info->pcktrecv++;
		}
		else
		{
			// TODO: handle error
			if (errno != EAGAIN || errno != EWOULDBLOCK)
				return ;
		}
	}
}
