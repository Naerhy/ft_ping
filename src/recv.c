#include "ft_ping.h"

static uint16_t sw16(uint16_t v)
{
	return (v << 8) | (v >> 8);
}

static int valid_type(uint8_t type)
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
					print_icmp_reply(ip, icmp, icmplen);
					info->pcktrecv++;
				}
			}
			else if (valid_type(icmp->icmp_type))
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
