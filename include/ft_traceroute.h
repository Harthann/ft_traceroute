#ifndef FT_TRACEROUTE
#define FT_TRACEROUTE

/*	Sockets headers */
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netdb.h>

#include <netinet/in.h>
#include <netinet/udp.h>
#include <netinet/ip_icmp.h>

/* Utils headers */
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include "error.h"

/* PPH stands for Prob Per Hop */
/* It determines how many packets are sent for each hop test */
#define PPH 3
#define MAX_HOP 30

#define BASE_PORT 33434
#define UDP_MSG "42424242424242424242424242424242"
#define MSG_LEN sizeof(UDP_MSG) - 1
#define PACKET_SIZE sizeof(struct udphdr) + sizeof(struct iphdr) + MSG_LEN


typedef struct s_icmppkt {
	struct iphdr	ip; 
	struct icmphdr	hdr;
	struct iphdr	reqip;
	struct udphdr	reqhdr;
}	__attribute__((packed)) t_icmppkt;

typedef struct {

	/* Target hostnames if provided */
	char				*host;

	/* Target IP from getaddrinfo */
	char				*ip;

	/* Target info given by getaddrinfo */
	struct sockaddr_in	sockaddr;
	socklen_t			addrlen;

	/* Target socket created to send/recv */
	int					socketfd;
	int					receiverfd;

	/*	Keep track of the actual ttl tested */
	int					ttl;

} t_target;

typedef struct s_resinfo {
	struct timeval	timestamps;
	int				port;
	unsigned int	saddr;
	unsigned int	error;
} t_resinfo;

void trace(t_target target);
void print_result(t_target target, t_resinfo *infos ,struct timeval *timestamps);

#endif
