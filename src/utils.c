#include "ft_ping.h"

double timestamp_diff(struct timeval* before)
{
	struct timeval now;
	double ms;

	// TODO: handle error
	gettimeofday(&now, NULL);
	ms = (((double)now.tv_sec * 1000) + ((double)now.tv_usec / 1000)) -
			(((double)before->tv_sec * 1000) + ((double)before->tv_usec / 1000));
	return ms;
}

Timestamp convert_ts(double ms)
{
	Timestamp ts;

	ts.whole = (uint64_t)ms;
	ts.fractional = (uint64_t)((ms - ts.whole) * 1000);
	return ts;
}

double calc_stddev(void)
{
	double stddev;

	stddev = 0;
	for (uint32_t i = 0; i < info->pcktrecv; i++)
		stddev += pow(info->list_ts[i] - info->avg_ts, 2);
	return sqrt(stddev / 10);
}
