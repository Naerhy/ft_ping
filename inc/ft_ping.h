#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/ip_icmp.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#define ICMP_MSG_SIZE 64

typedef struct Info
{
	int sockfd;
	struct addrinfo* addrs;
	struct sockaddr_in* addr;
	char ipstr[INET_ADDRSTRLEN];
	pid_t pid;
	uint32_t seq;
	int close;
} Info;

extern Info* info;

#endif
