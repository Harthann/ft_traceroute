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
	if (av[ft_optind + 1] != NULL) {
		printf("Sorry i don't handle packetlen yet\n");
		return 1;
	}
	return 0;
}

int	resolve_host(t_target *target)
{
	struct addrinfo		hints;
	struct addrinfo		*res;

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	if (getaddrinfo(target->host, NULL, &hints, &res))
		return -1;
	target->sockaddr = *(struct sockaddr_in*)res->ai_addr;
	target->ip = inet_ntoa(target->sockaddr.sin_addr);
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
	if (resolve_host(&target) != 0) {
		PRINT_ERROR(ERR_DNS, "%s: %s\n", target.host);
	}
	printf("Target: %s\nTarget IP: %s\n", target.host, target.ip);
	return 0;
}
