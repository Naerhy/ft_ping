#include "ft_ping.h"

void print_help(void)
{
	printf("Usage: ping [OPTION...] HOST\n");
	printf("Send ICMP ECHO_REQUEST packets to network host.\n\n");
	printf("Options:\n");
	printf("  -c NUMBER		stop after sending NUMBER packets\n");
	printf("  -i NUMBER		wait NUMBER seconds between sending each packet\n");
	printf("  --ttl=N		specify N as time-to-live\n");
	printf("  -v			verbose output\n");
	printf("  -?			display this help list\n");
}

static char const* icmp_type_error(uint8_t type)
{
	if (type == ICMP_DEST_UNREACH)
		return "Destination unreachable";
	else if (type == ICMP_SOURCE_QUENCH)
		return "Source quench";
	else if (type == ICMP_REDIRECT)
		return "Redirect";
	else if (type == ICMP_TIME_EXCEEDED)
		return "Time exceeded";
	else
		return "Parameter problem";
}

void print_icmp_reply(struct ip* ip, struct icmp* icmp, uint16_t icmplen, double ts_diff)
{
	Timestamp ts;

	ts = convert_ts(ts_diff);
	printf("%u bytes from %s: icmp_seq=%u ttl=%u time=%lu,%lu ms\n", icmplen,
			info->ipstr, icmp->icmp_seq, ip->ip_ttl, ts.whole, ts.fractional);
}

// TODO: check if handle errors for getnameinfo and inet_ntop or not
void print_icmp_error(struct ip* ip, struct icmp* icmp, uint16_t icmplen)
{
	struct sockaddr_in src;
	char ipstr[INET_ADDRSTRLEN];
	char hostname[200];

	memset(ipstr, 0, sizeof(ip));
	memset(hostname, 0, sizeof(hostname));
	inet_ntop(AF_INET, &ip->ip_src, ipstr, sizeof(ipstr));
	src.sin_family = AF_INET;
	src.sin_addr = ip->ip_src;
	memset(src.sin_zero, 0, sizeof(src.sin_zero));
	getnameinfo((struct sockaddr*)&src, sizeof(src), hostname, sizeof(hostname), NULL, 0, 0);
	if (info->flags & VERBOSE)
		printf("%u bytes from %s (%s): %s (type = %d, code = %d)\n", icmplen, hostname, ipstr,
				icmp_type_error(icmp->icmp_type), icmp->icmp_type, icmp->icmp_code);
	else
		printf("%u bytes from %s (%s): %s\n", icmplen, hostname, ipstr,
				icmp_type_error(icmp->icmp_type));
}

void print_stats(char const* hostname)
{
	uint32_t pcktloss;
	Timestamp min;
	Timestamp max;
	Timestamp avg;
	Timestamp stddev;

	pcktloss = 100 - (((double)info->pcktrecv / (double)info->pcktsent) * 100);
	printf("--- %s ping statistics ---\n", hostname);
	printf("%u packets transmitted, %u packets received, %u%% packet loss\n", info->pcktsent,
			info->pcktrecv, pcktloss);
	if (info->pcktrecv)
	{
		min = convert_ts(info->min_ts);
		max = convert_ts(info->max_ts);
		avg = convert_ts(info->avg_ts);
		stddev = convert_ts(calc_stddev());
		printf("round-trip min/avg/max/stddev = %lu,%lu/%lu,%lu/%lu,%lu/%lu,%03lu ms\n",
				min.whole, min.fractional, avg.whole, avg.fractional,
				max.whole, max.fractional, stddev.whole, stddev.fractional);
	}
}
