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

#define ERROR(x) fprintf(stderr, "%s\nErrno said: %s\n", errors[x], strerror(errno))
#define PANIC(x) { fprintf(stderr, "Panic! %s\n", errors[x]); exit (x); } 
#define PANICERRNO(x) { fprintf(stderr, "Panic! %s\nErrni said: %s\n", errors[x], strerror(errno)); exit (x); } 


