/* noautorejoin module for psotnic
 *
 * This module avoids that users rejoin automatically after being kicked.
 * The user will be banned temporarily.
 */

#include "global-var.h"
#include "match.h"
#include "module.h"
#include "noautorejoin.hpp"

/* if the user rejoins after being kicked within this time (in seconds),
   it will be handled as autorejoin.
*/

#define NOARJ_DELAY 3

// ban time as punishment for autorejoin (in minutes)

#define NOARJ_BAN_TIME 1

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

arj_chk::arj_chk(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir) : Module(handle, file, md5sum, loadDate, dataDir)
{
	data.removePtrs();
}

arj_chk::~arj_chk()
{
	data.clear();
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

void arj_chk::onKick(chan *ch, chanuser *kicked, chanuser *kicker, const char *reason)
{
	if(!(ch->me->flags&IS_OP) || kicked->flags & (IS_OP | HAS_O | HAS_V | HAS_F))
		return;

	add(ch, kicked->nick);
}

void arj_chk::onJoin(chanuser *u, chan *ch, const char *mask, int netjoin)
{
	char buffer[MAX_LEN];
	entry *entry;

	if(netjoin || !(ch->me->flags&IS_OP))
		return;

	if((entry=find(ch, u->nick)))
	{
		data.remove(entry);
		snprintf(buffer, MAX_LEN, "*!%s@%s", u->ident, u->host);

		if(set.BOTS_CAN_ADD_SHIT
				&& protmodelist::addShit(ch->name, buffer, "noautorejoin", NOARJ_BAN_TIME*60, "Please disable autorejoin"))
			return;

		else
		{
			snprintf(buffer, MAX_LEN, "Please disable autorejoin - banned for %d min%s", NOARJ_BAN_TIME, NOARJ_BAN_TIME==1?"":"s");
			ch->knockout(u, buffer, NOARJ_BAN_TIME*60);
		}
	}
}

void arj_chk::onTimer()
{
	ptrlist<entry>::iterator i, j;

	i=data.begin();

	while(i)
	{
		j=i;
		j++;

		if(i->expired())
			data.remove(i);

		i=j;
	}
}


MOD_LOAD( arj_chk );
MOD_DESC( "NoAutoRejoin", "auto rejoin protection" );
MOD_AUTHOR( "patrick", "patrick@psotnic.com" );
MOD_VERSION( "0.2" );

