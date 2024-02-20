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

static void sendping(void)
{
	struct icmp* icmp;
	char msg[ICMP_MSG_SIZE];
	struct sockaddr_in* addr;
	char ipstr[INET_ADDRSTRLEN];
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
	addr = (struct sockaddr_in*)info->addrs->ai_addr;
	memset(ipstr, 0, INET_ADDRSTRLEN);
	inet_ntop(info->addrs->ai_family, (void const*)(&addr->sin_addr), ipstr, INET_ADDRSTRLEN);
	printf("ft_ping: sending ICMP message to %s\n", ipstr);
	sendto(info->sockfd, (void const*)msg, ICMP_MSG_SIZE, 0, (struct sockaddr const*)addr, sizeof(struct sockaddr_storage));

	char buf[1000];
	struct sockaddr aa;
	socklen_t ss = sizeof(aa);
	memset(buf, 0, 1000);
	recvfrom(info->sockfd, buf, 1000, 0, &aa, &ss);
	printf("%s\n", buf);
}

void hdlsig(int signum)
{
	if (signum == SIGALRM)
	{
		sendping();
		alarm(1);
	}
	else if (signum == SIGINT)
		info->close = 1;
}

static int init_info(void)
{
	info = malloc(sizeof(Info));
	if (!info)
		return 0;
	info->pid = getpid();
	info->seq = 0;
	info->close = 0;
	return 1;
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
		// free info
		return 1;
	}
	info->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (info->sockfd == -1)
	{
		freeaddrinfo(info->addrs);
		printf("ft_ping: unable to create socket\n");
		// free info
		return 1;
	}
	signal(SIGALRM, hdlsig);
	raise(SIGALRM);
	while (!info->close) {}
	freeaddrinfo(info->addrs);
	free(info);
	return 0;
}
