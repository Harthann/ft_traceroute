#include "ft_traceroute.h"

void send_triplet
(int socketfd, struct sockaddr_in sockaddr, struct timeval *timestamps)
{
	int tmp = ntohs(sockaddr.sin_port);

	for (int i = 0; i < PPH; i++) {
		sockaddr.sin_port = htons(tmp + i);
		sendto(socketfd, UDP_MSG, MSG_LEN, 0, (struct sockaddr*)&sockaddr, sizeof(struct sockaddr));
		gettimeofday(timestamps + i, NULL);
	}
}
#include <unistd.h>

int receive_responses(t_target target, t_resinfo *infos)
{
	unsigned int	index;
	t_icmppkt		packet = {0};
	int				ret = 0;
	int				counter = 0;
	struct sockaddr socktmp;
	socklen_t		socklen = sizeof(socktmp);

	errno = 0;
	while (counter < PPH) {
		memset(&packet, 0, sizeof(packet));
		errno = 0;
		if (recvfrom(target.receiverfd, &packet, sizeof(packet), 0, &socktmp, &socklen) > 0) {
			index = ntohs(packet.reqhdr.dest) - BASE_PORT;
			if (index < PPH || !SHOULD_IGNORE(packet.hdr)) {
				gettimeofday(&infos[index].timestamps, NULL);
				infos[index].port = ntohs(packet.reqhdr.dest);
				infos[index].saddr = packet.ip.saddr;
				infos[index].error = packet.hdr.code + 2;
				if (packet.hdr.type == 3 && packet.hdr.code == 3)
					ret = 1;
			}
		} else {
			PANICERRNO(ERR_UNDEFINED);
		}
		counter += 1;
	}

	return ret;
}

void trace(t_target target) {
	int				error = 0;
	struct 	timeval	timestamps[PPH] = {0};

	t_resinfo	infos[PPH] = {{
		.port = 0,
		.saddr = 0,
		.error = 0,
		.timestamps = {0}
	}};

/* Initializing ttl to the minimum value possible */
	target.ttl = 1;
	errno = 0;
	if (setsockopt(target.socketfd, IPPROTO_IP, IP_TTL, &target.ttl, sizeof(uint8_t)))
		PANICERRNO(ERR_SETSOCK);

	struct timeval tv;
	tv.tv_sec = TIMEOUT;
	tv.tv_usec = 0;
	if (setsockopt(target.receiverfd, SOL_SOCKET, SO_RCVTIMEO , &tv, sizeof(tv)) < 0)
		PANICERRNO(ERR_SETSOCK);

/* Setting up our bases port reference */
	target.sockaddr.sin_port = htons(BASE_PORT);
	printf("ft_traceroute to %s (%s), %d hops max, %ld byte packets\n",
			target.host, target.ip, MAX_HOP, PACKET_SIZE);

	for (int i = 0; i < MAX_HOP; i++) {
		memset(infos, 0, sizeof(infos));
		target.sockaddr.sin_port = htons(BASE_PORT);
		send_triplet(target.socketfd, target.sockaddr, timestamps);
		error = receive_responses(target, infos);
		if (error == 1)
		{
			print_result(target, infos, timestamps);
			exit (0);
		}
		print_result(target, infos, timestamps);
		target.ttl += 1;
		if (setsockopt(target.socketfd, IPPROTO_IP, IP_TTL, &target.ttl, sizeof(uint8_t)) < 0)
			PANICERRNO(ERR_SETSOCK);
	}
}
