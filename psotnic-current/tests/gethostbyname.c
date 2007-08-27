#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <sys/socket.h>

int main()
{
    struct hostent *h = gethostbyname("localhost");
    
    return h == NULL;
}
