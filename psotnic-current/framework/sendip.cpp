#include "framework.h"
#include "func.h"
#include "var.h"
#include "common.h"

#define DELAY	60
#define ADDR	"127.0.0.1"

struct PACKET
{
    char ver; 
    short int id;
    char ip[32];
};

Hooks backupHooks;
time_t nextSend;

void hook_timer()
{
    if(nextSend <= *NOW)
    {
	int s = socket(AF_INET, SOCK_DGRAM, 0);
	
	if(s > 0)
	{
	    struct sockaddr_in sin;    
	    PACKET pkt;
	    
	    pkt.ver = 1;
	    pkt.id = 1;
	    strncpy(pkt.ip, ircIp(), 32);
	    
	    memset (&sin, 0, sizeof (sin));
	    sin.sin_family = AF_INET;
	    sin.sin_addr.s_addr = INADDR_ANY;

	    sendto(s, &pkt, sizeof(PACKET), MSG_DONTWAIT, (const sockaddr* ) &sin, sizeof(sin));
	}
	nextSend = *NOW + DELAY;
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

    nextSend = *NOW + DELAY;
}
