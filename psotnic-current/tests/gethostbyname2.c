#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>

int main()
{
    struct hostent *h = gethostbyname2("localhost", AF_INET);
    
    return h == NULL;
}
