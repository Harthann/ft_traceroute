#include "parser.h"
#include "ft_traceroute.h"

void print_help()
{
	printf("Usage:\n\ttraceroute [-h] host\n");
	printf("Options:\n");
	printf("-h\t\t\tPrint this message");
}

int parse_arg(int ac, char **av, t_target *target) {
	char c;

	while ((c = ft_getopt(ac, av, "ht:")) != -1)
	{
		switch (c)
		{
			case 'h':
				print_help();
				exit(0);
			case '?':
				print_help();
				exit(0);
		}
	}
	target->host = av[ft_optind];
	if (av[ft_optind + 1] != NULL) {
		printf("Sorry i don't handle packetlen yet\n");
		return 1;
	}
	return 0;
}

int	init_socket(t_target *target)
{
	struct addrinfo		hints;
	struct addrinfo		*res;
	struct timeval		tv = { 0 };
	char				on = 1;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	errno = 0;
	if (getaddrinfo(target->host, NULL, &hints, &res))
		RET_ERROR(-1);

	target->sockaddr = *(struct sockaddr_in*)res->ai_addr;
	target->addrlen = res->ai_addrlen;
	target->ip = inet_ntoa(target->sockaddr.sin_addr);

/*
** Opening socket to our target with UDP protocol
** Opening a second socket to read icmp responses
*/
	errno = 0;
	target->socketfd = socket(AF_INET, SOCK_RAW, IPPROTO_UDP);
	target->receiverfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP);
	if (target->socketfd < 0 || target->receiverfd < 0)
		RET_ERROR(-1);

/*
** Telling our kernel that we will handle ip header
** So he doesn't fill it himself
*/
	if (setsockopt(target->socketfd, IPPROTO_IP, IP_HDRINCL, (char*)&on, sizeof(on)) < 0)
		RET_ERROR(-1);

/*
**	Setting timeout to our receiving socket
*/
	tv.tv_sec = TIMEOUT;
	if (setsockopt(target->receiverfd, SOL_SOCKET, SO_RCVTIMEO , &tv, sizeof(tv)) < 0)
		RET_ERROR(-1);

	freeaddrinfo(res);
	return 0;
}

int main(int ac, char **av)
{
	t_target target = {
		.host = NULL,
		.ip = NULL
	};

	if (ac < 2) {
		print_help();
		return 0;
	}

	if (parse_arg(ac, ++av, &target) != 0)
		return 0;
	if (init_socket(&target) != 0)
		return -1;
	trace(target);
	return 0;
}
