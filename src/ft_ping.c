#include "ft_ping.h"

Info* info = NULL;

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

// TODO: decide what to do in case of error
static void sendping(void)
{
	struct icmp* icmp;
	char msg[ICMP_MSG_SIZE];
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1)
		return ;
	memset(msg, 0, ICMP_MSG_SIZE);
	icmp = (struct icmp*)msg;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_id = info->pid;
	icmp->icmp_seq = info->seq;
	*((time_t*)icmp->icmp_data) = tv.tv_sec;
	icmp->icmp_cksum = generate_checksum((uint16_t*)icmp);
	if (sendto(info->sockfd, msg, ICMP_MSG_SIZE, 0, (struct sockaddr*)info->addr,
			sizeof(struct sockaddr_storage)) == -1)
		return ;
	info->pcktsent++;
}

static void recvping(void)
{
	char buf[200];
	struct sockaddr_in recvaddr;
	socklen_t recvaddrlen;

	while (!info->close)
	{
		recvaddrlen = sizeof(recvaddr);
		memset(buf, 0, sizeof(buf));
		if (recvfrom(info->sockfd, buf, sizeof(buf), MSG_DONTWAIT, (struct sockaddr*)&recvaddr, &recvaddrlen) != -1)
		{
			printf("received message\n");
			info->pcktrecv++;
		}
	}
}

static void handle_signals(int signum)
{
	if (signum == SIGALRM)
	{
		sendping();
		alarm(1);
	}
	else if (signum == SIGINT)
	{
		info->close = 1;
	}
}

static int init_info(void)
{
	info = malloc(sizeof(Info));
	if (!info)
		return 0;
	info->sockfd = -1;
	memset(info->ipstr, 0, sizeof(info->ipstr));
	info->pid = getpid();
	info->seq = 0;
	info->close = 0;
	info->pcktsent = 0;
	info->pcktrecv = 0;
	return 1;
}

static void exit_free(void)
{
	if (info->sockfd != -1)
		close(info->sockfd);
	freeaddrinfo(info->addrs);
	free(info);
}

static void print_stats(char const* hostname)
{
	uint32_t pcktloss;

	pcktloss = 100 - (((double)info->pcktrecv / (double)info->pcktsent) * 100);
	printf("--- %s ping statistics ---\n", hostname);
	printf("%u packets transmitted, %u packets received, %u%% packet loss\n", info->pcktsent,
			info->pcktrecv, pcktloss);
	// TODO: calculate those numbers
	printf("round-trip min/avg/max/stddev = ...\n");
}

int main(int argc, char** argv)
{
	struct addrinfo hints;

	if (argc < 2)
	{
		printf("ft_ping: missing host operand\n");
		return 1;
	}
	if (!init_info())
	{
		printf("ft_ping: unable to allocate memory\n");
		return 1;
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	if (getaddrinfo(*(argv + 1), NULL, &hints, &info->addrs))
	{
		printf("ft_ping: unknown host\n");
		exit_free();
		return 1;
	}
	info->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (info->sockfd == -1)
	{
		printf("ft_ping: unable to create socket\n");
		exit_free();
		return 1;
	}
	info->addr = (struct sockaddr_in*)info->addrs->ai_addr;
	inet_ntop(info->addrs->ai_family, &info->addr->sin_addr, info->ipstr, sizeof(info->ipstr));
	printf("PING %s (%s): 56 data bytes\n", *(argv + 1), info->ipstr);
	signal(SIGALRM, handle_signals);
	signal(SIGINT, handle_signals);
	// invert raise and recvping to avoid missing first recv?
	raise(SIGALRM);
	recvping();
	print_stats(*(argv + 1));
	exit_free();
	return 0;
}
