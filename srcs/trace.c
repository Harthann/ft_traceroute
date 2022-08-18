#include "ft_traceroute.h"

void fill_ip(struct iphdr *ip, t_target target)
{
	ip->version = 4;
	ip->ihl = 5;
	ip->tos = 0; // Low delay
	ip->tot_len = sizeof(t_udppkt);
	ip->id = 0;
	ip->ttl = target.ttl; // hops
	ip->protocol = IPPROTO_UDP; // UDP
	// Source IP address, can use spoofed address here!!!
	ip->saddr = target.self.s_addr;
	// The destination IP address
	ip->daddr = target.sockaddr.sin_addr.s_addr;
}

void send_triplet
(t_target *target, struct timeval *timestamps, u_int16_t *checks)
{
	int tmp = ntohs(target->sockaddr.sin_port);
	t_udppkt packet = { 0 };

	memset(&packet.msg, 42, sizeof(packet.msg));
	fill_ip(&packet.ip, *target);
	packet.udp.len = htons(sizeof(struct udphdr) + MSG_LEN);

	for (int i = 0; i < PPH; i++) {
		packet.udp.check = 0;
		target->sockaddr.sin_port = htons(tmp + i);
		packet.udp.dest = target->sockaddr.sin_port;
		packet.udp.source = htons(ft_rand());
		packet.udp.check = udp_checksum(&packet);
		checks[i] = packet.udp.check;

		errno = 0;
		sendto(target->socketfd, &packet, sizeof(packet), 0, (struct sockaddr*)&target->sockaddr, sizeof(struct sockaddr));

		gettimeofday(timestamps + i, NULL);
	}
}
#include <unistd.h>


int receive_responses(t_target target, t_resinfo *infos)
{
	unsigned int	index = -1;
	t_icmppkt		packet = {0};
	int				ret = 0;
	int				counter = 0;
	struct sockaddr socktmp;
	socklen_t		socklen = sizeof(socktmp);

	errno = 0;

	while (counter < PPH) {
		index = -1;
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
		}
		counter += 1;
	}

	return ret;
}

void retreive_self_addr(t_target *target) {
	struct sockaddr socktmp;
	t_udppkt		payload;


	fill_ip(&payload.ip, *target);
	payload.ip.saddr = INADDR_ANY;
	memset(&payload.msg, 42, MSG_LEN);
	
	payload.udp.len = htons(sizeof(struct udphdr) + MSG_LEN);
	payload.udp.dest = htons(33333);
	payload.udp.source = htons(ft_rand());
	payload.udp.check = 0;
	
	socklen_t		socklen = sizeof(socktmp);
	t_icmppkt		packet = {0};

	errno = 0;
	sendto(target->socketfd, &payload, sizeof(t_udppkt), 0, (struct sockaddr*)&target->sockaddr, sizeof(struct sockaddr));
	target->sockaddr.sin_port = htons(ntohs(target->sockaddr.sin_port) + 1);

	while (packet.hdr.type != 11 || packet.hdr.code != 0)
		recvfrom(target->receiverfd, &packet, sizeof(packet), 0, &socktmp, &socklen);
	memcpy(&target->self, &packet.ip.daddr, sizeof(u_int32_t));
}

void trace(t_target target) {
	int				error = 0;
	struct 	timeval	timestamps[PPH] = {0};
	u_int16_t			checks[PPH] = { 0 };

	t_resinfo	infos[PPH] = {{
		.port = 0,
		.saddr = 0,
		.error = 0,
		.timestamps = {0}
	}};

/*
** Sending a first packet to fetch our own ip from response
** This is done in order to fill the ip header
** And so properly calculating checksum
*/
	target.sockaddr.sin_port = htons(BASE_PORT);
	retreive_self_addr(&target);
	
	printf("ft_traceroute to %s (%s), %d hops max, %ld byte packets\n",
			target.host, target.ip, MAX_HOP, PACKET_SIZE);

	for (int i = 0; i < MAX_HOP; i++) {
		memset(infos, 0, sizeof(infos));
		target.sockaddr.sin_port = htons(BASE_PORT);

		send_triplet(&target, timestamps, checks);

		error = receive_responses(target, infos);
		if (error == 1) {
			print_result(target, infos, timestamps);
			exit (0);
		}

		print_result(target, infos, timestamps);
		target.ttl += 1;
	}
}
