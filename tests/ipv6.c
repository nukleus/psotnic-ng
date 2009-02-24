#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char *argv[])
{
    struct sockaddr_in6 sin6;

    if(socket(PF_INET6, SOCK_STREAM, 0) != -1)
	return 0;
	
    return 1;
}
