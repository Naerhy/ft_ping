#include "ft_ping.h"

int main(int argc, char** argv)
{
	struct addrinfo hints;
	struct addrinfo* addrs;
	struct sockaddr_in* addr;
	char ipstr[INET_ADDRSTRLEN];

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
	for (struct addrinfo* p = addrs; p; p = p->ai_next)
	{
		addr = (struct sockaddr_in*)p->ai_addr;
		inet_ntop(p->ai_family, (void const*)(&addr->sin_addr), ipstr, INET_ADDRSTRLEN);
		printf("%s\n", ipstr);
	}
	freeaddrinfo(addrs);
	/*
	int sockfd;

	sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (setsockopt(sockfd, IPPROTO_IP, IP_HDRINCL, &optvalue, sizeof(optvalue)))
	*/
	return 0;
}
