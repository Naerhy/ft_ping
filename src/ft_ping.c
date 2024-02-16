#include "ft_ping.h"

void send_ping(int sockfd, struct addrinfo* addrs)
{
	struct icmp* icmp;
	char msg[64];
	int* ts;
	struct sockaddr_in* addr;
	char ipstr[INET_ADDRSTRLEN];

	memset(msg, 0, 64);
	icmp = (struct icmp*)msg;
	icmp->icmp_type = ICMP_ECHO;
	icmp->icmp_code = 0;
	icmp->icmp_cksum = 0;
	icmp->icmp_id = 123;
	icmp->icmp_seq = 10;
	ts = (int*)icmp->icmp_data;
	*ts = 1708102699;
	addr = (struct sockaddr_in*)addrs->ai_addr;
	memset(ipstr, 0, INET_ADDRSTRLEN);
	inet_ntop(addrs->ai_family, (void const*)(&addr->sin_addr), ipstr, INET_ADDRSTRLEN);
	printf("ft_ping: sending ICMP message to %s\n", ipstr);
	sendto(sockfd, (void const*)msg, 64, 0, (struct sockaddr const*)addr, sizeof(struct sockaddr_storage));
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
