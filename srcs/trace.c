#include "ft_traceroute.h"

static unsigned long int next_rand = 1;

void		ft_srand(unsigned int seed)
{
	next_rand = seed;
}

int			ft_rand(void)
{
	next_rand = next_rand * 1103515245 + 12345;
	return (unsigned int)(next_rand / 65536) % 32768;
}

unsigned short checksum(void *addr, size_t count)
{
	unsigned short *ptr;
	unsigned long sum;

	ptr = addr;
	for (sum = 0; count > 1; count -= 2)
		sum += *ptr++;
	if (count > 0)
		sum += *(unsigned char *)ptr;
	while (sum >> 16)
		sum = (sum & 0xffff) + (sum >> 16);
	return (~sum);
}

unsigned short udp_checksum(t_target target, t_udppkt *addr)
{
	t_pseudo_udphdr dummy = { 0 };

	memcpy(&dummy.udp, &addr->udp, sizeof(t_udppkt));
	dummy.saddr = target.self;
	dummy.daddr = ((struct sockaddr_in)target.sockaddr).sin_addr;
	dummy.protocol = IPPROTO_UDP;
	dummy.udplen = addr->udp.len;
	return (checksum(&dummy, sizeof(dummy)));
}

void fill_ip(struct iphdr *ip, t_target target)
{
	 ip->ihl = 5;
	ip->version = 4;
	ip->tos = 16; // Low delay
	ip->tot_len = sizeof(struct iphdr) + sizeof(t_udppkt);
	ip->id = htons(54321);
	ip->ttl = target.ttl; // hops
	ip->protocol = 17; // UDP
	// Source IP address, can use spoofed address here!!!
	ip->saddr = target.self.s_addr;
	// The destination IP address
	ip->daddr = target.sockaddr.sin_addr.s_addr;
}

void send_triplet
(t_target target, struct timeval *timestamps, short *checks)
{
	int tmp = ntohs(target.sockaddr.sin_port);
	t_udppkt packet = { 0 };
	fill_ip(&packet.ip, target);

	memset(&packet, 42, sizeof(packet));
	packet.udp.len = sizeof(packet);
	

	for (int i = 0; i < PPH; i++) {
		target.sockaddr.sin_port = htons(tmp + i);
		packet.udp.dest = target.sockaddr.sin_port;
		packet.udp.source = htons(ft_rand());
		packet.udp.check = udp_checksum(target, &packet);
		checks[i] = packet.udp.check;

		errno = 0;
		int ret = sendto(target.socketfd, &packet, sizeof(packet), 0, (struct sockaddr*)&target.sockaddr, sizeof(struct sockaddr));
		printf("%d %ld %d %s %x\n", ret, sizeof(packet), errno, strerror(errno), checks[i]);

		gettimeofday(timestamps + i, NULL);
	}
}
#include <unistd.h>

int receive_responses(t_target target, t_resinfo *infos, short *checks)
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
			(void)checks;
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
	struct in_addr tmp;


	socklen_t		socklen = sizeof(socktmp);
	t_icmppkt		packet = {0};

	errno = 0;
	sendto(target->socketfd, UDP_MSG, MSG_LEN, 0, (struct sockaddr*)&target->sockaddr, sizeof(struct sockaddr));

	recvfrom(target->receiverfd, &packet, sizeof(packet), 0, &socktmp, &socklen);
	tmp.s_addr = packet.ip.daddr;
	target->self = tmp;
}

void trace(t_target target) {
	int				error = 0;
	struct 	timeval	timestamps[PPH] = {0};
	short			checks[PPH] = { 0 };

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

	retreive_self_addr(&target);

	char on = 1;
	if (setsockopt(target.socketfd, IPPROTO_IP, IP_HDRINCL, (char *) &on, sizeof(on)) < 0)
		PANICERRNO(ERR_SETSOCK);

	for (int i = 0; i < MAX_HOP; i++) {
		memset(infos, 0, sizeof(infos));
		target.sockaddr.sin_port = htons(BASE_PORT);
		send_triplet(target, timestamps, checks);
		error = receive_responses(target, infos, checks);
		if (error == 1)
		{
			print_result(target, infos, timestamps);
			exit (0);
		}
		print_result(target, infos, timestamps);
		target.ttl += 1;
		//if (setsockopt(target.socketfd, IPPROTO_IP, IP_TTL, &target.ttl, sizeof(uint8_t)) < 0)
		//	PANICERRNO(ERR_SETSOCK);
	}
}
