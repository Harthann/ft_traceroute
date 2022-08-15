#include "ft_traceroute.h"

float		diff_time(struct timeval a, struct timeval b)
{
	a.tv_sec -= b.tv_sec;
	a.tv_usec -= b.tv_usec;
	return ( ((float)a.tv_sec * 1000000 + (float)a.tv_usec) / 1000);
}

void print_result(t_target target, t_resinfo *infos ,struct timeval *timestamps)
{
	struct in_addr		tmp = { 0 };
	char				hostname[NI_MAXHOST] = { 0 };
	struct sockaddr_in	address = { 0 };

	address.sin_family = AF_INET;
	address.sin_addr = tmp;

	printf("%d ", target.ttl);

	for (int i = 0; i < PPH; i++) {
		if (infos[i].error != 0)
		{
			tmp.s_addr = infos[i].saddr;
			getnameinfo((struct sockaddr*)&address, sizeof(address), hostname, sizeof(hostname), NULL, 0, NI_NOFQDN);

			if (i == 0 || infos[i].saddr != infos[i - 1].saddr) {
				printf(" %s (%s)", inet_ntoa(tmp), inet_ntoa(tmp));
			}
			printf(" %.3f ms", diff_time(infos[i].timestamps, timestamps[i]));
		}
		else if (infos[i].error < NB_ICMPCODE) {
			printf(" %s", icmp_errors[infos[i].error]);
		}
	}
	printf("\n");
}
