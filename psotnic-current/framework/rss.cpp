#include "framework.h"
#include "func.h"
#include "var.h"
#include "common.h"

#define DELAY	600
#define CHANNEL "#psotnic"

Hooks backupHooks;
time_t nextCheck;

void printRss(char *script, char *channel, char *reqChannel)
{
    FILE *p = popen(script, "r");	
	
	if(!p)
	    return;
	    
	char buf[40960];
	int n;
	
	while(1)
	{
	    memset(buf, 0, 40960);
	    fgets(buf, 40960, p);
	    if((n = strlen(buf)))
	    {
		if(n > 512)
		    buf[512] = '\0';
		    
		if(findChannel(reqChannel))
		    privmsg(channel, buf);
	    }
	    else
		break;
	}
	fclose(p);
}

void hook_timer()
{
    if(nextCheck <= *NOW)
    {
	printRss("/root/perl/psotnic.pl", "#psotnic", "#psotnic");
	printRss("/root/perl/evangeline.pl", "#evangeline", "#evangeline");
	printRss("/root/perl/all.pl", "#psotnic,#evangeline", "#psotnic");
	
	nextCheck = *NOW + DELAY;
    }

    if(backupHooks.timer)
	backupHooks.timer();
}

/*
 * Init stuff
 */

extern "C" void init(Hooks *hooks)
{
    memcpy(&backupHooks, hooks, sizeof(Hooks)); 
    
    (FUNCTION) hooks->timer = (FUNCTION) hook_timer;

    nextCheck = *NOW;
}
