#ifndef FT_TRACEROUTE
#define FT_TRACEROUTE

/*	Sockets headers */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>

/* Utils headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

/* Error messages */
char *errors[] = {
"Temporary failure in dns resolution",
"Undefined error"
};

/* Error codes */
#define ERR_DNS 0
#define ERR_UNDEFINED sizeof(errors) / sizeof(char*) - 1

#define ERROR(x) fprintf(stderr, "%s\n", errors[x])
#define PRINT_ERROR(x, fmt, ...) fprintf(stderr, fmt, errors[x], __VA_ARGS__)

typedef struct {
	char				*host;
	char				*ip;
	struct sockaddr_in	sockaddr;
} t_target;

#endif
