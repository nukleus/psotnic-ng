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

protmodelist::entry::entry()
{
}

protmodelist::entry::entry(const char *_mask, const char *_by, const char *_reason, time_t _when, time_t _expires, bool _sticky)
{
	mem_strcpy(mask, _mask);
	mem_strcpy(by, _by);
	if(_reason && *_reason)
		mem_strcpy(reason, _reason);
	else
		mem_strcpy(reason, "requested");

	when = _when;
	expires = _expires;
	sticky = _sticky;
	last_used = 0;
}

protmodelist::entry::~entry()
{
	free(mask);
	free(by);
	free(reason);
}

char *protmodelist::entry::fullReason()
{
	static char buf[MAX_LEN];

	if(expires && expires - NOW <= 20*3600)
	{
		snprintf(buf, MAX_LEN, "\002%s\002: %s - expires: %s", by, reason,
			timestr("%T", expires));
	}

	else
	{
		snprintf(buf, MAX_LEN, "\002%s\002: %s - expires: %s", by, reason,
			expires ? timestr("%d/%m/%Y", expires) : "never");
	}

	return buf;
}

protmodelist::protmodelist(int _type, char _mode)
{
	type = _type;
	mode = _mode;
	data.removePtrs();
}

protmodelist::~protmodelist()
{
	data.clear();
}

protmodelist::entry *protmodelist::wildMatch(const char *mask)
{
	ptrlist<entry>::iterator l = data.begin();

	while(l)
	{
		if(::wildMatch(mask, l->mask))
			return l;
		l++;
	}

	return NULL;
}

protmodelist::entry *protmodelist::conflicts(const char *mask)
{
	ptrlist<entry>::iterator i = data.begin();

	while(i)
	{
		if(::match(i->mask, mask))
			return i;
		i++;
	}
	return NULL;
}

protmodelist::entry *protmodelist::match(const chanuser *u)
{
	ptrlist<entry>::iterator l = data.begin();

	while(l)
	{
		if(matchBan(l->mask, u))
			return l;
		l++;
	}
	return NULL;
}

protmodelist::entry *protmodelist::add(const char *mask, const char *by, time_t when, time_t expires, const char *reason, bool sticky)
{
	entry *e = new entry(mask, by, reason, when, expires, sticky);
	data.addLast(e);
	return e;
}

int protmodelist::remove(const char *mask)
{
	ptrlist<entry>::iterator l = data.begin();

	if(*mask == '#')
	{
		if(!*mask || !isdigit(mask[1]))
			return 0;

		return remove(atoi(mask+1));
	}

	while(l)
	{
		if(!strcmp(mask, l->mask))
		{
			data.removeLink(l);
			return 1;
		}
		l++;
	}
	return 0;
}

int protmodelist::remove(int n)
{
	ptrlist<entry>::iterator l = data.getItem(n-1);

	if(l)
	{
		data.removeLink(l);
		return 1;
	}

	return 0;
}

int protmodelist::sendShitsToOwner(inetconn *c, const char *name, int i)
{
	//int i = start;
	ptrlist<entry>::iterator s = data.begin();

	if(s)
	{
		if(type == BAN)
			c->send(name, " shits: ", NULL);
		else if(type == INVITE)
			c->send(name, " invites: ", NULL);
		else if(type == EXEMPT)
			c->send(name, " exempts: ", NULL);
		else if(type == REOP)
			c->send(name, " reops: ", NULL);

		while(s)
		{
			++i;
			c->send(i > 99 ? "[\002" : i > 9 ? "[ \002" : "[  \002", itoa(i), "\002]\002:\002 ", s->mask,
					" (expires\002:\002 ", s->expires ?
					timestr("%d/%m/%Y %T", s->expires) : "never", ")", NULL);

			c->send(s->sticky ? "[ * ]  " : "       ", s->by, "\002:\002 ", s->reason, NULL);
			c->send("       created\002:\002 ", timestr("%d/%m/%Y %T", s->when), NULL);
			if(s->last_used)
				c->send("       last used\002:\002 ", timestr("%d/%m/%Y %T", s->last_used), NULL);

			s++;
		}
	}
	return i;
}

void protmodelist::sendToUserlist(inetconn *c, const char *name)
{
	ptrlist<entry>::iterator s = data.begin();
	char *botnet_cmd;

	while(s)
	{
		switch(type)
		{
			case BAN : if(s->sticky)
				   	botnet_cmd = S_ADDSTICK;
				   else
					botnet_cmd = S_ADDSHIT;
				   break;
			case INVITE : botnet_cmd = S_ADDINVITE; break;
			case EXEMPT : botnet_cmd = S_ADDEXEMPT; break;
			case REOP : botnet_cmd = S_ADDREOP; break;
			default : return;
		}

		c->send(botnet_cmd, " ", name, " ", s->mask, " ", s->by, " ",
				itoa(s->when), " ", itoa(s->expires), " ", s->reason, NULL);
		//if(s->last_used)
		//	c->send(S_LASTUSED_PROTMODE, " ", type, " ", itoa(s->last_used), NULL); 
		s++;
	}
}

int protmodelist::sendShitsToOwner(inetconn *c, int type, const char *channel, const char *)
{
	int matches = 0;
	CHANLIST *chLst;

	if((!channel || !*channel || !strcmp(channel, "*")) && c->checkFlag(HAS_N))
	{
		matches += userlist.protlist[type]->sendShitsToOwner(c, "Global");
	}

	if(channel && *channel)
	{
		foreachMatchingChanlist(chLst, channel)
		{
			if(c->checkFlag(HAS_N) || c->checkFlag(HAS_N, _i))
				matches += chLst->protlist[type]->sendShitsToOwner(c, chLst->name);
		}
	}

	if(!matches)
		c->send("No matches has been found", NULL);
	else
		c->send("--- Found ", itoa(matches), matches == 1 ? " match" : " matches", NULL);

	return matches;
}


protmodelist::entry *protmodelist::find(const char *str)
{
	ptrlist<entry>::iterator s = data.begin();

	if(str[0]=='#')
	{
		if(!isdigit(str[1]))
			return NULL;

		if((s=data.getItem(atoi(str+1)-1)))
			return s;
		else
			return NULL;	
	}

	while(s)
	{
		if(!strcmp(str, s->mask))
			return s;

		s++;
	}

	return NULL;
}

int protmodelist::expire(const char *channel)
{
	if(config.bottype != BOT_MAIN)
		return 0;

	ptrlist<entry>::iterator n, s = data.begin();
	int i=0;
	chan *ch;
	char *botnet_cmd, _mode[3], *type_str;

	_mode[0] = '-';
	_mode[1] = mode;
	_mode[2] = '\0';

	switch(type)
	{
		case BAN : botnet_cmd=S_RMSHIT; type_str="shit"; break;
		case INVITE : botnet_cmd=S_RMINVITE; type_str="invite"; break;
		case EXEMPT : botnet_cmd=S_RMEXEMPT; type_str="exempt"; break;
		case REOP : botnet_cmd=S_RMREOP; type_str="reop"; break;
		default : return 0;
	}

	while(s)
	{
		n = s;
		n++;
		if(s->expires && s->expires <= NOW)
		{
			net.send(HAS_B, botnet_cmd, " ", s->mask, " ", channel, NULL);

			if(channel)
			{
				net.send(HAS_N, "[*] ", type_str, " `\002", s->mask, "'\002 on `\002", channel, "\002' has expired", NULL);
	
				if((ch=ME.findChannel(channel)) && ch->synced() && ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
					ch->modeQ[PRIO_LOW].add(NOW+5, _mode, s->mask);
			}

			else
			{
				net.send(HAS_N, "[*] ", type_str, " `\002", s->mask, "'\002 has expired", NULL);

				foreachSyncedChannel(ch)
					if(ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
						ch->modeQ[PRIO_LOW].add(NOW+5, _mode, s->mask);
			}

			data.removeLink(s);
			++userlist.SN;
			++i;
		}
		s = n;
	}
	return i;
}

int protmodelist::expireAll()
{
	if(config.bottype != BOT_MAIN)
		return 0;

	CHANLIST *chLst;
	int i=0;

	foreachNamedChanlist(chLst)
	{
		i += chLst->protlist[BAN]->expire(chLst->name);
		i += chLst->protlist[INVITE]->expire(chLst->name);
		i += chLst->protlist[EXEMPT]->expire(chLst->name);
		i += chLst->protlist[REOP]->expire(chLst->name);
	}

	i += userlist.protlist[BAN]->expire();
	i += userlist.protlist[INVITE]->expire();
	i += userlist.protlist[EXEMPT]->expire();
	i += userlist.protlist[REOP]->expire();

	if(i)
		userlist.nextSave = NOW + SAVEDELAY;

	return i;
}

void protmodelist::clear()
{
	data.clear();
}

bool protmodelist::isSticky(const char *mask, int type, const chan *ch)
{
	entry *b = ch->protlist[type]->find(mask);

	if((b && b->sticky) || ((b = userlist.protlist[type]->find(mask)) && b->sticky))
		return true;

	return false;
}

protmodelist::entry *protmodelist::findSticky(const char *mask, int type, const chan *ch)
{
	entry *b = ch->protlist[type]->find(mask);

	if((b && b->sticky) || ((b = userlist.protlist[type]->find(mask)) && b->sticky))
		return b;

	return NULL;
}

protmodelist::entry *protmodelist::findByMask(const char *mask)
{
	ptrlist<entry>::iterator i = data.begin();

    while(i)
    {
        if(!strcmp(i->mask, mask))
            return i;
        i++;
    }
    return NULL;
}

protmodelist::entry *protmodelist::findBestByMask(const char *channel, const char *mask, int type)
{
    protmodelist *s;
	entry *e;

	int chanNum;

	if((chanNum = userlist.findChannel(channel)) != -1)
	{
    	s = userlist.chanlist[chanNum].protlist[type];
		e = s->findByMask(mask);
		if(e)
			return e;
	}

	return userlist.protlist[type]->findByMask(mask);
}

protmodelist::entry *protmodelist::updateLastUsedTime(const char *channel, const char *mask, int type)
{
	entry *e = findBestByMask(channel, mask, type);
	if(e)
	{
		e->last_used = NOW;
		DEBUG(printf("[D] Updating SHIT last use: %s\n", e->mask));
		userlist.SN++;
	}
	return e;
}
