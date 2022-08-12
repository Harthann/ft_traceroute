#include <stdio.h>
#include <errno.h>

extern char *errors[];
/* Error codes */
#define ERR_DNS 0
//#define ERR_UNDEFINED 2 / sizeof(char*) - 1
#define ERR_UNDEFINED 1

#define ERROR(x) fprintf(stderr, "%s\nErrno said: %s\n", errors[x], strerror(errno))


