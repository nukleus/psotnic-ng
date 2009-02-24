#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <dlfcn.h>
       
int main(int argc, char *argv[])
{
    dlopen(NULL, 0);
    return 0;
}
