#include "ft_ping.h"

Info* info = NULL;

// TODO: check if useful to keep this function
/*
static uint32_t sw32(uint32_t v)
{
	return ((v << 24) & 0xFF000000)
			| ((v << 8) & 0x00FF0000)
			| ((v >> 8) & 0x0000FF00)
			| ((v >> 24) & 0x000000FF);
}
*/

static void handle_signals(int signum)
{
	if (signum == SIGALRM)
	{
		sendping();
		alarm(1);
	}
	else
		info->close = 1;
}

static void exit_free(void)
{
	if (info->sockfd != -1)
		close(info->sockfd);
	freeaddrinfo(info->addrs);
	free(info);
}

static int init_info(void)
{
	info = malloc(sizeof(Info));
	if (!info)
		return 0;
	info->sockfd = -1;
	memset(info->ipstr, 0, sizeof(info->ipstr));
	info->pid = getpid();
	info->close = 0;
	info->pcktsent = 0;
	info->pcktrecv = 0;
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
