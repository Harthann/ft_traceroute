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

int receive_responses(t_target target, t_resinfo *infos)
{
	unsigned int	index;
	t_icmppkt		packet = {0};
	int				ret = 0;
	t_icmppkt		recv = { 0 };
	struct sockaddr socktmp;
	socklen_t socklen;

	/* recvmsg struct initialisation */
	struct iovec	iov = {
		.iov_base = &packet,
		.iov_len = sizeof(t_icmppkt)
	};
	struct msghdr	message = {
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = NULL,
		.msg_controllen = 0,
		.msg_flags = 0
	};

	(void)message;

	errno = 0;
	for (int i = 0; i < PPH; i++)
	{
		errno = 0;
		int tmp;
		//if ((tmp = recvmsg(target.receiverfd, &message, MSG_WAITALL)) > 0)
		if ((tmp = recvfrom(target.receiverfd, &recv, sizeof(recv), 0, &socktmp, &socklen)) > 0)
		{
			printf("Port :%d\n", ntohs(packet.reqhdr.dest));
			index = ntohs(packet.reqhdr.dest) - BASE_PORT;
			if (index >= PPH) {
				i--;
			}
			else {
				gettimeofday(&infos[index].timestamps, NULL);
				infos[index].port = ntohs(packet.reqhdr.dest);
				infos[index].saddr = packet.ip.saddr;
				infos[index].error = packet.hdr.code + 2;
				if (packet.hdr.type == 3 && packet.hdr.code == 3)
					ret = 1;
			}
		}
		else
		{
			printf("%d %s\n", tmp, strerror(errno));
		}
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
	tv.tv_sec = 1;
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
