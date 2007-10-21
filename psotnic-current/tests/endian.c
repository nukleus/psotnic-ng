#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main()
{
	int ip = inet_addr("255.128.0.192");

	switch(ip)
	{
		case 0xC00080FF:
		printf("LITTLE_ENDIAN\n");
		break;

		case 0xFF8000C0:
		printf("BIG_ENDIAN\n");
		break;

		default:
		printf("UNKNOWN\n");
		return 1;
	}

	return 0;
}

