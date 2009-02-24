#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main()
{
    struct hostent *h = gethostbyname2("localhost", AF_INET);

    return h == NULL;
}
