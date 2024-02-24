#ifndef FT_PING_H
#define FT_PING_H

#include <arpa/inet.h>
#include <errno.h>
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

#define IP_HEADER_MIN_SIZE 20
#define ICMP_MSG_SIZE 64
#define ICMP_MIN_SIZE 16

typedef struct Info
{
	int sockfd;
	uint8_t ttl;
	struct addrinfo* addrs;
	struct sockaddr_in* addr;
	char ipstr[INET_ADDRSTRLEN];
	uint16_t pid;
	int close;
	uint32_t pcktsent;
	uint32_t pcktrecv;
} Info;

typedef struct Timestamp
{
	uint64_t whole;
	uint64_t fractional;
} Timestamp;

extern Info* info;

void sendping(void);

void recvping(void);

void print_icmp_reply(struct ip* ip, struct icmp* icmp, uint16_t icmplen);
void print_icmp_error(struct ip* ip, struct icmp* icmp, uint16_t icmplen);
void print_stats(char const* hostname);

#endif
