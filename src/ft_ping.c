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

static void send_ping(int sockfd, struct addrinfo* addrs)
{
	struct icmp* icmp;
	char msg[ICMP_MSG_SIZE];
	struct sockaddr_in* addr;
	char ipstr[INET_ADDRSTRLEN];
	struct timeval tv;

	// handle error cases for gettimeofday
	gettimeofday(&tv, NULL);
	memset(msg, 0, ICMP_MSG_SIZE);
	icmp = (struct icmp*)msg;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_id = getpid();
	icmp->icmp_seq = 0;
	*((time_t*)icmp->icmp_data) = tv.tv_sec;
	icmp->icmp_cksum = generate_checksum((uint16_t*)icmp);
	addr = (struct sockaddr_in*)addrs->ai_addr;
	memset(ipstr, 0, INET_ADDRSTRLEN);
	inet_ntop(addrs->ai_family, (void const*)(&addr->sin_addr), ipstr, INET_ADDRSTRLEN);
	printf("ft_ping: sending ICMP message to %s\n", ipstr);
	sendto(sockfd, (void const*)msg, ICMP_MSG_SIZE, 0, (struct sockaddr const*)addr, sizeof(struct sockaddr_storage));

	char buf[1000];
	struct sockaddr aa;
	socklen_t ss = sizeof(aa);
	memset(buf, 0, 1000);
	recvfrom(sockfd, buf, 1000, 0, &aa, &ss);
	printf("%s\n", buf);
}

int main(int argc, char** argv)
{
	struct addrinfo hints;
	struct addrinfo* addrs;
	int sockfd;

	if (argc < 2)
	{
		printf("ft_ping: missing host operand\n");
		return 1;
	}
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	if (getaddrinfo(*(argv + 1), NULL, &hints, &addrs))
	{
		printf("ft_ping: unknown host\n");
		return 1;
	}
	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (sockfd == -1)
	{
		freeaddrinfo(addrs);
		printf("ft_ping: unable to create socket\n");
		return 1;
	}
	send_ping(sockfd, addrs);
	freeaddrinfo(addrs);
	return 0;
}
