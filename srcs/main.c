#include "parser.h"
#include "ft_traceroute.h"
#include <time.h>

void print_help()
{
	printf("Usage:\n\t./ft_traceroute [-h] host\n");
	printf("Options:\n");
	printf("-h\t\t\tPrint this message\n");
	printf("host\t\t\tCan be either an ip or hostname\n\t\t\tEx: ./ft_traceroute google.com\n");
}

int parse_arg(int ac, char **av, t_target *target) {
	char c;

	while ((c = ft_getopt(ac, av, "h")) != -1)
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
	if (target->host == NULL) {
		printf("Missing argument\n");
		print_help();
		return (-1);
	}
	return 0;
}

/*
** Initialize two sockets to proceed exchange
** One udp socket for sending and one icmp for receiving
** If one of the syscalls fail (socket or setsockopt)
** Goto will make us jump at the of the function to free addrinfo
** and return an error
*/
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

	if (getaddrinfo(target->host, NULL, &hints, &res)) {
		fprintf(stderr, "Couldn't resolve hostname\n");
		return (-1);
	}

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
		goto _init_socket_end;

/*
** Telling our kernel that we will handle ip header
** So he doesn't fill it himself
*/
	errno = 0;
	if (setsockopt(target->socketfd, IPPROTO_IP, IP_HDRINCL, (char*)&on, sizeof(on)) < 0)
		goto _init_socket_end;

/*
**	Setting timeout to our receiving socket
*/
	tv.tv_sec = TIMEOUT;
	errno = 0;
	setsockopt(target->receiverfd, SOL_SOCKET, SO_RCVTIMEO , &tv, sizeof(tv));

_init_socket_end:
	freeaddrinfo(res);
	if (errno != 0)
		RET_ERROR(-1);
	return 0;
}

int main(int ac, char **av)
{
	t_target target = {
		.host = NULL,
		.ip = NULL
	};

	if (getuid() != 0) {
		printf("%s: don't have enough permission\n", av[0]);
		return 0;
	}

	srand(time(NULL));

	if (parse_arg(ac - 1, ++av, &target) != 0)
		return 0;
	if (init_socket(&target) != 0)
		return -1;

	trace(target);
	return 0;
}
