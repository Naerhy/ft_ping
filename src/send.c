#include "ft_ping.h"

static uint16_t generate_checksum(uint16_t* x)
{
	int cksum;

	cksum = 0;
	for (size_t i = 0; i < ICMP_MSG_SIZE / 2; i++)
		cksum += x[i];
	cksum = (cksum >> 16) + (cksum & 0xffff);
	cksum += (cksum >> 16);
	return ~cksum;
}

// TODO: add info->close condition to prevent sending a last ping on exit?
// TODO: verify that sendto return is 64 and decide what to do if error
void sendping(void)
{
	struct icmp* icmp;
	char msg[ICMP_MSG_SIZE];
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1)
		info->close = -1;
	else
	{
		memset(msg, 0, ICMP_MSG_SIZE);
		icmp = (struct icmp*)msg;
		icmp->icmp_type = ICMP_ECHO;
		icmp->icmp_code = 0;
		icmp->icmp_cksum = 0;
		icmp->icmp_id = info->pid;
		icmp->icmp_seq = info->pcktsent;
		memcpy(icmp->icmp_data, &tv, sizeof(tv));
		icmp->icmp_cksum = generate_checksum((uint16_t*)icmp);
		if (sendto(info->sockfd, msg, ICMP_MSG_SIZE, 0, (struct sockaddr*)info->addr,
				sizeof(struct sockaddr_in)) == -1)
			info->close = -1;
		info->pcktsent++;
	}
}
