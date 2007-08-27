/*
    This module shows all not linked bots on all channels
    Copyright <c> 2004 Grzegorz Rusin <pks@irc.pl>

    To be distributed under GPL
*/

#include "framework.h"
#include "func.h"
#include "var.h"
#include "common.h"

Hooks backupHooks;

void botnetcmd(char *from, char *cmd)
{
    char arg[2][MAX_LEN];
    char buf[MAX_LEN];
    
    str2words(arg[0], cmd, 2, MAX_LEN);
    
    if(!strcmp(arg[1], "notlinked"))
    {
	chan *ch = ME->first;
	ptrlist<chanuser>::iterator u;
	inetconn *c;
	int i = 0;
	
	while(ch)
	{
	    u = ch->users.begin();
	    while(u)
	    {
		if(u != ch->me && u->flags & HAS_B)
		{
		    c = findConnByName(u->nick);
		    if(!c)
		    {
		        snprintf(buf, MAX_LEN, "[-] notlinked.so :: %s :: \002%s\002!%s@%s",
				ch->name, u->nick, u->ident, u->host);
		        sendToOwner(arg[0], buf);
			++i;
		    }
		}
		u++;
	    }
	    ch = ch->next;
	}
	snprintf(buf, MAX_LEN, "[*] notlinked.so :: found %d matches", i);
	sendToOwner(arg[0], buf);
    }

    if(backupHooks.botnetcmd)
	backupHooks.botnetcmd(from, cmd);
}	    

extern "C" void init(Hooks *hooks)
{
    //make backup copy of hooks structure
    memcpy(&backupHooks, hooks, sizeof(Hooks)); 
    
    (FUNCTION) hooks->botnetcmd = (FUNCTION) botnetcmd;
    
}
