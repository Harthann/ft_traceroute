#include "ft_traceroute.h"

void dbg_dump_bytes(const void* data, size_t size) {
	char ascii[17];
	size_t i;
	ascii[16] = '\0';
	for (i = 0; i < size; ++i) {
		if (i % 16 == 0)
			fprintf(stderr, "%p: ", data + i);
		fprintf(stderr, "%02x ", ((unsigned char*)data)[i]);
		if (((unsigned char*)data)[i] >= ' ' && ((unsigned char*)data)[i] <= '~') {
			ascii[i % 16] = ((unsigned char*)data)[i];
		} else {
			ascii[i % 16] = '.';
		}
		if ((i+1) % 8 == 0 || i+1 == size) {
			fprintf(stderr, " ");
			if ((i+1) % 16 == 0) {
				fprintf(stderr, "|  %s \n", ascii);
			} else if (i+1 == size) {
				ascii[(i+1) % 16] = '\0';
				if ((i+1) % 16 <= 8) {
					fprintf(stderr, " ");
				}
				fprintf(stderr, "%*.0d", 3 * (16 - (((int)i + 1) % 16)), 0);
				fprintf(stderr, "|  %s \n", ascii);
			}
		}
	}
}


float		diff_time(struct timeval a, struct timeval b)
{
	a.tv_sec -= b.tv_sec;
	a.tv_usec -= b.tv_usec;
	return ( ((float)a.tv_sec * 1000000 + (float)a.tv_usec) / 1000);
}

void send_triplet
(int socketfd, struct sockaddr_in* sockaddr, struct timeval timestamps[PPH])
{
	int tmp = ntohs(sockaddr->sin_port);

	/*	First udp package */
	for (int i = 0; i < PPH; i++) {
		sendto(socketfd, UDP_MSG, MSG_LEN, 0, (struct sockaddr*)sockaddr, sizeof(struct sockaddr));
		gettimeofday(timestamps + i, NULL);
		sockaddr->sin_port = htons(tmp + i);
	}
}

int receive_responses(t_target target, t_resinfo infos[PPH])
{
	char	buffer[64];
	t_icmppkt packet = {0};

	/* recvmsg struct initialisation */
	struct iovec	iov = {
		.iov_base = &packet,
		.iov_len = sizeof(t_icmppkt)
	};
	struct msghdr	message = {
		.msg_iov = &iov,
		.msg_iovlen = 1,
		.msg_control = buffer,
		.msg_controllen = sizeof(buffer),
		.msg_flags = 0
	};

	errno = 0;
	for (int i = 0; i < PPH; i++)
	{
		if (recvmsg(target.receiverfd, &message, 0) < 0) {
			fprintf(stderr, "Error: %s\n", strerror(errno));
			return -1;
		}
		gettimeofday(&infos[i].timestamps, NULL);
		infos[i].port = ntohs(packet.reqhdr.dest);
		infos[i].saddr = packet.ip.saddr;
	}

	return 0;
}

void trace(t_target target) {
	int ttl = 1;
	int error = 0;
	struct timeval timestamps[PPH] = {{ 0 }};
	t_resinfo	infos[PPH] = {{
		.port = 0,
		.saddr = 0,
		.timestamps = {0}
	}};

	if (setsockopt(target.socketfd, IPPROTO_IP, IP_TTL, &ttl, sizeof(uint8_t)))
		fprintf(stderr, "Failed to setsockopt(): %s\n", strerror(errno));

	target.sockaddr.sin_port = htons(BASE_PORT);
	printf("Selected port: %d\n", ntohs(target.sockaddr.sin_port));


	for (int i = 0; i < MAX_HOP; i++) {
		target.sockaddr.sin_port = htons(BASE_PORT + ((ttl - 1) * 3));
		send_triplet(target.socketfd, &target.sockaddr, timestamps);
		error = receive_responses(target, infos);
		(void)error;
		print_result(target, infos, timestamps);
		// Print timings
		// Increment ttl if not reached target
		exit (0);
	}
}
