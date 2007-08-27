#include <pthread.h>
#include <stdio.h>

void *test(void *nothing)
{
   pthread_exit((void *) 0);
}

int main(int argc, char *argv[])
{
    int status;
    pthread_t thread;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

    if(pthread_create(&thread, &attr, test, NULL))
	return 1;
	
    pthread_attr_destroy(&attr);

    if(pthread_join(thread, (void **)&status))
	return 1;
    
    pthread_exit(NULL);
    
    return 0;
}
