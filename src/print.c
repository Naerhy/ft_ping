#include "ft_ping.h"

void print_reply(void)
{
}

void print_stats(char const* hostname)
{
	uint32_t pcktloss;

	pcktloss = 100 - (((double)info->pcktrecv / (double)info->pcktsent) * 100);
	printf("--- %s ping statistics ---\n", hostname);
	printf("%u packets transmitted, %u packets received, %u%% packet loss\n", info->pcktsent,
			info->pcktrecv, pcktloss);
	// TODO: calculate those numbers
	printf("round-trip min/avg/max/stddev = ...\n");
}
