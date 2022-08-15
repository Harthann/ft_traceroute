/* Error messages */
char *errors[] = {
"Temporary failure in dns resolution",
"Invalid port",
"Error setting socket options",
"Undefined error"
};


char *icmp_errors[] = {
" *",				// Correspond to timeout error message
"Icmp reply",
"No route to host. The network is unreachable. ",
"No route to host. The host is unreachable. ",
"Connection refused. The protocol is unreachable. ",
"Destination port unreachable",
"Fragmentation needed but do not fragment (DF) bit was set.",
"Source route failed.",
"Destination network unknown",
"Destination host unknown",
"Source host isolated",
"Network administratively prohibited",
"Host administratively prohibited",
"Network unreachable for ToS",
"Host unreachable for ToS",
"Communication administratively prohibited.",
"Host Precedence Violation",
"Precedence cutoff in effect",
};
