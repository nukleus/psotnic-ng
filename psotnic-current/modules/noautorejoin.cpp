/* noautorejoin module for psotnic
 *
 * This module avoids that users rejoin automatically after being kicked.
 * The user will be banned temporarily.
 */

#include "../prots.h"
#include "../global-var.h"
#include "module.h"

/* if the user rejoins after being kicked within this time (in seconds),
   it will be handled as autorejoin.
*/

#define NOARJ_DELAY 3

// ban time as punishment for autorejoin (in minutes)

#define NOARJ_BAN_TIME 1

class arj_chk
{
 public:
	class entry
	{
	 public:
		chan *channel;
		char nick[32];
		time_t timestamp;

		entry(chan *, char *, time_t);
		bool expired();
	};

	ptrlist<entry> data;

	arj_chk();
	void add(chan *, char *);
	entry *find(chan *, char *);
};

arj_chk::entry::entry(chan *_channel, char *_nick, time_t _timestamp)
{
	channel=_channel;
	strncpy(nick, _nick, sizeof(nick)-1);
	nick[sizeof(nick)-1]='\0';
	timestamp=_timestamp;
}

bool arj_chk::entry::expired()
{
    return NOW>timestamp+NOARJ_DELAY;
}

arj_chk::arj_chk()
{
	data.removePtrs();
}

void arj_chk::add(chan *channel, char *nick)
{
	entry *node=new entry(channel, nick, NOW);
	data.add(node);
}

arj_chk::entry *arj_chk::find(chan *channel, char *nick)
{
        ptrlist<entry>::iterator i;

        for(i=data.begin(); i; i++)
                if(i->channel==channel && !strcmp(i->nick, nick))
			return i;
	return NULL;
}

arj_chk autorejoincheck;

void hook_kick(chan *ch, chanuser *kicked, chanuser *kicker, const char *reason)
{
    if(!(ch->me->flags&IS_OP) || kicked->flags & (IS_OP | HAS_O | HAS_V | HAS_F))
        return;

    autorejoincheck.add(ch, kicked->nick);
}

void hook_join(chanuser *u, chan *ch, const char *mask, int netjoin)
{
    char buffer[MAX_LEN];
    arj_chk::entry *entry;

    if(netjoin || !(ch->me->flags&IS_OP))
        return;
	
    if((entry=autorejoincheck.find(ch, u->nick)))
    {
        autorejoincheck.data.remove(entry);
        snprintf(buffer, MAX_LEN, "*!%s@%s", u->ident, u->host);

        if(set.BOTS_CAN_ADD_SHIT
           && protmodelist::addShit(ch->name, buffer, "noautorejoin", NOARJ_BAN_TIME*60, "Please disable autorejoin"))
            return;

        else
        {
            snprintf(buffer, MAX_LEN, "Please disable autorejoin - banned for %d min%s", NOARJ_BAN_TIME, NOARJ_BAN_TIME==1?"":"s");
            knockout(ch, u, buffer, NOARJ_BAN_TIME*60); 
        }
    }
}

void hook_timer()
{
        ptrlist<arj_chk::entry>::iterator i, j;

        i=autorejoincheck.data.begin();

	while(i)
	{
		j=i;
		j++;

		if(i->expired())
			autorejoincheck.data.remove(i);

		i=j;
	}
}

extern "C" module *init()
{
    module *m=new module("noautorejoin", "patrick <patrick@psotnic.com>", "0.2");
    m->hooks->join=hook_join;
    m->hooks->kick=hook_kick;
    m->hooks->timer=hook_timer;
    return m;
}

extern "C" void destroy()
{
    autorejoincheck.data.clear();
}
