#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

int main()
{
    struct hostent ret;
    struct hostent *retptr;
    char buf[4096];
    int error;

    gethostbyname2_r("localhost", AF_INET, &ret, buf, 4096, &retptr, &error);

    return retptr == NULL;
}
