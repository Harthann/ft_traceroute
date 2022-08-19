#include "ft_traceroute.h"

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

unsigned short udp_checksum(t_udppkt *addr)
{
	t_pseudo_udphdr dummy = { 0 };

	memcpy(&dummy.udp, &addr->udp, sizeof(struct udphdr));
	memset(&dummy.msg, 42, MSG_LEN);
	dummy.saddr = addr->ip.saddr;
	dummy.daddr = addr->ip.daddr;
	dummy.protocol = IPPROTO_UDP;
	dummy.udplen = addr->udp.len;

	return (checksum(&dummy, sizeof(dummy)));
}

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

