#include "ft_traceroute.h"

void print_result(t_target target, t_resinfo infos[PPH] ,struct timeval timestamps[PPH])
{
	printf("%d %s (%s) ", 1, "TODO", inet_ntoa(infos[0].saddr));
	for (int i = 0; i < PPH; i++) {
		printf("%f ms ", 0.0);
	}
	printf("\n");
}
