#include <stdio.h>
#include <errno.h>

extern char *errors[];
extern char *icmp_errors[];

/* Error codes */
#define ERR_DNS 0
#define ERR_PORT 1
#define ERR_SETSOCK 2
#define ERR_UNDEFINED 3
#define NB_ICMPCODE 16

#define RET_ERROR(x) { \
	fprintf(stderr, "%s\n", strerror(errno)); \
	return (x); \
}

#define PANIC(x) { fprintf(stderr, "Panic! %s\n", strerror(errno)); exit (x); } 


