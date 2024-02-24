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

static int exit_ping(char const* err)
{
	if (err)
		write(1, err, strlen(err));
	if (info)
	{
		if (info->sockfd != -1)
			close(info->sockfd);
		if (info->addrs)
			freeaddrinfo(info->addrs);
		free(info);
	}
	return err ? 1 : 0;
}

static int init_socket(char const* host)
{
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	if (getaddrinfo(host, NULL, &hints, &info->addrs))
		return -1;
	info->sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (info->sockfd == -1)
		return -2;
	// TODO: handle ttl flag in argv + validate max value
	if (setsockopt(info->sockfd, IPPROTO_IP, IP_TTL, &info->ttl, sizeof(info->ttl)) == -1)
		return -3;
	info->addr = (struct sockaddr_in*)info->addrs->ai_addr;
	inet_ntop(info->addrs->ai_family, &info->addr->sin_addr, info->ipstr, sizeof(info->ipstr));
	return 1;
}

static int init_info(void)
{
	info = malloc(sizeof(Info));
	if (!info)
		return 0;
	info->sockfd = -1;
	info->ttl = 64;
	info->addrs = NULL;
	memset(info->ipstr, 0, sizeof(info->ipstr));
	info->pid = getpid();
	info->close = 0;
	info->pcktsent = 0;
	info->pcktrecv = 0;
	return 1;
}

int main(int argc, char** argv)
{
	int sockinit;

	if (argc < 2)
		return exit_ping("ft_ping: missing host operand\n");
	if (!init_info())
		return exit_ping("ft_ping: unable to allocate memory\n");
	sockinit = init_socket(argv[1]);
	if (sockinit != 1)
		return exit_ping(sockinit == -1 ? "ft_ping: unknown host\n"
				: sockinit == -2 ? "ft_ping: unable to create socket\n"
				: "ft_ping: unable to set socket option\n");
	printf("PING %s (%s): 56 data bytes\n", *(argv + 1), info->ipstr);
	signal(SIGALRM, handle_signals);
	signal(SIGINT, handle_signals);
	raise(SIGALRM);
	recvping();
	if (info->close == -1)
		return exit_ping("ft_ping: cannot send or recv ICMP message\n");
	print_stats(argv[1]);
	return exit_ping(NULL);
}
