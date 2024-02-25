#include "ft_ping.h"

Info* info = NULL;

static void handle_signals(int signum)
{
	if (signum == SIGALRM)
	{
		sendping();
		alarm(info->interval);
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
		if (info->flags & HELP)
			print_help();
		if (info->sockfd != -1)
			close(info->sockfd);
		if (info->addrs)
			freeaddrinfo(info->addrs);
		if (info->list_ts)
			free(info->list_ts);
		free(info);
	}
	return err ? 1 : 0;
}

static int init_socket(void)
{
	struct addrinfo hints;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_RAW;
	if (getaddrinfo(info->host, NULL, &hints, &info->addrs))
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

static int parse_args(int argc, char** argv)
{
	for (int i = 1; i < argc; i++)
	{
		if (!strcmp(argv[i], "-?"))
			info->flags |= HELP;
		else if (!strcmp(argv[i], "-v"))
			info->flags |= VERBOSE;
		else if (!strcmp(argv[i], "-c"))
		{
			if (!argv[i + 1])
				return 0;
			info->count = strtoul(argv[i + 1], NULL, 10);
			if (!info->count)
				return 0;
			i++;
		}
		else if (!strcmp(argv[i], "-i"))
		{
			if (!argv[i + 1])
				return 0;
			info->interval = strtoul(argv[i + 1], NULL, 10);
			if (!info->interval)
				return 0;
			i++;
		}
		else if (!strncmp(argv[i], "--ttl=", 6))
		{
			if (!strlen(argv[i] + 6))
				return 0;
			info->ttl = strtoul(argv[i] + 6, NULL, 10);
			if (!info->ttl || info->ttl > 255)
				return 0;
		}
		else
		{
			if (!info->host)
				info->host = argv[i];
		}
	}
	return 1;
}

static int init_info(void)
{
	info = malloc(sizeof(Info));
	if (!info)
		return 0;
	info->sockfd = -1;
	info->flags = 0;
	info->count = 0;
	info->interval = 1;
	info->ttl = 64;
	info->host = NULL;
	info->addrs = NULL;
	memset(info->ipstr, 0, sizeof(info->ipstr));
	info->pid = getpid();
	info->close = 0;
	info->pcktsent = 0;
	info->pcktrecv = 0;
	info->list_ts = malloc(sizeof(double) * LIST_SIZE_INIT);
	if (!info->list_ts)
		return 0;
	info->list_max_items = LIST_SIZE_INIT;
	return 1;
}

int main(int argc, char** argv)
{
	int sockinit;

	if (!init_info())
		return exit_ping("ft_ping: unable to allocate memory\n");
	if (!parse_args(argc, argv))
		return exit_ping("ft_ping: invalid argument\n");
	if (info->flags & HELP)
		return exit_ping(NULL);
	if (!info->host)
		return exit_ping("ft_ping: missing host operand\n");
	sockinit = init_socket();
	if (sockinit != 1)
		return exit_ping(sockinit == -1 ? "ft_ping: unknown host\n"
				: sockinit == -2 ? "ft_ping: unable to create socket\n"
				: "ft_ping: unable to set socket option\n");
	// TODO: remove 56 data bytes > dynamically set value
	if (info->flags & VERBOSE)
		printf("PING %s (%s): 56 data bytes, id %#x = %d\n", info->host, info->ipstr, info->pid, info->pid);
	else
		printf("PING %s (%s): 56 data bytes\n", info->host, info->ipstr);
	signal(SIGALRM, handle_signals);
	signal(SIGINT, handle_signals);
	raise(SIGALRM);
	recvping();
	if (info->close == -1)
		return exit_ping("ft_ping: cannot send or recv ICMP message\n");
	print_stats(argv[1]);
	return exit_ping(NULL);
}
