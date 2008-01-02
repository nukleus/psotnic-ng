/***************************************************************************
 *   Copyright (C) 2003-2006 by Grzegorz Rusin                             *
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

flagTable FT[] = {
    { 'x',  HAS_X,  7,  X_FLAGS,    "perms",            "can do everything" },
    { 's',  HAS_S,  6,  S_FLAGS,    "super owners",     "can manage bots" },
    { 'n',  HAS_N,  5,  N_FLAGS,    "owners",           "can change channel modes"},
    { 'm',  HAS_M,  4,  M_FLAGS,    "masters",          "can op one person at a time"},
    { 'f',  HAS_F,  3,  F_FLAGS,    "friends",          "friends; can be invited by bots; can recive server op" },
    { 'o',  HAS_O,  2,  0,          "ops",              "can be oped by bots; cannot recieve server op" },
    { 'v',  HAS_V,  1,  0,          "voices",           "combined with 'a' flag gives autovoice" },
    { 'r',  HAS_R,  0,  HAS_O,      "reops",            "instant reop if deoped" },
    { 'a',  HAS_A,  0,  0,          "auto mode",        "auto op/voice" },
    { 'i',  HAS_I,  0,  0,          "auto invite",      "auto invite when kicked out" },
    { 'e',  HAS_E,  0,  0,          "idiot exempted",         "ignored by idiots check" },
    { 'c',  HAS_C,  0,  0,          "clone exempted",   "ignored by clone check" },
    { 't',  0,      0,  0,          "ignored",          "ignored; left for compatibility reasons" }, 
    { 'p',  HAS_P,  0,  0,          "partyline",        "allowed to access partyline, level is defined by 'n', 's', 'x' flags" },
    { 'q',  HAS_Q, -1,  0,          "quiet",            "not allowed to have voice" },
    { 'd',  HAS_D, -2,  0,          "deoped",           "not allowed to have op" },
    { 'k',  HAS_K, -3,  0,          "kicked",           "kick on join" },
    { 0,    0,      0,  0,          NULL,               NULL }
};

flagTable BFT[] = {
    { 'b',  HAS_B,          6,  0,          "bot",      "" },
    { 's',  HAS_H | HAS_L,  0,  HAS_B,      "slave",    "" },
    { 'l',  HAS_L,          0,  HAS_B,      "leaf",     "" },
    { 'h',  HAS_H,          0,  HAS_B,      "main",     "" },
    { 'p',  HAS_P,          0,  HAS_B,      "dynamic",  "dynamic host" },
    { 'y',  0,              0,  0,          "",         "ignored; left for compatibility reasons" },
    { ',',  0,              0,  0,          "",         "ignored; left for compatibility reasons" },
    { 0,    0,              0,  0,          NULL,       NULL }
};


/*
 * finds flag by letter in FLAG table
 * @return pointer to flagTable entry, otherwise NULL
 */
flagTable *ul::findFlagByLetter(char letter, flagTable *ft)
{
    while(ft->letter)
    {
        if(ft->letter == letter)
            return ft;

        ++ft;
    }
        
    return NULL;
}

/*
 * merges \flags with \str of letters prefixed with + or - (eg. +af-t)
 * @return true if \str consists of valid flags
 */
bool ul::mergeFlags(unsigned int &flags, const char *str)
{
    //two special cases
    if(!strcmp(str, "-"))
    {
        flags = 0;
        return true;
    }

    if(!strcmp(str, "*"))
        return true;
    
    bool plus = true;
    bool bot = flags & HAS_B;
    flagTable *ft;

    if(!strchr(str, '+') && !strchr(str, '-'))
    {
	if(!bot)
	    flags = 0;
//	else
//	    flags = HAS_B; 
    }
    
    while(*str)
    {
        switch(*str)
        {
            case '+':
                plus = true;
                break;
            case '-':
                plus = false;
                break;
            default:
                ft = findFlagByLetter(*str, bot ? BFT : FT);

                if(!ft)
                {
                    if(bot)
                    {
                        ++str;
                        continue;
                    }
                    else
                        return false;
                }
                
                if(plus)
                    flags |= ft->flag | ft->enforced;
                else
                {
                    flagTable *x = bot ? BFT : FT;

                    while(x->letter)
                    {
                        if(ft->flag & x->enforced)
                            flags &= ~x->flag;
                        ++x;
                    }

                    flags &= ~ft->flag;
                }
        }
        ++str;
    }
    return true;
}

void ul::sendDSetsToBots(const char *channel)
{
	pstring<> prefix(S_CHSET);
	prefix += " ";
	prefix += channel;

	for(int i=0; i<net.max_conns; ++i)
		if(net.conn[i].isRegBot())
			dset->sendToFile(&net.conn[i], prefix);
}

CHANLIST *ul::findChanlist(const char *name)
{
	int i;
	i = findChannel(name);
	if(i != -1)
		return &chanlist[i];
	else
		return NULL;
}

HANDLE *ul::me()
{
	return first->next;
}

int ul::isLeaf(const HANDLE *h)
{
	return isBot(h) && ((h->flags[GLOBAL] & (HAS_H + HAS_L)) == HAS_L);
}

int ul::isSlave(const HANDLE *h)
{
	return isBot(h) && ((h->flags[GLOBAL] & (HAS_H + HAS_L)) == (HAS_H + HAS_L));
}

int ul::isMain(const HANDLE *h)
{
	return isBot(h) && ((h->flags[GLOBAL] & (HAS_H | HAS_L)) == HAS_H);
}

void ul::sendHandleInfo(inetconn *c, const HANDLE *h, const char *mask)
{
        int i, n;
	char *a, buf[MAX_LEN];

	c->send("Matching ", userlist.isBot(h) ? "bot '\002" : "user '\002", h->name, "\002'", NULL);
	if(!userlist.isBot(h))
	{
		userlist.flags2str(h->flags[MAX_CHANNELS], buf);
		c->send("global flags:\002 ", buf, "\002", !isNullString((char *) h->pass, 16) ? ", password is set" : "", NULL);
	}
	else if(c->checkFlag(HAS_S))
	{
		userlist.flags2str(h->flags[MAX_CHANNELS], buf);
		c->send("flags: \002", buf, "\002, ip:\002 ", inet2char(h->ip), "\002", !isNullString((char *) h->pass, 16) ? ", password is set" : "", (ul::isMain(h) || net.findConn(h)) ? ", linked" : ", \002not\002 linked", NULL);
	}
	else
	{
		c->send(S_NOPERM, NULL);
		return;
	}

	a = push(NULL, "channel flags: ", NULL);
	for(n=i=0; i<MAX_CHANNELS; ++i)
	{
		if(h->flags[i] && userlist.chanlist[i].name)
		{
			userlist.flags2str(h->flags[i], buf);
			if(!n) a = push(a, "", (const char *) userlist.chanlist[i].name, "(\002", buf, "\002)", NULL);
			else a = push(a, "\002,\002 ", (const char *) userlist.chanlist[i].name, "(\002", buf, "\002)", NULL);
			++n;
		}
	}
	if(n) c->send(a, NULL);
	free(a);

	if(h->info && h->info->data.entries())
	{
		ptrlist<comment::entry>::iterator e = h->info->data.begin();
		a = NULL;

		while(e)
		{
			a = push(a, a ? (char *) "\002,\002 " : (char *) " ", e->key, ": ",
				 e->value, NULL);
			e++;
		}
		c->send(a+1, NULL);
		free(a);
	}
		
	if(h->createdBy)
		c->send("created at ", h->creation->ctime(), " by ", h->createdBy, NULL);
	else
		c->send("created at ", h->creation->ctime(), NULL);

	if(h->history && h->history->data.entries() && MAX_WHOIS_OFFENCES > 0)
	{
	    char flags1[32], flags2[32];
	    ptrlist<offence::entry>::iterator e = h->history->data.begin();
	    i = 0;
	    
	    c->send("offence history: ", NULL);
	    while(e && i < MAX_WHOIS_OFFENCES)
	    {
		userlist.flags2str(e->fromFlags, flags1);
		userlist.flags2str(e->toFlags, flags2);
		snprintf(buf, MAX_LEN, "[%3d]: %s(%d): %s\n       %s flags decreased from `%s' to `%s'\n       Created: %s", 
		++i, e->chan, e->count, e->mode, e->global ? "Global" : "Channel", flags1, flags2, timestr("%d/%m/%Y %T", e->time));
		c->send(buf, NULL);
		e++;
	    }
	}

	if(isBot(h))
	{
		Pchar chans;
		chans.clean();

		for(i=0; i<MAX_CHANNELS; ++i)
		{
			if(isRjoined(i, h))
			{
				if(chans.len + strlen(chanlist[i].name) > 50)
				{
					c->send("channels: ", chans.data, NULL);
					chans.clean();
				}

				if(chans.len)
					chans.push(", ");
				chans.push((const char *) chanlist[i].name);
			}
		}
		if(chans.len)
			c->send("channels: ", chans.data, NULL);
	}

	for(i=0, n=0; i<MAX_HOSTS; i++)
	{
		if(h->host[i])
		{
			if(isBot(h) && i == MAX_HOSTS-1)
			{
				if((mask && *mask) ? (match(h->host[i], mask) || match(mask, h->host[i])) : 1)
					sprintf(buf, "[\002tmp\002]: ");
				else
					sprintf(buf, "[tmp]: ");
				++n;
			}
			else
			{
				if((mask && *mask) ? (match(h->host[i], mask) || match(mask, h->host[i])) : 1)
					sprintf(buf, "[\002#%2d\002]: ", ++n);
				else
					sprintf(buf, "[#%2d]: ", ++n);
			}
			if(h->hostBy[i]) c->send(buf, h->host[i], " (", h->hostBy[i], ")", NULL);
			else c->send(buf, h->host[i], NULL);
		}
	}
	if(!n) c->send("No hosts has been found", NULL);
}

HANDLE *ul::changeIp(char *user, char *ip)
{
	HANDLE *h = findHandle(user);

    if(h && isBot(h))
    {
		unsigned int addr = inet_addr(ip);
		if(!strcmp(inet2char(addr), ip))
		{
			h->ip = addr;
			return h;
		}
	}
	return NULL;
}

/* by Pawe³ (Googie) Salawa, boogie@myslenice.one.pl */
void ul::sendBotTree(inetconn *c)
{
	int slaves = 0;

	for (int i = 0; i < net.max_conns; ++i )
    {
        if (net.conn[i].isSlave())
            slaves++;
    }

	c->send(config.handle, " (has ", itoa(slaves), " slave",
		slaves == 1 ? (char *) "" : (char *) "s", ")", NULL);

    for ( int i = 0; i < net.max_conns; ++i )
    {
        if (net.conn[i].isSlave())
        {
        	if(net.conn[i].isRedir())
        		c->send("[!] ", net.conn[i].handle->name, " has slave flags but is not linked to me", NULL);

            int leafs = 0;
            for ( int j = 0; j < net.max_conns; ++j )
            {
                if (net.conn[j].isLeaf() && net.conn[j].fd == net.conn[i].fd)
                {
                	if(!net.conn[j].isRedir())
        				c->send("[!] ", net.conn[j].handle->name, " has leaf flags, but is linked to me", NULL);
        			leafs++;
        		}
            }

			slaves--;
			c->send(slaves ? (char *) " |" : (char *) " `", "-", net.conn[i].handle->name,
				" (has ", itoa(leafs), " leaf", leafs == 1 ? (char *) "" : (char *) "s", ")", NULL);

            for ( int j = 0; j < net.max_conns; ++j )
            {
                if (net.conn[j].isLeaf() && net.conn[j].fd == net.conn[i].fd)
                {
                    leafs--;
					if (slaves)
					   	c->send(leafs ? (char *) " |   |" : (char *) " |   `", "-", net.conn[j].handle->name, NULL);
					else
                    	c->send(leafs ? (char *) "     |" : (char *) "     `", "-", net.conn[j].handle->name, NULL);
			    }
			}
        }
    }
	int n = net.bots() + 1;

	c->send("[*] ", itoa(n), " bot",
		n == 1 ? (char *) "" : (char *) "s", " on-line", NULL);

}
/* end of by Googie */

void ul::cleanChannel(int i)
{
    if(!chanlist[i].name) return;
	chanlist[i].pass = "";

	delete chanlist[i].chset;
	delete chanlist[i].protlist[BAN];
	delete chanlist[i].protlist[INVITE];
	delete chanlist[i].protlist[EXEMPT];
	delete chanlist[i].protlist[REOP];

	chanlist[i].chset  = new chanset;
	chanlist[i].protlist[BAN] = new protmodelist(BAN, 'b');
	chanlist[i].protlist[INVITE] = new protmodelist(INVITE, 'I');
	chanlist[i].protlist[EXEMPT] = new protmodelist(EXEMPT, 'e');
	chanlist[i].protlist[REOP] = new protmodelist(REOP, 'R');

	chan *ch = ME.findNotSyncedChannel(chanlist[i].name);
	if(ch)
	{
		ch->chset = chanlist[i].chset;
		ch->protlist[BAN] = chanlist[i].protlist[BAN];
		ch->protlist[INVITE] = chanlist[i].protlist[INVITE];
		ch->protlist[EXEMPT] = chanlist[i].protlist[EXEMPT];
		ch->protlist[REOP] = chanlist[i].protlist[REOP];
	}
	chanlist[i].updated = 0;
}

void ul::cleanHandle(HANDLE *h)
{
	int i=0;

	memset(h->pass, 0, 16);

	for(i=0; i<MAX_HOSTS; ++i)
	{
		if(h->host[i])
		{
			free(h->host[i]);
			h->host[i] = NULL;
		}
	}

	h->updated = 0;

	for(i=0; i<MAX_CHANNELS; ++i)
	{
		h->flags[i] = 0;
	}
	h->flags[MAX_CHANNELS] = (h->flags[MAX_CHANNELS] & HAS_B) ? B_FLAGS : 0;
}

int ul::parse(char *data)
{
	char arg[10][MAX_LEN];
	char *marg;
	HANDLE *h;

	if(!data || !strlen(data)) return 0;

	str2words(arg[0], data, 10, MAX_LEN);

	if((!strcmp(arg[0], S_ADDUSER) || !strcmp(arg[0], S_ADDBOT) ||
		!strcmp(arg[0], "ADDUSER") || !strcmp(arg[0], "ADDBOT"))
		&& strlen(arg[3]))
	{
		if((h = userlist.findHandle(arg[1])))
		{
        		ptime t(arg[2], arg[3]);

			if(!memcmp(&t.tv, &h->creation->tv, sizeof(timeval)))
			{
            			h->updated = 1;
				return 0;
			}
			else
			{
            			userlist.removeHandle(arg[1]);
			}
		}

		if(!strcmp(arg[0], S_ADDUSER) || !strcmp(arg[0], "ADDUSER")) userlist.addHandle(arg[1], 0, 0, arg[2], arg[3], arg[4]);
		else userlist.addHandle(arg[1], inet_addr(arg[4]), B_FLAGS, arg[2], arg[3], arg[5]);
    		h = userlist.findHandle(arg[1]);
    		if(h)
		{
			h->updated = 1;
			delete h->creation;
			h->creation = new ptime(arg[2], arg[3]);
        		return 1;
		}
		return 0;
	}
	if((!strcmp(arg[0], S_ADDHOST) || !strcmp(arg[0], "ADDHOST")) && strlen(arg[2]))
	{
		h = userlist.findHandle(arg[1]);
		if(h)
		{
			userlist.addHost(h, arg[2], arg[3]);
			ME.nextRecheck = NOW + 5;
        		return 1;
		}
		return 0;
	}
	if((!strcmp(arg[0], S_ADDCHAN) || !strcmp(arg[0], "ADDCHAN")) && strlen(arg[2]))
	{
      		userlist.addChannel(arg[2], arg[3], arg[1]);

		int i = userlist.findChannel(arg[2]);
		if(i > -1)
		{
			ME.rejoin(arg[2], atoi(arg[4]));
			chanlist[i].updated = 1;
			return 1;
		}
		return 0;
	}
	if(!strcmp(arg[0], S_RMUSER) && strlen(arg[1]))
	{
		userlist.removeHandle(arg[1]);
		ME.nextRecheck = NOW + 5;
		return 1;
	}
	if(!strcmp(arg[0], S_RMHOST) && strlen(arg[2]))
	{
		h = userlist.findHandle(arg[1]);
		if(h)
		{
			userlist.removeHost(h, arg[2]);
			ME.nextRecheck = NOW + 5;
			return 1;
		}
		return 0;
	}
	if(!strcmp(arg[0], S_RMCHAN) && strlen(arg[1]))
	{
		userlist.removeChannel(arg[1]);
		net.irc.send("PART ", arg[1], " :", (const char *) config.partreason, NULL);
		return 1;
   	}

	/* other */
	if((!strcmp(arg[0], S_CHATTR) || !strcmp(arg[0], "CHATTR")) && strlen(arg[2]))
	{
		userlist.changeFlags(arg[1], arg[2], arg[3]);
		ME.nextRecheck = NOW + 5;
		return 1;
	}
	if(!strcmp(arg[0], S_ULSAVE))
	{
		userlist.save(config.userlist_file);
		return 0;
	}

	if((!strcmp(arg[0], S_SET) || !strcmp(arg[0], "SET")) && strlen(arg[2]))
	{
		set.setVariable(arg[1], arg[2]);
		return 1;
	}
	if((!strcmp(arg[0], S_CHSET) || !strcmp(arg[0], "CHSET")) && strlen(arg[3]))
	{
		int i = userlist.findChannel(arg[1]);
		if(i != -1)
		{
			marg = srewind(data, 3);		
    			userlist.chanlist[i].chset->setVariable(arg[2], marg ? marg : "");
			return 1;
		}
		return 0;

	}
	if(!strcmp(arg[0], S_DSET))
	{
		marg = srewind(data, 2);		
		dset->setVariable(arg[1], marg ? marg : "");
		return 1;
	}
	if(!strcmp(arg[0], S_GCHSET) && strlen(arg[2]))
	{
		marg = srewind(data, 2);
		userlist.globalChset(NULL, arg[1], marg ? marg : "");
		return 1;
	}
	if((!strcmp(arg[0], S_PASSWD) || !strcmp(arg[0], "PASSWD")) && strlen(arg[2]))
	{
		userlist.setPassword(arg[1], arg[2]);
		return 1;
	}
	if((!strcmp(arg[0], S_ADDR) || !strcmp(arg[0], "ADDR")) && strlen(arg[2]))
	{
		userlist.changeIp(arg[1], arg[2]);
		return 1;
	}
	if((!strcmp(arg[0], S_SN) || !strcmp(arg[0], "SN")) && strlen(arg[1]))
	{
		SN = strtoul(arg[1], 0, 10);
		return 0;
	}
	if(!strcmp(arg[0], S_RJOIN) && strlen(arg[2]))
	{
		char *a = srewind(data, 2);
		char buf[MAX_LEN];
		int i;

		while(a && *a)
		{
			for(i=0; i<MAX_LEN && *a; ++i, ++a)
			{
				if(*a == ' ')
				{
					buf[i] = '\0';
					userlist.rjoin(arg[1], buf);
					++a;
					break;
				}
				buf[i] = *a;
			}
		}
		return 1;
	}
	if(!strcmp(arg[0], S_CHHANDLE) && strlen(arg[2]))
	{
		HANDLE *h = userlist.findHandle(arg[1]);
		if(h)
		{
			free(h->name);
			mem_strcpy(h->name, arg[2]);
			return 1;
		}
		return 0;
	}
	if(!strcmp(arg[0], S_PROXYHOST) && strlen(arg[1]))
	{
		HANDLE *h = userlist.findHandle(arg[1]);
		if(h && userlist.isBot(h))
		{
			userlist.addHost(h, arg[2], NULL, 0, MAX_HOSTS-1);
			return 1;
		}
		return 0;
	}
	if(!strcmp(arg[0], S_ADDINFO) && strlen(arg[3]))
	{
		HANDLE *h = userlist.findHandle(arg[1]);
		if(h)
		{
			if(!h->info) h->info = new comment;
			char *a = srewind(data, 3);
			h->info->add(arg[2], a);
		}
		return 0;
	}	
	if(!strcmp(arg[0], S_ADDOFFENCE) && strlen(arg[8]))
	{
		HANDLE *h = userlist.findHandle(arg[1]);
		if(h)
		{
			if(!h->history) h->history = new offence;
			char *a = srewind(data, 8);
			h->history->add(arg[2], a, atol(arg[3]), atoi(arg[4]), atoi(arg[5]), atoi(arg[6]), atoi(arg[7]) ? true : false);
		}
		return 0;
	}	
	int stick;
	if(((stick = !strcmp(arg[0], S_ADDSTICK)) || !strcmp(arg[0], S_ADDSHIT) || (stick=!strcmp(arg[0], S_ADDINVITE)) || (stick=!strcmp(arg[0], S_ADDEXEMPT)) || (stick=!strcmp(arg[0], S_ADDREOP))) && strlen(arg[5]))
	{
		chan *ch;
		protmodelist::entry *s;
		int type = -1;

		if(!strcmp(arg[0], S_ADDSTICK) || !strcmp(arg[0], S_ADDSHIT))
			type = BAN;

		else if(!strcmp(arg[0], S_ADDINVITE))
			type = INVITE;

		else if(!strcmp(arg[0], S_ADDEXEMPT))
			type = EXEMPT;

		else if(!strcmp(arg[0], S_ADDREOP))
			type = REOP;

		if(!strcmp(arg[1], "*"))
		{
			s = protlist[type]->add(arg[2], arg[3], atol(arg[4]), atol(arg[5]), srewind(data, 6), stick);
			if(type == BAN && s)
			{
				foreachSyncedChannel(ch)
					ch->applyShit(s);
			}
		}
		else
		{
			int i = findChannel(arg[1]);
			if(i != -1)
			{
				s = chanlist[i].protlist[type]->add(arg[2], arg[3], atol(arg[4]), atol(arg[5]), srewind(data, 6), stick);

				if(type == BAN && s && (ch = ME.findChannel(arg[1])))
					ch->applyShit(s);
			}
		}
		return 1;
	}
	if((!strcmp(arg[0], S_RMSHIT) || !strcmp(arg[0], S_RMINVITE) || !strcmp(arg[0], S_RMEXEMPT) || !strcmp(arg[0], S_RMREOP)) && strlen(arg[1]))
	{
		chan *ch;
		protmodelist::entry *s;
		char mode[3];
		int type = -1;

		if(!strcmp(arg[0], S_RMSHIT))
			type = BAN;
		else if(!strcmp(arg[0], S_RMINVITE))
			type = INVITE;
		else if(!strcmp(arg[0], S_RMEXEMPT))
			type = EXEMPT;
		else if(!strcmp(arg[0], S_RMREOP))
			type = REOP;

		if(strlen(arg[2]))
		{
			int i = findChannel(arg[2]);
			if(i != -1)
			{
				if((s=chanlist[i].protlist[type]->find(arg[1])) && (ch=ME.findChannel(arg[2])))
				{
					if(ch->synced() && ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
					{
						mode[0] = '-';
						mode[1] = chanlist[i].protlist[type]->mode;
						mode[2] = '\0';
						ch->modeQ[PRIO_LOW].add(NOW+5, mode, s->mask);
					}
				}

				return chanlist[i].protlist[type]->remove(arg[1]);
			}
		}
		else
		{
			if((s=protlist[type]->find(arg[1])))
			{
				foreachSyncedChannel(ch)
				{
					if(ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
					{
						mode[0] = '-';
						mode[1] = protlist[type]->mode;
						mode[2] = '\0';
						ch->modeQ[PRIO_LOW].add(NOW+5, mode, s->mask);
					}
				}
			}

			return protlist[type]->remove(arg[1]);
		}
		return 1;
	}
	return 0;
}

void ul::sendToAll()
{
	int i;
	for(i=0; i<net.max_conns; ++i)
		if(net.conn[i].isRegBot() && net.conn[i].fd > 0 && !net.conn[i].isRedir())
			send(&net.conn[i]);
}

int ul::hasEmptyFlags(const HANDLE *h) const
{
	int i;

	for(i=0; i<MAX_CHANNELS+1; ++i)
		if(h->flags[i]) return 0;

	return 1;
}


int ul::userLevel(const HANDLE *h, int n) const
{
	if(!h)
        return 0;

    flagTable *ft = FT;
    
    while(ft->letter)
    {
        if(h->flags[n] & ft->flag)
            return ft->level;
        ft++;
    }
	return 0;
}

int ul::userLevel(int flags) const
{
    flagTable *ft = FT;

    while(ft->letter)
    {
        if(flags & ft->flag)
            return ft->level;
        ft++;
    }
    return 0;
}

void ul::reportNewOffences(inetconn *c, const bool showCmd)
{
    HANDLE *h = first;
    int count = 0;
    pstring<128> buf = "";
    
    while(h)
    {
	if(h->history && h->history->data.entries())
	{
	    if(hasReadAccess(c, h))
	    {
		if(buf)
		    buf += "\002,\002 ";
		buf += h->name;
		count++;
	    }
	}
	h = h->next;
    }
    if(showCmd)
	net.sendCmd(c, "offences", NULL);
    
    if(count)
	c->send("New offences (\002", itoa(count), "\002)\002:\002 ", (const char *) buf, NULL);
    else
    {
	if(showCmd) c->send("No new offences found", NULL);
    }
}

void ul::sendUsers(inetconn *c)
{
	HANDLE *h;
	int i, count;
    pstring<128> buf;
    unsigned int not_flags;
    flagTable *ft;
    bool send_chan;
        
    for(i=GLOBAL; i != -1; --i)
	{
		if(i == GLOBAL)
		{
			if(c->checkFlag(HAS_N))
                c->send("--- \002global users\002 ---", NULL);
			else
                continue;

            send_chan = false;
		}
		else if(!chanlist[i].name)
            continue;
        else
            send_chan = true;
        
        ft = FT;
        not_flags = 0;
        
        while(ft->letter)
        {
            h = first;
            count = 0;
            buf = "";
            
            while(h)
		    {
			    if(!(h->flags[GLOBAL] & HAS_B) && hasReadAccess(c, h) &&
                            h->flags[i] & ft->flag && !(h->flags[i] & (not_flags)))
                {
                    ++count;
                    if(buf)
                        buf += "\002,\002 ";
                    buf += h->name;
				}
			    h = h->next;
		    }
            
            if(count)
            {
                if(send_chan)
                {
                    c->send("--- \002", (const char *) chanlist[i].name, "\002 users ---", NULL);
                    send_chan = false;
                }
                c->send(ft->desc, " (\002", itoa(count), "\002)\002:\002 ", (const char *) buf, NULL);
                send_chan = false;
            }
            not_flags |= ft->flag;
            ++ft;
        }

		if(i == GLOBAL)
		{
			/* special case, no flags */
			h = first;
        	count = 0;
        	buf = "";
		
			while(h)
			{
				if(!h->flags[GLOBAL])
				{
					++count;
					if(buf)
                		buf += "\002,\002 ";
                	buf += h->name;
            	}
            	h = h->next;
			}
		
			if(count)
				c->send("no flags (\002", itoa(count), "\002)\002:\002 ", (const char *) buf, NULL);
		}
	}
}

void ul::autoSave(int notice)
{
	if(nextSave && config.userlist_file && nextSave <= NOW)
	{
		save(config.userlist_file);
		nextSave = 0;
		if(config.bottype == BOT_MAIN && notice)
			net.send(HAS_N, "[*] Autosaving userlist", NULL);
		ME.recheckFlags();
	}
}

int ul::wildFindHostExtBan(const HANDLE *p, const char *host) const
{
	int i;

	for(i=0; i<MAX_HOSTS; ++i)
		if(p->host[i] && matchBanMask(host, p->host[i], 1))
			return i;
	return -1;
}


int ul::wildFindHostExt(const HANDLE *p, const char *host) const
{
	int i;

	for(i=0; i<MAX_HOSTS; ++i)
		if(p->host[i] && (match(host, p->host[i]) || match(p->host[i], host)))
			return i;
	return -1;
}

int ul::wildFindHost(const HANDLE *p, const char *host) const
{
	int i;

	for(i=0; i<MAX_HOSTS; ++i)
		if(p->host[i] && match(p->host[i], host)) return i;
	return -1;
}


HANDLE *ul::matchPassToHandle(const char *pass, const char *host, const int flags) const
{
	HANDLE *h = first;

	while(h)
	{
		if(!isBot(h) && ((h->flags[GLOBAL] & flags) == flags) &&
			MD5Validate(h->pass, pass, strlen(pass)) &&
			wildFindHost(h, host) != -1)
				return h;

		h = h->next;
	}
	return NULL;
}

int ul::hasReadAccess(inetconn *c, HANDLE *h)
{
	if(h == c->handle)
		return 1;

	if(c->checkFlag(HAS_S))
		return 1;

	if(c->checkFlag(HAS_N))
	{
		return isBot(h) ? 0 : 1;
	}

	if(c->checkFlag(HAS_P) && !h->flags[GLOBAL] && !hasEmptyFlags(h))
	{
		for(int i=0; i<MAX_CHANNELS; ++i)
			if(h->flags[i] && !c->checkFlag(HAS_N, i))
				return 0;

		return 1;
	}
	return 0;
}

int ul::hasReadAccess(inetconn *c, char *handle)
{
	HANDLE *h = findHandle(handle);
	return h ? hasReadAccess(c, h) : -1;
}

int ul::hasWriteAccess(inetconn *c, char *handle)
{
	HANDLE *h = findHandle(handle);
	return h ? hasWriteAccess(c, h) : -1;
}

int ul::hasWriteAccess(inetconn *c, HANDLE *h)
{
	if(h == c->handle)
		return 1;

	if(c->checkFlag(HAS_N))
	{
		if(c->checkFlag(HAS_X))
			return 1;

		if(c->checkFlag(HAS_S))
		{
			if(!(h->flags[GLOBAL] & HAS_S)) return 1;
			else return 0;
		}

		if(!(h->flags[GLOBAL] & HAS_N)) return 1;
		else return 0;
	}
	else if(c->checkFlag(HAS_P) && !h->flags[GLOBAL] && !hasEmptyFlags(h))
	{
		for(int i=0; i<MAX_CHANNELS; ++i)
			if(h->flags[i] && !c->checkFlag(HAS_N, i))
				return 0;

		return 1;
	}

	return 0;
}

void ul::setPassword(char *user, char *pass)
{
	if(strlen(pass) != 32) return;

	HANDLE *h = findHandle(user);
	if(h)
	{
		quoteHex(pass, h->pass);
		nextSave = NOW + SAVEDELAY;
	}
}

HANDLE *ul::changePass(char *user, char *pass)
{
	HANDLE *h = findHandle(user);
	if(h)
	{
		MD5Hash(h->pass, pass, strlen(pass));
		nextSave = NOW + SAVEDELAY;
	}
	return h;
}


HANDLE *ul::checkPartylinePass(const char *username, const char *pass, int flags)
{
	HANDLE *h = findHandle(username);

	if(h && (h->flags[GLOBAL] & HAS_P) && ((h->flags[GLOBAL] & flags) == flags) &&
		!isBot(h) && MD5Validate(h->pass, pass, strlen(pass)))
			return h;
	else
		return NULL;
}

int ul::isIdiot(const char *mask, int channum) const
{
	if(HAS_D & (first->flags[MAX_CHANNELS] | first->flags[channum]))
	{
		for(int i=0; i<MAX_HOSTS; ++i)
			if(first->host[i])
				if(match(first->host[i], mask))
					return 1;
	}
	return 0;
}

bool ul::globalChset(inetconn *c, const char *var, const char *value, int *index)
{
	int i, updated = 0;
	options::event *e;

	for(i=0; i<MAX_CHANNELS; ++i)
	{
		if(chanlist[i].name)
		{
			e = chanlist[i].chset->setVariable(var, value);

			if(c && !updated)
			{
				if(e->ok)
				{
					net.sendCmd(c, "gset ", e->entity->print(), NULL);
					/* compatybylity reasons */
					if(index != NULL && *index == -1) *index = i;
				}
				c->send("gset: ", (const char *) e->reason, NULL);
			}

			if(!e->ok)
				return 0;

			updated = 1;
		}
	}
	if(!updated)
	{
		if(c)
			c->send("gset: I dont have any channels in my list", NULL);

		return 0;
	}

	nextSave = NOW + SAVEDELAY;
	return 1;
}

int ul::rpart(const char *bot, const char *channel)
{
	HANDLE *b = findHandle(bot);
	if(!b || !isBot(b))
		return -1;

	int n = findChannel(channel);
	if(n == -1) return -2;

	b->channels &= ~int(pow(2.0, double(n)));
	chanlist[n].status |= PRIVATE_CHAN;

	if(!strcmp(bot, config.handle))
	{
		net.send(HAS_N, "[*] Parting from ", channel, NULL);
		if(ME.findNotSyncedChannel(channel))
		{
			net.irc.send("PART ", channel, " :", (const char *) config.partreason, NULL);
			penalty += 5;
		}
	}
	return n;
}

int ul::rjoin(const char *bot, const char *channel)
{
	HANDLE *b = findHandle(bot);
	if(!b || !isBot(b))
		return -1;

	int n = findChannel(channel);
	if(n == -1) return -2;

	b->channels |= int(pow(2.0, double(n)));

	if(!strcmp(bot, config.handle) && !ulbuf)
	{
		if(net.irc.status & STATUS_REGISTERED)
			net.send(HAS_N, "[*] Joining ", channel, NULL);
		else
			net.send(HAS_N, "[*] Joining ", channel, " (wtf? i am not on irc)", NULL);
	}
	return n;
}

int ul::isRjoined(int i, const HANDLE *h)
{
	if(!h)
		h = first->next; //me

	return h->channels & int(pow(2.0, double(i)));
}

int ul::addChannel(const char *name, const char *pass, const char *attr)
{
	int j, i, bit;
	HANDLE *h = first;

	i = j = findChannel(name);

	if(i == -1)
	{
		/* add new channel */
		for(i=0; i<MAX_CHANNELS; ++i)
		{
			if(!chanlist[i].name)
			{
				chanlist[i].name = name;
				chanlist[i].pass = pass;
				chanlist[i].chset = new chanset();
				chanlist[i].wasop = new wasoptest(set.WASOP_CACHE_TIME);
				chanlist[i].protlist[BAN] = new protmodelist(BAN, 'b');
				chanlist[i].protlist[INVITE] = new protmodelist(INVITE, 'I');
				chanlist[i].protlist[EXEMPT] = new protmodelist(EXEMPT, 'e');
				chanlist[i].protlist[REOP] = new protmodelist(REOP, 'R');

				goto added;
			}
		}
		return -2;
	}

	added:
	if(strchr(attr, 'T'))
		chanlist[i].chset->TAKEOVER = 1;

	if(strchr(attr, 'P'))
	{
		chanlist[i].status |= PRIVATE_CHAN;
		bit = 0;
	}
	else
	{
		bit = int(pow(2.0, double(i)));
		chanlist[i].status &= ~PRIVATE_CHAN;
	}

	while(h)
	{
		//clean flags only when new channel is created
		if(j == -1) h->flags[i] = 0;

		if(isBot(h) && bit)
			h->channels |= bit;
		h = h->next;
	}

	nextSave = NOW + SAVEDELAY;

#ifdef HAVE_MODULES
	if(chanlist[i].customDataConstructor)
		chanlist[i].customDataConstructor(&chanlist[i]);
#endif
	return i;
}

int ul::findChannel(const char *name) const
{
	int i;

	for(i=0; i<MAX_CHANNELS; ++i)
		if(chanlist[i].name)
			if(!strcasecmp(chanlist[i].name, name)) return i;
	return -1;
}

int ul::removeChannel(const char *name, char *removedName)
{
	int i=-1, bit;
	HANDLE *h = first;

	if(isdigit(*name))
	{
		int j = 0;
		int n = 0;
		int fin = atoi(name);

		if(fin < 1 || fin > MAX_CHANNELS)
			return 0;

		for(n=0; n<MAX_CHANNELS; ++n)
		{
			if(chanlist[n].name)
			{
				++j;
				if(j == fin)
				{
					i = n;
					break;
				}
			}
		}
	}
	else
	{
		i = findChannel(name);
	}

	if(i == -1) return 0;

	if(removedName)
		strncpy(removedName, chanlist[i].name, MAX_LEN);

	ME.removeChannel(chanlist[i].name);
	bit = int(pow(2.0, double(i)));

	while(h)
	{
		h->flags[i] = 0;
		if(isBot(h) && bit)
			h->channels &= ~bit;
		h = h->next;
	}

	chanlist[i].reset();

	nextSave = NOW + SAVEDELAY;
	return 1;
}

void ul::update()
{
	int n;
	char *b = ulbuf->data;
	char *a = NULL;
	HANDLE *q, *h = first->next;
    char buf[MAX_LEN];

	set.reset();

	while(h)
	{
        cleanHandle(h);
		h = h->next;
	}

	for(n=0; n<MAX_CHANNELS; ++n)
		cleanChannel(n);

	protlist[BAN]->clear();
	protlist[INVITE]->clear();
	protlist[EXEMPT]->clear();
	protlist[REOP]->clear();

	while(1)
	{
		memset(buf, 0, MAX_LEN);
 		a = strchr(b, '\n');
		if(a)
		{
			strncpy(buf, b, abs(b-a));
			b = a + 1;
           	//printf("parse(\"%s\");\n", buf);
			parse(buf);
		}
		else
		{
			//printf("end parse(\"%s\");\n", b);
			parse(b);
			break;
		}
	}

	delete ulbuf;
	ulbuf = NULL;

    for(n=0; n<MAX_CHANNELS; ++n)
    	if(chanlist[n].name && !chanlist[n].updated)
			removeChannel(chanlist[n].name);

	h = first->next;
	while(h)
	{
		q = h->next;
		if(!h->updated) removeHandle(h->name);
		h = q;
	}

	for(n=0; n<net.max_conns; ++n)
	{
		if(net.conn[n].fd && net.conn[n].isRegBot() && !(net.conn[n].status & STATUS_REDIR)) ++n;
	}

	net.hub.send(S_ULOK, " ", itoa(n), NULL);
    nextSave = 0;
	save(config.userlist_file);
	HOOK(userlistLoaded, userlistLoaded());
	
	//if(strlen(ME.nick) && wildFindHost(userlist.me(), mask) == -1)
	//	ME.joinAllChannels();
}

void ul::send(inetconn *c)
{
    int i;
    char buf[MAX_LEN];
    int strip = isLeaf(c->handle);

    c->send(S_UL_UPLOAD_START, NULL);

    HANDLE *h = first->next->next;

    for(i=0; i<MAX_CHANNELS; ++i)
		if(chanlist[i].name) send(c, &chanlist[i]);

	for(unsigned int j=0; j<sizeof(protlist)/sizeof(protlist[0]); j++)
		protlist[j]->sendToUserlist(c, "*");

	send(c, first, strip);
	send(c, first->next->next, strip);
	send(c, first->next, strip);

	while(h)
	{
		send(c, h, strip);
		h = h->next;
	}

	set.sendToFile(c, S_SET);

	sprintf(buf, "%llu", SN);
	c->send(S_SN, " ", buf, NULL);
	c->send(S_UL_UPLOAD_END, NULL);
}

HANDLE *ul::checkBotMD5Digest(unsigned int ip, const char *digest, const char *authstr)
{
	HANDLE *h = first;

	while(h)
	{
		if(isBot(h) && ip == h->ip &&
			MD5HexValidate(digest, authstr, strlen(authstr), h->pass, 16))
				return h;
		h = h->next;
	}
	return NULL;
}

int ul::hasPartylineAccess(const char *mask) const
{
	HANDLE *h = first;

	while(h)
	{
		if(h->flags[GLOBAL] & HAS_P)
		{
			if(wildFindHost(h, mask) != -1)
				return 1;
		}
		h = h->next;
	}
	return 0;
}

int ul::isBot(const char *name)
{
	HANDLE *h = findHandle(name);

	if(h && isBot(h)) return 1;
	else return 0;
}

int ul::isBot(const HANDLE *h)
{
	return h && h->flags[MAX_CHANNELS] & HAS_B;
}

int ul::isBot(unsigned int ip)
{
	HANDLE *h = first;

	while(h)
	{
		if(isBot(h) && h->ip == ip) return 1;
		h = h->next;
	}
	return 0;
}

int ul::getFlags(const char *mask, const chan *ch)
{
	HANDLE *p = first;
	int need = HAS_ALL, got;

	/*
	if(isIdiot(mask, ch->channum))
		return first->flags[GLOBAL] | first->flags[ch->channum];
	*/

	while(p)
	{
        if(isBot(p))
        {
			if(wildFindHost(p, mask) != -1)
				return B_FLAGS;
		}
        else
        {
			got = need & (p->flags[MAX_CHANNELS] | p->flags[ch->channum]);
			if(got)
			{
				if(wildFindHost(p, mask) != -1)
					need -= got;
      		}
		}
		p = p->next;
	}
	got = HAS_ALL - need;

	if(got & HAS_D)
	{
		if(got & (HAS_N | HAS_B)) got &= ~HAS_D;
		else got = HAS_D | (got & (HAS_V | HAS_A | HAS_C));
	}
	if(got & HAS_K)
	{
		if(got & (HAS_N | HAS_B)) got &= ~HAS_K;
		else got = HAS_K;
	}
	else if(got & HAS_O) got &= ~(HAS_V | HAS_Q);
	else if(got & HAS_Q) got &= ~HAS_V;

	return got;
}

void ul::send(inetconn *c, CHANLIST *ch)
{
	if(ch->status & PRIVATE_CHAN)
		c->send(S_ADDCHAN, " P ", (const char *) ch->name, " ", (const char *) ch->pass, NULL);
	else
		c->send(S_ADDCHAN, " * ", (const char *) ch->name, " ", (const char *) ch->pass, NULL);

	pstring<> prefix(S_CHSET);
	prefix += " ";
	prefix += ch->name;
	ch->chset->sendToFile(c, prefix);

	for(unsigned int j=0; j<sizeof(ch->protlist)/sizeof(ch->protlist[0]); j++)
		ch->protlist[j]->sendToUserlist(c, ch->name);
}

void ul::send(inetconn *c, HANDLE *h, int strip)
{
    if(h)
    {
	    int i;
    	char buf[MAX_LEN];

		if(isBot(h)) c->send(S_ADDBOT, " ", h->name, " ",  h->creation->print(), " ", strip ? "-" : inet2char(h->ip), " ", c->status & STATUS_FILE ? h->createdBy : NULL, NULL);
		else c->send(S_ADDUSER, " ", h->name, " ", h->creation->print(), " ", c->status & STATUS_FILE ? h->createdBy : NULL, NULL);

		if(h->flags[MAX_CHANNELS])
		{
            flags2str(h->flags[MAX_CHANNELS], buf);
			c->send(S_CHATTR, " ", h->name, " ", buf, NULL);
	    }
		if(!isBot(h))
		{
			for(i=0; i<MAX_HOSTS; i++)
				if(h->host[i]) c->send(S_ADDHOST, " ", h->name, " ", h->host[i], " ", c->status & STATUS_FILE ? h->hostBy[i] : NULL, NULL);

			for(i=0; i<MAX_CHANNELS; ++i)
			{
				if(chanlist[i].name && h->flags[i])
				{
					flags2str(h->flags[i], buf);
					c->send(S_CHATTR, " ", h->name, " ", buf, " ", (const char *) chanlist[i].name, NULL);
				}
			}
		}
		else
		{
			for(i=0; i<MAX_HOSTS-1; i++)
				if(h->host[i]) c->send(S_ADDHOST, " ", h->name, " ", h->host[i], " ", c->status & STATUS_FILE ? h->hostBy[i] : NULL, NULL);

			if(h->host[MAX_HOSTS-1] && c->isRegBot())
				c->send(S_PROXYHOST, " ", h->name, " ", h->host[MAX_HOSTS-1], NULL);

			if(h->channels)
			{
				char *a = NULL;
				int i, bit;

				for(i=0, bit=1; i<MAX_CHANNELS; ++i, bit*=2)
					if(h->channels & bit && chanlist[i].status & PRIVATE_CHAN)
						a = push(a, chanlist[i].name, " ", NULL);
				if(a)
				{
					c->send(S_RJOIN, " ", h->name, " ", a, NULL);
					free(a);
				}
			}
		}
		if(h->pass && strip ? !isBot(h) : 1) c->send(S_PASSWD, " ", h->name, " ", quoteHexStr(h->pass, buf), NULL);
		if(h->info && c->status & STATUS_FILE)
		{
			ptrlist<comment::entry>::iterator e = h->info->data.begin();

			while(e)
			{
				c->send(S_ADDINFO, " ", h->name, " ", e->key, " ", e->value, NULL);
				e++;
			}
		}
		if(h->history && c->status & STATUS_FILE)
		{
			ptrlist<offence::entry>::iterator e = h->history->data.begin();

			while(e)
			{
				c->send(S_ADDOFFENCE, " ", h->name, " ", e->chan, " ", itoa(e->time), " ", itoa(e->count), " ", itoa(e->fromFlags), " ", itoa(e->toFlags), " ", e->global ? "1" : "0", " ", e->mode, NULL);
			    e++;
			}
		}

	}
}

int ul::save(const char *file, const int cypher, const char *key)
{
    inetconn uf;
    char buf[MAX_LEN];
	HANDLE *h = first;
	int i;

	if(config.bottype == BOT_LEAF) return 1;
	if(!SN) return 0;

	if((uf.open(file, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR)) < 1)
	{
		net.send(HAS_N, "[-] Cannot open ", file, " for writing: ", strerror(errno), NULL);
		nextSave = NOW + SAVEDELAY;
		return 0;
	}

	if(cypher)
	{
		if(key) uf.enableCrypt((unsigned char *)key, strlen(key));
		else
		{
			uf.status |= STATUS_ULCRYPT;
			uf.enableLameCrypt();
		}
	}
    else
	{
		uf.send("# Userlist generated by ", S_BOTNAME, " version ", S_VERSION, ".", NULL);
		uf.send("# Please do not hand edit.", NULL);
		uf.send("#                       Thank you --pks", NULL);
	}
	for(i=0; i<MAX_CHANNELS; ++i)
		if(chanlist[i].name) send(&uf, &chanlist[i]);

	for(unsigned int j=0; j<sizeof(protlist)/sizeof(protlist[0]); j++)
		protlist[j]->sendToUserlist(&uf, "*");

	while(h)
	{
		send(&uf, h);
		h = h->next;
	}

	set.sendToFile(&uf, S_SET);
	if(config.bottype == BOT_MAIN)
		dset->sendToFile(&uf, S_DSET);

    sprintf(buf, "%llu", SN);
    uf.send(S_SN, " ", buf, NULL);

	nextSave = 0;
	return 1;
}

int ul::load(const char *file, const int cypher, const char *key)
{
    char buf[MAX_LEN];
	inetconn uf;
    int n;

	if(uf.open(file, O_RDONLY) < 1)	return 0;

	if(cypher)
	{
		if(key) uf.enableCrypt((unsigned char *) key, strlen(key));
		else
		{
			uf.status |= STATUS_ULCRYPT;
			uf.enableLameCrypt();
		}
	}

	while(1)
	{
		n = uf.readln(buf, MAX_LEN);
		if(n > 0)
		{
			if(*buf == '\0' || *buf == '#' || *buf == ';') continue;
			parse(buf);
		}
		else if (n == -1) break;
	}

	if(!SN) return -1;

	nextSave = 0;

	return 1;
}

void ul::flags2str(int flags, char *str)
{
	int i = 0;

    if(flags & HAS_B)
    {
		str[i++] = 'b';
    	
        if(flags & HAS_L)
    	{
			if(flags & HAS_H)
                str[i++] = 's';
			else
                str[i++] = 'l';
		}
		else if(flags & HAS_H)
            str[i++] = 'h';

		if(flags & HAS_P)
            str[i++] = 'p';
	}
    else
    {
        flagTable *ft = FT;
        while(ft->letter)
        {
            if(flags & ft->flag)
                str[i++] = ft->letter;

            ft++;
        }
    }
                
	if(i == 0)
        str[i++] = '-';
	str[i] = '\0';
}

int ul::str2userFlags(const char *str)
{
	int flags = 0;

    flagTable *ft = FT;

    while(ft->letter)
    {
        if(strchr(str, ft->letter))
            flags |= ft->flag | ft->enforced;
        ft++;
    }
    
	return flags;
}

int ul::str2botFlags(const char *str)
{
	int flags = 0;

    flagTable *ft = BFT;

    while(ft->letter)
    {
        if(strchr(str, ft->letter))
            flags |= ft->flag | ft->enforced;
        ft++;
    }
    
	return flags;
}

int ul::changeFlags(HANDLE *p, const char *flags, int channum, inetconn *c)
{
	if(c && (!userlist.hasWriteAccess(c, p) || !c->checkFlag(HAS_N)))
		return -3;

    unsigned int f = p->flags[channum];

	if(*flags != '+' && *flags != '-')
		f &= HAS_B;


    if(!mergeFlags(f, flags))
        return -5;
        
	//changing user flags
	if(!isBot(p))
    {
		/* forbiden channel flags: sxp */
		if(channum != GLOBAL && f & (HAS_S | HAS_X | HAS_P))
			return -4;

		/* forbiden global flags: z */
		if(channum == GLOBAL && f & HAS_Z)
			return -6;
		
		/* negtive flags shouldnt exists with positive */
		/* comment if dont want to use this feature */
		if(((f & HAS_D) || (f & HAS_Q) || (f & HAS_K)) && userLevel(f) > 0)
		    return -7; 

		/* oh, we got some user */
		if(c)
		{
			/* thats me ? */
			if(c->handle == p)
			{
				if(!c->checkFlag(HAS_X) && (f & (HAS_X | HAS_Z)))
		                    return -3;
				if(!c->checkFlag(HAS_S) && f & HAS_S)
            			    return -3;
				if(!c->checkFlag(HAS_N) && f & HAS_N)
                		    return -3;
			}
			else
			{
				if(!c->checkFlag(HAS_X))
				{
		 			if(f & (HAS_S | HAS_X))
						return -3;
					if(channum == GLOBAL && f & HAS_N)
						return -3;
				}
			}
		}

        //make sure that super owner does not loose his status
		if(channum == GLOBAL && p->flags[GLOBAL] & HAS_X)
    		    f |= HAS_X | X_FLAGS;

		//idiot handle
		if(p == first)
		{
			if(f & HAS_D)
			{
			    if(channum == GLOBAL)
            			p->flags[channum] |= HAS_D;
			    // should we remove channel flags?
			    //else 
			    //	p->flags[channum] = 0;
			}
		}
		else
        	    p->flags[channum] = f;
	}
	//bot flags
	else
	{
		if(channum != GLOBAL)
        	    return -4;

		p->flags[GLOBAL] = f | B_FLAGS;
	}

	nextSave = NOW + SAVEDELAY;
	return p->flags[channum];
}

int ul::changeFlags(const char *name, const char *flags, const char *channel, inetconn *c)
{
	/* find handle */
	HANDLE *h = findHandle(name);
	if(!h) return -1;

	/* find channel */
	int num;

	if(!channel || !*channel) num = GLOBAL;
	else
	{
		num = findChannel(channel);
		if(num == -1) return -2;
	}

	return changeFlags(h, flags, num, c);
}

int ul::findHost(const HANDLE *p, const char *host) const
{
	int i;

	if(!p) return -1;

	if(*host != '#')
	{
		for(i=0; i<MAX_HOSTS; i++)
			if(p->host[i])
				if(!strcasecmp(p->host[i], host)) return i;
	}
	else
	{
		int n, k;
		n = atoi(host+1) - 1;
		if(n < 0 || n >= MAX_HOSTS) return -1;

		for(k=i=0; i<MAX_HOSTS; ++i)
		{
			if(p->host[i])
			{
				if(k == n) return i;
				else ++k;
			}
		}
	}
	return -1;
}

HANDLE *ul::findHandle(const char *name) const
{
	HANDLE *p = first;

	while(p)
	{
		if(!strcmp(p->name, name)) return p;
		p = p->next;
	}
	return NULL;
}

int ul::removeHost(HANDLE *p, const char *host)
{
	int i = findHost(p, host);

	if(i != -1)
	{
		free(p->host[i]);
		p->host[i] = NULL;
		if(p->hostBy[i])
		{
			free(p->hostBy[i]);
			p->hostBy[i] = NULL;
		}
		nextSave = NOW + SAVEDELAY;
		return i;
	}
	return -1;
}

int ul::addHost(HANDLE *p, const char *host, const char *by, time_t when, int num)
{
	int i;

	if(num == MAX_HOSTS - 1)
	{
		if(p->host[num]) free(p->host[num]);
		if(strlen(host) > 5) mem_strcpy(p->host[num], host);
		else p->host[num] = NULL;

		if(p->hostBy[num]) free(p->hostBy[num]);
		if(by && *by) mem_strcpy(p->hostBy[num], by);
		else p->hostBy[num] = NULL;

		nextSave = NOW + SAVEDELAY;
		ME.nextRecheck = NOW + 5;
		return num;
	}

	if(findHost(p, host) != -1) return -1;

	for(i=0; i<MAX_HOSTS; i++)
	{
		if(p->host[i] == NULL)
		{
			mem_strcpy(p->host[i], host);
			if(by && *by)
			{
				if(when)
				{
					char buf[MAX_LEN];
					strftime(buf, MAX_LEN, "%Y/%m/%d", localtime(&when));
					p->hostBy[i] = push(NULL, by, "@", buf, NULL);
				}
				else mem_strcpy(p->hostBy[i], by);
			}
			nextSave = NOW + SAVEDELAY;
			return i;
		}
	}
	return -1;
}

int ul::removeHandle(const char *name)
{
	HANDLE *p = first;

	if(!first || !name) return 0;
    if(!strcasecmp(first->name, name) || !strcasecmp(first->next->name, name))
		return -1;

    if(!strcasecmp(last->name, name))
	{
        if(last->flags[MAX_CHANNELS] & HAS_X) return -1;
		p = last->prev;
		p->next = NULL;
		destroy(last);
		last = p;
		nextSave = NOW + SAVEDELAY;
		return 1;
	}
	else
	{
		while(p)
		{
			if(!strcasecmp(p->name, name))
			{
                if(p->flags[MAX_CHANNELS] & HAS_X) return -1;
				p->prev->next = p->next;
				if(p->next) p->next->prev = p->prev;
				destroy(p);
				nextSave = NOW + SAVEDELAY;
				return 1;
			}
			p = p->next;
		}
	}
	return 0;
}

HANDLE *ul::addHandle(const char *name, unsigned int ip, int flags, const char *sec, const char *nano, const char *by)
{
    HANDLE *p;

	if(findHandle(name)) return NULL;

    if(!first)
    {
		last = first = new(HANDLE);
		first->next = first->prev = NULL;
		memset(last, 0, sizeof(HANDLE));
	}
    else
    {
		p = last->next = new(HANDLE);
        memset(p, 0, sizeof(HANDLE));
		p->prev = last;
		p->next = NULL;
		last = p;
    }

	mem_strcpy(last->name, name);
	last->ip = ip;
    last->flags[MAX_CHANNELS] = flags;

	if(flags & HAS_B)
	{
		for(int i=0; i<MAX_CHANNELS; ++i)
			if(chanlist[i].name && !(chanlist[i].status & PRIVATE_CHAN))
				last->channels |= int(pow(2.0, double(i)));
		++bots;
	}
    else ++users;

    if(sec && nano) last->creation = new ptime(sec, nano);
    else last->creation = new ptime();

	if(by && *by) mem_strcpy(last->createdBy, by);

    nextSave = NOW + SAVEDELAY;

    return last;
}

/* Constructor */
ul::ul()
{
	first = last = NULL;
	bots = users = 0;
	SN = 0;
	nextSave = 0;
	ulbuf = NULL;

	protlist[BAN] = new protmodelist(BAN, 'b');
	protlist[INVITE] = new protmodelist(INVITE, 'I');
	protlist[EXEMPT] = new protmodelist(EXEMPT, 'e');
	protlist[REOP] = new protmodelist(REOP, 'R');

	int i;

	for(i=0; i<MAX_CHANNELS; ++i)
	{
		chanlist[i].chset = NULL;
		chanlist[i].wasop = NULL;
		chanlist[i].protlist[BAN] = NULL;
		chanlist[i].protlist[INVITE] = NULL;
		chanlist[i].protlist[EXEMPT] = NULL;
		chanlist[i].protlist[REOP] = NULL;
		chanlist[i].allowedOps = NULL;
#ifdef HAVE_MODULES
		chanlist[i].customDataDestructor = NULL;
#endif
	}
	dset = new chanset();
}

/* Destruction derby */
ul::~ul()
{
	HANDLE *hp = first;
	HANDLE *hq;
	int i;

	while(hp)
	{
		hq = hp;
		hp = hp->next;
		destroy(hq);
	}

	for(i=0; i<MAX_CHANNELS; ++i)
		chanlist[i].reset();
}

void ul::destroy(HANDLE *p)
{
	int i;

	inetconn *c = net.findConn(p);

	if(p->flags[GLOBAL] & HAS_B) --bots;
	else --users;

    while(c)
    {
		c->close("Lost handle");
		c = net.findConn(p);
		if(!c) break;
	}

	for(i=0; i<MAX_HOSTS; i++)
	{
		if(p->host[i]) free(p->host[i]);
		if(p->hostBy[i]) free(p->hostBy[i]);
	}
	free(p->name);
	if(p->creation)
		delete p->creation;
	if(p->createdBy)
		free(p->createdBy);
	if(p->info)
		delete p->info;
	delete(p);
}


void ul::reset()
{
	HANDLE *hp = first;
	HANDLE *hq;
	int i;

	while(hp)
	{
		hq = hp;
		hp = hp->next;
		destroy(hq);
	}

	first = last = NULL;
	bots = users = 0;
	SN = 0;
	nextSave = 0;
	for(i=0; i<MAX_CHANNELS; ++i)
		chanlist[i].reset();

	delete dset;
	dset = new chanset();

	addHandle("idiots", 0, 0, 0, 0, config.handle);

	protlist[BAN]->clear();
	protlist[INVITE]->clear();
	protlist[EXEMPT]->clear();
	protlist[REOP]->clear();
}


void CHANLIST::reset()
{
	if(chset)
		delete(chset);
	if(wasop)
		delete(wasop);
	if(protlist[BAN])
		delete(protlist[BAN]);
	if(protlist[INVITE])
		delete(protlist[INVITE]);
	if(protlist[EXEMPT])
		delete(protlist[EXEMPT]);
	if(protlist[REOP])
		delete(protlist[REOP]);
	if(allowedOps)
		 delete(allowedOps);

#ifdef HAVE_MODULES
	if(customDataDestructor)
		customDataDestructor(this);
#endif

	chset = NULL;
	wasop = NULL;
	protlist[BAN] = NULL;
	protlist[INVITE] = NULL;
	protlist[EXEMPT] = NULL;
	protlist[REOP] = NULL;
	allowedOps = NULL;

	pass = "";
	name = "";

	status = nextjoin = updated = 0;
}

/* `idiots' code */

int ul::levelFlags(int level) const
{
	switch(level)
	{
		case 7: return X_FLAGS;
		case 6: return S_FLAGS;
		case 5: return N_FLAGS;
		case 4: return M_FLAGS;
		case 3: return F_FLAGS;
		case 2: return HAS_O;
		//case 1: return HAS_V;
		case 1:
		case 0: return 0;
		
		//case -1: return HAS_Q;
		//case -2: return HAS_D;
		//case -3: return HAS_K;
		default: return HAS_D; // no flags
        }
}

void ul::decrementFlags(HANDLE *h, int channum, unsigned int number)
{
 	int level = userLevel(h, channum);
	int dif = level - number;

	if(dif < 0)
		h->flags[channum] = HAS_D;
	else
		h->flags[channum] = levelFlags(dif);
}

int ul::punishIdiot(HANDLE *h, int channum, unsigned int number)
{
	if(!number) return 1;
	if(!h) return 2;
	if(channum < 0 || channum >= GLOBAL) return 3;

	int i = (int) userlist.chanlist[channum].chset->IDIOTS;

//	if(!i) return 4; /* its checked in addIdiot() method. Should we check id second time? */
	if((h->flags[channum] & HAS_D) && i != 5) return 5;
	if((h->flags[GLOBAL] & HAS_D) && i == 5) return 6;

	switch(i)
	{
		case 5:
			h->flags[GLOBAL] = HAS_D;
			break;
		case 4:
		        h->flags[channum] = HAS_D;
		        break;
		case 3:
			if(number > 1 || userLevel(h, channum) <= 0)
				h->flags[channum] = HAS_D;
			else
				h->flags[channum] = 0;
			break;
		case 2:
			decrementFlags(h, channum, number);
			break;
		case 1:
			if(h->flags[channum] & HAS_A)
                        	h->flags[channum] &= ~HAS_A;
			else
				return 7;

	}
        return 0;
}

HANDLE *ul::findHandleByHost(const char *host, const int channum) const
{
	int i;
	HANDLE *ret = NULL;
	HANDLE *p = first;
	//HANDLE *p = first->next; // we should handle `idiots' handle also first;

	while(p)
	{	
		for(i=0; i<MAX_HOSTS; i++)
			if(p->host[i])
				if(match(p->host[i],host)) 
				{
				    if(ret)
				    {
					if(userLevel(ret, GLOBAL) > userLevel(p, GLOBAL))
					    continue;
					if(userLevel(ret, channum) > userLevel(p, channum))
						continue;
				    }
				    ret = p;
				}
			
		p = p->next;
	}
	return ret;
}

int ul::addIdiot(const char *mask, const char *channel, const char *reason, const char *number)
{
	if(_isnumber(number))
		return addIdiot(mask, channel, reason, (unsigned int) atoi(number));
	return -2;
}

int ul::addIdiot(const char *mask, const char *channel, const char *reason, unsigned int number)
{
	int chnum = userlist.findChannel(channel);

	if(chnum == -1)
    	    return -1;

	int value = (int) userlist.chanlist[chnum].chset->IDIOTS;
//	if(!value) /* we dont neet to check it (its allready checked before entering in this method) */
//	    return 1;

	static char buf[MAX_LEN], buf2[MAX_LEN];
	HANDLE *h = findHandleByHost(mask, chnum);
	int i;
	
	if(!h) // if handle not found we add host to global idiots handle
	{
		h = userlist.first;
		chanuser *user = new chanuser(mask, (chan*)0, 0, 0);	
		if(isPrefix(user->ident[0]))
			snprintf(buf, MAX_LEN, "*!?%s@%s", (char *) user->ident+1, user->host);
		else
                	snprintf(buf, MAX_LEN, "*!%s@%s", user->ident, user->host);

                if((i = addHost(first, buf, config.handle, NOW)) != -1)
                {		
			net.send(HAS_N, "New offence", (number == 1) ? " " : "s ", "spoted on \002", (const char*) channel, "\002 from \002", mask, "\002: ", reason, NULL);	

                     	net.send(HAS_N, "Adding host `\002", buf, "\002' to handle `\002", first->name, "\002'", NULL);
                     	net.send(HAS_B, S_ADDHOST, " ", first->name, " ", buf, NULL);
			
			if(!h->history) h->history = new offence;			
			
			snprintf(buf2, MAX_LEN, "%s (by %s)", reason, mask);
			if(h->history->add((const char *) channel, (const char *) buf2, NOW, number, 0, HAS_D))
			{
			    //net.send(HAS_S, S_ADDOFFENCE, " ", first->name, " ", channel, " ", itoa(NOW), " 0 ", atoi(HAS_D), " 1 ", buf2, NULL);	
                     	    ++userlist.SN;
                	    userlist.nextSave = NOW + SAVEDELAY;
			    //ME.nextRecheck = NOW + SAVEDELAY;
			}
                }
//		else /* it may be reason of flood on partyline when PUNISH_BOTS value is really high */
//		{
//		    net.send(HAS_N, "Host `\002", buf, "\002' exists for handle `\002", first->name, "\002'", NULL);
//		}
		delete user;
                return (i != -1) ? 0 : 2;
	}

	if(h == userlist.first) // idiots handle. mask exists so we shouldnt do enything
	    return 3;

	if((h->flags[GLOBAL] & (HAS_E | HAS_B)) || (h->flags[chnum] & HAS_E))
		return 4;
	
	if(!h->history) h->history = new offence;
	
	// check for duplicate entries.. 
	if(h->history->data.entries())
	{
	    offence::entry *e = h->history->get(channel, reason, NOW, number);
	    if(e) // found entries
		return 5;
	}
		
	
	if(value != 5)
	    i = h->flags[chnum];
	else
	    i = h->flags[GLOBAL];
	    
	net.send(HAS_N, "New offence", (number == 1) ? " " : "s ", "spoted on \002", (const char*) channel, "\002 from \002", h->name, "\002: ", reason, NULL);

	if(!punishIdiot(h, chnum, number)) // we've changded some flags
	{
		userlist.flags2str(i, buf2);
		
		if(value != 5)	
		{
		    userlist.flags2str(h->flags[chnum], buf);

		    net.send(HAS_N, "Changing \002", channel, "\002 flags for `\002", h->name, "\002' to `\002", buf, "\002'", NULL);
		
		    if(set.PRE_0211_FINAL_COMPAT)
		    {
			net.send(HAS_B, S_CHATTR, " ", h->name, " - ", channel, NULL);		    
			net.send(HAS_B, S_CHATTR, " ", h->name, " ", buf, " ", channel, NULL);		    
			++userlist.SN;
		    }
		    else
			net.send(HAS_B, S_CHATTR, " ", h->name, " -", buf2, "+", buf, " ", channel, NULL);
		}
		else
		{
		    userlist.flags2str(h->flags[GLOBAL], buf);

		    net.send(HAS_N, "Changing Global flags for `\002", h->name, "\002' to `\002", buf, "\002'", NULL);
		
		    if(set.PRE_0211_FINAL_COMPAT)
		    {
			net.send(HAS_B, S_CHATTR, " ", h->name, " -", NULL);		    
			net.send(HAS_B, S_CHATTR, " ", h->name, " ", buf,  NULL);		    
			++userlist.SN;
		    }
		    else
			net.send(HAS_B, S_CHATTR, " ", h->name, " -", buf2, "+", buf, NULL);
		
		    for(int j = 0; j < MAX_CHANNELS; j++)
		    {
			if(userlist.chanlist[j].name) // chan exists
			{
			    if(h->flags[j] && h->flags[j] != HAS_D) //user has flags and dont has +d
			    {	
				net.send(HAS_N, "Changing \002", (const char *) userlist.chanlist[j].name, "\002 flags for `\002", h->name, "\002' to `\002-\002'", NULL);
			    
				h->flags[j] = 0; // no flags
				net.send(HAS_B, S_CHATTR, " ", h->name, " - ", (const char *) userlist.chanlist[j].name, NULL);		    
				++userlist.SN;
			    }
			}
		    }
		}
				    
        	++userlist.SN;
        }

	if(value != 5)
	{
	    h->history->add((const char *) channel, (const char *) reason, NOW, number, i, h->flags[chnum]);
	    //net.send(HAS_S, S_ADDOFFENCE, " ", h->name, " ", channel, " ", itoa(NOW), " ", itoa(i), " ", itoa(h->flags[chnum]), " 0 ", reason, NULL);	
	}
	else
	{
	    h->history->add((const char *) channel, (const char *) reason, NOW, number, i, h->flags[GLOBAL], true);
	    //net.send(HAS_S, S_ADDOFFENCE, " ", h->name, " ", channel, " ", itoa(NOW), " ", itoa(i), " ", itoa(h->flags[chnum]), " 1 ", reason, NULL);	
	}
	//ME.nextRecheck = NOW + SAVEDELAY;
        userlist.nextSave = NOW + SAVEDELAY;
	
	return 0;
}

unsigned int ul::offences() const
{
    unsigned int i = 0;
    HANDLE *h = first;
    
    while(h)
    {
	if(h->history && h->history->data.entries()) i++;
	h = h->next;
    }
    return i;
}
