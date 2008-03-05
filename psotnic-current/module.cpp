/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "prots.h"
#include "global-var.h"

#ifdef HAVE_MODULES

/* intermediate functions - lame */
CHANLIST *findChanlist(const char *name)
	{ return userlist.findChanlist(name); }

chan *findChannel(const char *name)
	{ return ME.findChannel(name); }

chanuser *findUser(const char *nick, chan *channel)
	{ return channel->getUser(nick); }

void sendToOwner(const char *owner, const char *msg)
	{ net.sendOwner(owner, msg, NULL); }

void sendToOwners(const char *msg)
	{ net.send(HAS_N, msg, NULL); }

int kick4(chan *channel, chanuser **multHandle, int num)
	{ return channel->kick4(multHandle, num); }

int kick6(chan *channel, chanuser **multHandle, int num)
	{ return channel->kick6(multHandle, num); }

int kick(chan *channel, chanuser *p, const char *reason)
	{ return channel->kick(p, reason); }

void setReason(chanuser *u, const char *reason)
	{ u->setReason(reason); }

void addMode(chan *channel, const char mode[2], const char *arg, int prio, int delay=0)
	{ channel->modeQ[prio].add(NOW+delay, mode, arg); }

int flushModeQueue(chan *channel, int prio)
	{ return channel->modeQ[prio].flush(prio); }

void addKick(chan *channel, chanuser *p, const char *reason=NULL)
	{ p->setReason(reason); channel->toKick.sortAdd(p); }

int getPenalty()
	{ return penalty; }

void privmsg(const char *to, const char *msg)
	{ ctcp.push("PRIVMSG ", to, " :", msg, NULL); }

void notice(const char *to, const char *msg)
	{ ctcp.push("NOTICE ", to, " :", msg, NULL); }

const char *nick()
	{ return ME.nick; }

const char *handle()
	{ return config.handle; }

void cycle(chan *channel, const char *reason=NULL, int rejoinDelay=-1)
{
	net.irc.send("PART ", (const char *) channel->name, " :", reason ? reason : (const char *) config.cyclereason, NULL);
	ME.rejoin(channel->name, rejoinDelay != -1 ? rejoinDelay : set.CYCLE_DELAY);
}

int isMyChannel(const char *name)
{
	int i = userlist.findChannel(name);

	if(i == -1)
		return 0;

	return userlist.me()->channels & int(pow(2.0, double(i)));
}

char *server()
	{ return net.irc.name ; }

char *ircIp()
	{ return (char *) net.irc.getMyIpName(); }

void knockout(chan *ch, chanuser *u, const char *reason, int delay=60)
{
	static char buf[MAX_LEN];
	snprintf(buf, MAX_LEN, "*!%s@%s", u->ident, u->host);
    ch->modeQ[PRIO_HIGH].add(NOW, "+b", buf);
	ch->modeQ[PRIO_LOW].add(NOW+delay, "-b", buf)->backupmode = true;
	ch->modeQ[PRIO_HIGH].flush(PRIO_HIGH);

	ch->kick(u, reason);
	u->setReason(reason);
}

int saveUserlist(const char *file, const char *pass)
{
	if(config.bottype == BOT_MAIN)
		return userlist.save(file, pass ? 1 : 0, pass);

	errno = EACCES;
	return 0;
}

char *httpget(const char *link)
{
	http h;

	if(h.get(link) > 0)
	{
		char *tmp = h.data;
		h.data = NULL;
		return tmp;
	}
	return NULL;
}

void setTopic(chan *ch, const char *topic)
{
	net.irc.send("TOPIC ", (const char *) ch->name, " :", topic, NULL);
}

inetconn *findConnByHandle(HANDLE *h)
{
	return net.findConn(h);
}

inetconn *findConnByName(const char *name)
{
	return net.findConn(name);
}

void flags2str(int flags, char *str)
{
	ul::flags2str(flags, str);
}

bool isSticky (const char *ban, chan *ch)
{
	return protmodelist::isSticky(ban, BAN, ch);
}

void stop()
{
	stopParsing=true;
}

void reconnect(const char *reason, int delay=0)
{
	net.irc.send("QUIT :", reason, NULL);
	net.irc.close(reason);
	ME.nextConnToIrc = NOW + delay;
}

/*
 * Module loading related stuff
 */

#define _initCustomData(what, name) \
if(!strcmp(name, Class)) \
{ \
    memcpy(&what::customDataConstructor, &constructor, sizeof(void *)); \
    memcpy(&what::customDataDestructor, &destructor, sizeof(void *)); \
    return 1; \
}

FUNCTION (*chanuser::customDataConstructor)(chanuser *) = NULL;
FUNCTION (*chanuser::customDataDestructor)(chanuser *) = NULL;
FUNCTION (*CHANLIST::customDataConstructor)(CHANLIST *) = NULL;
FUNCTION (*CHANLIST::customDataDestructor)(CHANLIST *) = NULL;
FUNCTION (*chan::customDataConstructor)(chan *) = NULL;
FUNCTION (*chan::customDataDestructor)(chan *) = NULL;

int initCustomData(const char *Class, FUNCTION constructor, FUNCTION destructor)
{
	_initCustomData(chanuser, "chanuser");
	_initCustomData(CHANLIST, "CHANLIST");
	_initCustomData(chan, "chan");

	return 0;
}

#include "register_module.h"

#endif
