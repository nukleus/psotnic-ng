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

char _chmodes[MAX_LEN];

void chan::recheckShits()
{
	if(synced() < 9)
		return;

	protmodelist::entry *s;
	ptrlist<chanuser>::iterator u = users.begin();

	while(u)
	{
		if(u->flags & CHECK_SHIT && me->flags & IS_OP)
		{
			if((s = checkShit(u)))
			{
				u->setReason(s->fullReason());
				toKick.sortAdd(u);

				if(myTurn(chset->PUNISH_BOTS, u->hash32()))
				{
					modeQ[PRIO_HIGH].add(NOW, "+b", s->mask);
					modeQ[PRIO_HIGH].flush(PRIO_HIGH);
				}
				else modeQ[PRIO_LOW].add(NOW+2, "+b", s->mask);
			}
			u->flags &= ~CHECK_SHIT;
		}
		u++;
	}
}

protmodelist::entry *chan::checkShit(const chanuser *u, const char *host)
{
	static char h[MAX_LEN];
	protmodelist::entry *s;
	if(!host)
	{
		snprintf(h, MAX_LEN, "%s!%s@%s", u->nick, u->ident, u->host);
		host = h;
	}
	if(!(u->flags & HAS_F) && synced() >= 9 && !list[EXEMPT].match(host) &&
		((s = protlist[BAN]->match(u)) || (s = userlist.protlist[BAN]->match(u))))
		return s;
	else
		return NULL;
}

/** Checks if all sticky bans, invites, exempts and reops are set.
 *
 * TODO: use invex and excepts of class server
 *
 * \author patrick <patrick@psotnic.com>
 */

void chan::checkList()
{
	ptrlist<protmodelist::entry>::iterator l;
	char mode[3];
	unsigned int j;

	if(synced() < 11)
		return;

	// global

	for(j=0; j<sizeof(userlist.protlist)/sizeof(userlist.protlist[0]); j++)
	{
		mode[0] = '+';
		mode[1] = userlist.protlist[j]->mode;
		mode[2] = '\0';

		for(l=userlist.protlist[j]->data.begin(); l; l++)
		{
			if(userlist.protlist[j]->type == BAN && !l->sticky)
				continue;

			if(!list[userlist.protlist[j]->type].find(l->mask) && myTurn(chset->GUARDIAN_BOTS, hash32(l->mask)) && !modeQ[PRIO_LOW].find(mode, l->mask))
				modeQ[PRIO_LOW].add(NOW, mode, l->mask);
		}
	}

	// local

	for(j=0; j<sizeof(protlist)/sizeof(protlist[0]); j++)
	{
		mode[0] = '+';
		mode[1] = protlist[j]->mode;
		mode[2] = '\0';

		for(l=protlist[j]->data.begin(); l; l++)
		{
			if(protlist[j]->type == BAN && !l->sticky)
				continue;

			if(!list[protlist[j]->type].find(l->mask) && myTurn(chset->GUARDIAN_BOTS, hash32(l->mask)) && !modeQ[PRIO_LOW].find(mode, l->mask))
				modeQ[PRIO_LOW].add(NOW, mode, l->mask);
		}
	}
}

#ifdef HAVE_ADNS
void chan::updateDnsEntries()
{
	if(!config.resolve_threads)
		return;

	ptrlist<chanuser>::iterator u = users.begin();

	while(u)
	{
		if(!*u->ip4 && !*u->ip6)
		{
			if(u->updateDNSEntry())
				checkClone(u);
		}
		u++;
	}
}
#endif

int chan::flushKickQueue()
{
	chanuser *MultHandle[6];

	if(toKick.entries() - sentKicks)
	{
		int j = getRandomItems(MultHandle, toKick.begin(), toKick.entries() - sentKicks, 6, KICK_SENT);
		kick4(MultHandle, j);
		return 1;
	}
	return 0;
}

int chan::myPos()
{
	ptrlist<chanuser>::iterator p = opedBots.begin();
	int i;

	for(i=0; p; ++i)
	{
		if(me == p) return i;
		p++;
	}

	return -1;
}


char chan::valid(const char *str)
{
	if(strlen(str) > 50)
		return 0;

	if(*str == '#' || *str == '&' || *str == '+') return 1;
	if(*str == '!' && str[1] != '!') return 1;

	return 0;
}

void chan::names(const char *owner)
{

	int j, i, rows;
	rows = (users.entries() / 6) + 1;

	ptrlist<chanuser>::iterator p;
	char nick[6][13];

	net.sendOwner(owner, "[Users ", (const char *) name, "]", NULL);
	for(i=0; i<rows; ++i)
	{
		for(j=0; j<6; ++j)
		{
			memset(nick[j], 0, 13);
			p = users.getItem(i*6+j);
			if(p)
				snprintf(nick[j], 13, "[%c%-9s]", p->flags & IS_OP ? '@' : ' ', p->nick);
		}
		if(strlen(nick[0]))
			net.sendOwner(owner, nick[0], " ", nick[1], " ", nick[2], " ", nick[3], " ",  nick[4], " ", nick[5], NULL);
	}
	i = chops();
	char ops[16], normal[16], total[16];

	strcpy(ops, itoa(i));
	strcpy(total, itoa(users.entries()));
	strcpy(normal, itoa(users.entries() - i));
	net.sendOwner(owner, "-!- Psotnic: ", (const char *) name, "(", getModes(), "): Total of ", total, " nicks [",
			ops, " ops, ", normal, " normal]", NULL);
}

void chan::cwho(const char *owner, const char *arg)
{
	char buf[512];
	int i = 0, f = 0;
	ptrlist<chanuser>::iterator p = users.begin();
	if(arg && *arg)
	{
		if(strchr(arg, 'v')) f |= IS_VOICE;
		if(strchr(arg, 'o')) f |= IS_OP;
		if(strchr(arg, 'l')) f |= IS_LUSER;
		if(strchr(arg, 'b')) f |= HAS_B;
	}

	while(p)
	{
		if(f)
		{
			if(((f & IS_VOICE) && (p->flags & IS_VOICE)) || ((f & IS_OP) && (p->flags & IS_OP)) || ((f & HAS_B) && (p->flags & HAS_B)) || ((f & IS_LUSER) && !(p->flags & IS_VOICE || p->flags & IS_OP)))
			{
				memset(buf, 0, sizeof(buf));
				snprintf(buf, 512, "[%3d] [%c%-12s] [%12s\002@\002%-40s]", ++i, p->flags & IS_OP ? '@' : p->flags & IS_VOICE ? '+' : ' ', (const char *) p->nick, (const char *) p->ident, (const char *) p->host);
				net.sendOwner(owner, buf, NULL);
			}
		}
		else
		{
			memset(buf, 0, sizeof(buf));
			snprintf(buf, 512, "[%3d] [%c%-12s] [%12s\002@\002%-40s]", ++i, p->flags & IS_OP ? '@' : p->flags & IS_VOICE ? '+' : ' ', (const char *) p->nick, (const char *) p->ident, (const char *) p->host);
			net.sendOwner(owner, buf, NULL);
		}
		p++;
	}

	if(!i) net.sendOwner(owner, "[*] Psotnic: No matches found", NULL);
}

int chan::userLevel(int flags) const
{
	if(flags & HAS_X) return 6;
	if(flags & (HAS_S | HAS_B)) return 5;
	if(flags & HAS_N) return 4;
	if(flags & HAS_M) return 3;
	if(flags & HAS_F) return 2;
	if(flags & HAS_O) return 2;
	if(flags & HAS_V && flags & HAS_A) return 1;
	if(flags & HAS_Q) return -1;
	if(flags & HAS_D) return -2;
	if(flags & HAS_K) return -3;
	return 0;
}

int chan::userLevel(const chanuser *u) const
{
	return u ? userLevel(u->flags) : 0;
}

char *chan::getModes()
{
	/* imnpstaqr */
	memset(_chmodes, 0, MAX_LEN);

	//overflow
	if(key && strlen(key) > MAX_LEN - 50)
	{
		strcpy(_chmodes, "buffer overflow attempt");
		return _chmodes;
	}
	strcpy(_chmodes, "+");
	if(key && *key)	strcat(_chmodes, "k");
	if(limit) strcat(_chmodes, "l");
	if(flags & FLAG_I) strcat(_chmodes, "i");
	if(flags & FLAG_M) strcat(_chmodes, "m");
	if(flags & FLAG_N) strcat(_chmodes, "n");
	if(flags & FLAG_P) strcat(_chmodes, "p");
	if(flags & FLAG_S) strcat(_chmodes, "s");
	if(flags & FLAG_T) strcat(_chmodes, "t");
	if(flags & FLAG_Q) strcat(_chmodes, "q");
	if(flags & FLAG_R) strcat(_chmodes, "r");

	if(key && *key)
	{
		strcat(_chmodes, " ");
		strcat(_chmodes, key);
	}
	if(limit)
	{
		strcat(_chmodes, " ");
		strcat(_chmodes, itoa(limit));
	}
	return _chmodes;
}

int chan::myTurn(int num, int hash)
{
	chanuser **MultHandle;
	int i, j;

	num = numberOfBots(num);
	if(num < 1) return 0;

	MultHandle = (chanuser **) malloc(sizeof(chanuser *)*num);
	srand(hash, hash32(name));
	j = getRandomItems(MultHandle, opedBots.begin(), opedBots.entries(), num);
	for(i=0; i<j; ++i)
	{
		if(!strcmp(MultHandle[i]->nick, ME.nick))
		{
			free(MultHandle);
			srand();
			return 1;
		}
	}
	srand();
	free(MultHandle);
	return 0;
}

void chan::buildAllowedOpsList(const char *offender)
{
	ptrlist<chanuser>::iterator p = users.begin();
	chanuser *kicker;
	if(userlist.chanlist[channum].allowedOps)
		delete userlist.chanlist[channum].allowedOps;

	if(synced())
	{
		if(chset->TAKEOVER)
		{
			userlist.chanlist[channum].allowedOps = NULL;
			return;
		}
		else
			userlist.chanlist[channum].allowedOps = new wasoptest(60);

		kicker = getUser(offender);
		if(kicker && !(kicker->flags & HAS_F)) toKick.sortAdd(kicker);

		while(p)
		{
			if((p->flags & IS_OP) && !(p->flags & HAS_F) && !toKick.find(p))
			{
				userlist.chanlist[channum].allowedOps->add(p);
			}
			p++;
		}
	}
}

void chan::setFlags(const char *str)
{
	flags = 0;
	addFlags(str);
}

void chan::addFlags(const char *str)
{
	/* imnpstaqr */
	strchr(str, 'i') ? flags |= FLAG_I : 0;
	strchr(str, 'n') ? flags |= FLAG_N : 0;
	strchr(str, 's') ? flags |= FLAG_S : 0;
	strchr(str, 'm') ? flags |= FLAG_M : 0;
	strchr(str, 't') ? flags |= FLAG_T : 0;
	strchr(str, 'r') ? flags |= FLAG_R : 0;
	strchr(str, 'p') ? flags |= FLAG_P : 0;
	strchr(str, 'q') ? flags |= FLAG_Q : 0;

}

void chan::removeFlags(const char *str)
{
	strchr(str, 'i') ? flags &= ~FLAG_I : 0;
	strchr(str, 'n') ? flags &= ~FLAG_N : 0;
	strchr(str, 's') ? flags &= ~FLAG_S : 0;
	strchr(str, 'm') ? flags &= ~FLAG_M : 0;
	strchr(str, 't') ? flags &= ~FLAG_T : 0;
	strchr(str, 'r') ? flags &= ~FLAG_R : 0;
	strchr(str, 'p') ? flags &= ~FLAG_P : 0;
	strchr(str, 'q') ? flags &= ~FLAG_Q : 0;
}

int chan::hasFlag(char f) const
{
	if(f == 'i' && flags & FLAG_I) return 1;
	else if(f == 'n' && flags & FLAG_N) return 1;
	else if(f == 's' && flags & FLAG_S) return 1;
	else if(f == 'm' && flags & FLAG_M) return 1;
	else if(f == 't' && flags & FLAG_T) return 1;
	else if(f == 'r' && flags & FLAG_R) return 1;
	else if(f == 'p' && flags & FLAG_P) return 1;
	else if(f == 'q' && flags & FLAG_Q) return 1;
	else if(f == 'k' && key && *key) return 1;
	else if(f == 'l' && limit) return 1;

	return 0;
}

int chan::synced() const
{
	return synlevel;
}

int chan::chops() const
{
	int ops = 0;
	ptrlist<chanuser>::iterator u = users.begin();

	while(u)
	{
		if(u->flags & IS_OP) ++ops;
		u++;
	}
	return ops;
}

void chan::updateKey(const char *newkey)
{
	if(!strcmp(key, newkey)) return;

	key = newkey;

	if(set.REMEMBER_OLD_KEYS ? *newkey : 1)
	{
		userlist.chanlist[channum].pass = newkey;
		userlist.nextSave = NOW + SAVEDELAY;
	}
}

void chan::requestOp() const
{

	chanuser **multHandle = (chanuser **) malloc(sizeof(chanuser *) * chset->GETOP_BOTS);
	int i, j;
	inetconn *c;

	j = getRandomItems(multHandle, opedBots.begin(), opedBots.entries(), chset->GETOP_BOTS);

	for(i=0; i<j; ++i)
	{
		c = net.findConn(multHandle[i]->nick);
		if(c && c->isRegBot())
		{
			c->send(S_BOP, " ", (const char *) name, NULL);
		}
	}
	free(multHandle);
}

void chan::recheckFlags()
{
	ptrlist<chanuser>::iterator p = users.begin();

	botsToOp.clear();
	toOp.clear();
	opedBots.clear();

	while(p)
	{
		p->getFlags(this);
		if(p->flags & HAS_B)
		{
			if(p->flags & IS_OP) opedBots.sortAdd(p);
			else botsToOp.sortAdd(p);
		}
		p++;
	}
}

int chan::gotBan(const char *ban, chanuser *caster)
{
	if(!ban)
		return 0;

	if(config.bottype == BOT_MAIN)
		protmodelist::updateLastUsedTime(name, ban, BAN);
	else if(caster == me)
	{
		inetconn *c = net.findMainBot();
		if(c)
			c->send(S_SHITOBSERVED, " ", (const char *) name, " ", ban, " ", caster->nick, "!", caster->ident, "@", caster->host, NULL);
	}

	HANDLE *h = userlist.first;
	ptrlist<chanuser>::iterator u;

	if(!(caster->flags & HAS_B))
	{
		//check if banned mask matches smb with higer level
		u = users.begin();
		while(u)
		{
			if(userLevel(&u) > userLevel(caster))
			{
				if(u->matchesBan(ban))
				{
					if(caster->nick && *caster->nick) toKick.sortAdd(caster);
					return 1;
				}
			}
			u++;
		}

		if(chset->STRICT_BANS)
		{
			while(h)
			{
				if(userLevel(h->flags[GLOBAL] | h->flags[channum]) > userLevel(caster) &&
					userlist.wildFindHostExtBan(h, ban) != -1)
				{
					if(caster->nick && *caster->nick) toKick.sortAdd(caster);
					return 1;
				}
				h = h->next;
			}
		}
	}

	//enforce this ban
	if(chset->ENFORCE_BANS)
	{
		static char reason[MAX_LEN], buf[MAX_LEN];
		protmodelist::entry *s = NULL;

		if(caster->flags & HAS_B)
		{
			if((s = protlist[BAN]->find(ban)) || (s = userlist.protlist[BAN]->find(ban)))
			{
				if(synced() >= 9)
					snprintf(reason, MAX_LEN, "%s", s->fullReason());
				else
					//channel exempt list is not sycned yet, we have to w8 a while
					return 0;
			}
			else
				snprintf(reason, MAX_LEN, "banned");
		}
		else
			snprintf(reason, MAX_LEN, "banned by %s: \002%s\002", caster->nick, ban);

		int ulevel = userLevel(caster);

		if(chset->ENFORCE_BANS == 2 && ulevel <= userLevel(HAS_O))
			ulevel = userLevel(HAS_O);

		for(u = users.begin(); u; u++)
		{
			if(!(u->flags & HAS_F) && ulevel > userLevel(&u))
			{
				if(chset->CLONECHECK == 1 && caster->flags & HAS_B && (u->flags & HAS_C) && !s)
					continue;

				snprintf(buf, MAX_LEN, "%s!%s@%s", u->nick, u->ident, u->host);
				if(s ? (bool) checkShit(u, buf) : ((bool) u->matchesBan(ban) && !list[EXEMPT].match(buf)))
				{
					if(!u->reason)
						u->setReason(reason);
					toKick.sortAdd(u);
				}
			}
		}
	}
	return 0;
}

int chan::numberOfBots(int num) const
{
	if(num >= 0) return num;
	if(num == -100) return opedBots.entries();
	return ((num * opedBots.entries()) / (-100)) + 1;
}

void chan::reOp()
{
	if(users.entries() == 1)
	{
		net.irc.send("PART ", (const char *) name, NULL);
		net.irc.send("JOIN ", (const char *) name, NULL);
	}
	else if(users.entries() > 1 && config.listenport)
	{
		char *buf = push(NULL, S_CYCLE, (const char *) name, NULL);
		quoteBots(buf);
		net.send(HAS_N, "[*] Reoping ", (const char *) name, NULL);
		free(buf);
	}
}

void chan::quoteBots(const char *str)
{
	ptrlist<chanuser>::iterator p = users.begin();
	inetconn *c;

	while(p)
	{
		if(p->flags & HAS_B)
		{
			c = net.findConn(p->nick);
			if(c && c->isRegBot()) c->send(str, NULL);
			p++;
		}
	}
}

void chan::gotNickChange(const char *from, const char *to)
{
	chanuser *p = getUser(from);
	int status = 0;

	if(!p) return;

	if(toOp.remove(p)) status += 1;
	if(botsToOp.remove(p)) status += 2;
	if(opedBots.remove(p)) status += 4;
	if(toKick.remove(p)) status += 8;

	users.remove(p, 0);

	free(p->nick);
	mem_strcpy(p->nick, to);
	p->hash = hash32(p->nick);

	users.add(p);

	if(status & 1) toOp.sortAdd(p);
	if(status & 2) botsToOp.sortAdd(p);
	if(status & 4) opedBots.sortAdd(p);
	if(status & 8) toKick.sortAdd(p);

	if(p->flags & KICK_SENT)
	{
		p->flags &= ~KICK_SENT;
		--sentKicks;
	}

	if((chset->CHECK_SHIT_ON_NICK_CHANGE || config.check_shit_on_nick_change))
		p->flags |= CHECK_SHIT;
}

chanuser *chan::getUser(const char *nick)
{
	if(!nick) return NULL;

	chanuser x = chanuser(nick);
	ptrlist<chanuser>::iterator ret = users.find(x);

	return (bool) ret ? &ret : NULL;
}

void chan::gotKick(const char *victim, const char *offender, const char *reason)
{
	chanuser *kicked, *kicker;
	int inv = 0;

	kicked = getUser(victim);
	kicker = getUser(offender);

        HOOK(kick, kick(this, kicked, kicker, reason));
        if(stopParsing)
        {
            stopParsing=false;
            return;
        }

	if(kicker)
	{
		if(userLevel(kicked) > userLevel(kicker))
		{
			toKick.sortAdd(kicker);
			if(myTurn(chset->PUNISH_BOTS, kicker->hash32()))
			{
				kick(kicker, config.kickreason);

				/* idiots code */
				if((int) chset->IDIOTS)
				{
					char *b = push(NULL, (const char *) "kick ", victim, NULL);
					
					/* should we remove spaces from end of reason? */
					//b = rtrim(b);

					if(userlist.isMain(userlist.me()))
						userlist.addIdiot(offender, (const char *) name, b, 1);
					else
					{
						if(net.hub.fd && net.hub.isMain())
							net.hub.send(S_ADDIDIOT, " ", offender, " ", (const char *) name, " 1 ", b, NULL);
						else
						{
							for(int i=0; i<net.max_conns; ++i)
								if(net.conn[i].isMain() && net.conn[i].fd)
								{
									net.conn[i].send(S_ADDIDIOT, " ", offender, " ", (const char *) name, " 1 ", b, NULL);
									break;
								}
						}
					}
					free(b);
				}
			}
		}
	}

	if((limit <= users.entries() - 1 || (flags & FLAG_I)) && (kicked->flags & HAS_I) &&
		myTurn(chset->INVITE_BOTS, kicker->hash32()))
		inv = 1;

	gotPart(victim);

	if(inv)
	{
		invite(victim);
		if(toKick.entries() < 2 && penalty < 9) ::invite.flush(&net.irc);
	}
}

void chan::gotPart(const char *nick, int netsplit)
{
	chanuser *p = getUser(nick);

	if(p)
	{
		if(netsplit && (p->flags & (IS_OP | HAS_F)) == IS_OP && !toKick.find(p))
			wasop->add(p);

		if(p->flags & KICK_SENT) --sentKicks;

		toKick.remove(p);
		opedBots.remove(p);
		botsToOp.remove(p);
		toOp.remove(p);

		clone_host x1 = clone_host(p, HOST_DOMAIN);
		clone_host x2 = clone_host(p, HOST_IPV4);
		clone_host x3 = clone_host(p, HOST_IPV6);
		clone_ident y = clone_ident(p);
		clone_proxy z = clone_proxy(p);
		hostClones.remove(x1);
		hostClones.remove(x2);
		hostClones.remove(x3);
		identClones.remove(y);
		proxyClones.remove(z);

		users.remove(p);

		if(chset->CYCLE && synced() && users.entries() == 1 && !(me->flags & IS_OP))
		{
			net.irc.send("PART ", (const char *) name, " :regaining op... duuuh", NULL);
			penalty += 2;
			ME.rejoin(name, 0);
		}
	}

	if(!set.PRE_0211_FINAL_COMPAT && synced() && nextlimit > NOW + chset->LIMIT_TIME_DOWN)
	{
		int tolerance;

		if(chset->LIMIT_TOLERANCE > 0)
			tolerance = chset->LIMIT_TOLERANCE;
		else
			tolerance = (chset->LIMIT_TOLERANCE * chset->LIMIT_OFFSET)/(-100);

		if(limit > users.entries() + chset->LIMIT_OFFSET + tolerance)
			nextlimit = NOW + chset->LIMIT_TIME_DOWN;
	}
}

#ifdef HAVE_DEBUG
void chan::display()
{
	printf("users (%d):\n", users.entries());
	users.display();
	printf("toOp (%d):\n", toOp.entries());
	toOp.display();
	printf("botsToOp (%d):\n", botsToOp.entries());
	botsToOp.display();
	printf("toKick (%d):\n", toKick.entries());
	toKick.display();
	printf("opedBots (%d):\n", opedBots.entries());
	opedBots.display();
	printf("modes: %s\n", getModes());
}
#endif




bool chan::checkClone(chanuser *p)
{
	char buf[MAX_LEN];
	bool badBoy = false;

	/* clone check */
	if(!(p->flags & HAS_C) && chset->CLONECHECK)
	{
		buf[0] = '\0';

		/* ident clones */
		if((p->clones_to_check & CLONE_IDENT) && set.IDENT_CLONES && identClones.addLast(new clone_ident(p)) > set.IDENT_CLONES && synced())
		{
			if(isPrefix(*p->ident))
				snprintf(buf, MAX_LEN, "*!?%s@*", p->ident+1);
			else
				snprintf(buf, MAX_LEN, "*!%s@*", p->ident);

			punishClones(buf, myTurn(chset->GUARDIAN_BOTS, hash32(p->ident)));
			badBoy = true;
		}

		p->clones_to_check &= ~CLONE_IDENT;

		/* proxy clones */
		if((p->clones_to_check & CLONE_PROXY) && set.PROXY_CLONES && isPrefix(*p->ident) && getPartOfDomain(p->host, 3) && synced() &&
			proxyClones.addLast(new clone_proxy(p)) > set.PROXY_CLONES)
		{
			//FIXME: what about ipv6 proxy clones? :P
			snprintf(buf, MAX_LEN, "*!*@*.%s", getPartOfDomain(p->host, 2));

			punishClones(buf, myTurn(chset->GUARDIAN_BOTS));
			badBoy = true;
		}

		p->clones_to_check &= ~CLONE_PROXY;

		/* host clones */
		if(set.HOST_CLONES && synced())
		{
			if(p->dnsinfo & HOST_DOMAIN)
			{
				DEBUG((p->clones_to_check & CLONE_HOST) && printf("[D] CLONE_HOST check for: %s!%s@%s\n", p->nick, p->ident, p->host));

				if((p->clones_to_check & CLONE_HOST) && hostClones.addLast(new clone_host(p, HOST_DOMAIN)) > set.HOST_CLONES)
				{
					snprintf(buf, MAX_LEN, "*!*@%s", p->host);
				}
				p->clones_to_check &= ~CLONE_HOST;
			}

			if(p->dnsinfo & HOST_IPV4)
			{
				DEBUG((p->clones_to_check & CLONE_IPV4) && printf("[D] CLONE_IPV4 check for: %s!%s@%s\n", p->nick, p->ident, p->ip4));

				if((p->clones_to_check & CLONE_IPV4) && hostClones.addLast(new clone_host(p, HOST_IPV4)) > set.HOST_CLONES)
				{
					char *n = nindex(p->ip4, 3, '.');
					if(n)
					{
						//the trick
						*n = '\0';
						snprintf(buf, MAX_LEN, "*!*@%s.*", p->ip4);
						*n = '.';
					}
				}
				p->clones_to_check &= ~CLONE_IPV4;
			}

			if(p->dnsinfo & HOST_IPV6)
			{
				DEBUG((p->clones_to_check & CLONE_IPV6) && printf("[D] CLONE_IPV6 check for: %s!%s@%s\n", p->nick, p->ident, p->ip6));

				if((p->clones_to_check & CLONE_IPV6) && hostClones.addLast(new clone_host(p, HOST_IPV6)) > set.HOST_CLONES)
				{
					char *n = nindex(p->ip6, 4, ':');
					if(n)
					{
						//the trick
						*n = '\0';
						snprintf(buf, MAX_LEN, "*!*@%s:*", p->ip6);
						*n = ':';
					}
				}
				p->clones_to_check &= ~CLONE_IPV6;
			}

			if(*buf)
			{
				punishClones(buf, myTurn(chset->GUARDIAN_BOTS, hash32(p->host)));
				badBoy = true;
			}
		}
	}
	return badBoy;
}

chanuser *chan::gotJoin(const char *mask, int def_flags)
{
	static char buf[MAX_LEN];
	chanuser *p  = new chanuser(mask, this, def_flags, 1);
	protmodelist::entry *s;
	bool badBoy;

#ifdef HAVE_ADNS
	 badBoy = p->updateDNSEntry();
#endif

	users.add(p);

	if(ME.overrider)
	{
		if(synced() && !(p->flags & HAS_F))
		{
			chanuser *over = getUser(ME.overrider);

			if(!over)
			{
				p->setReason(config.limitreason);
				toKick.sortAdd(p);

				chanuser tmp(ME.overrider, this);
				if(tmp.ok())
				{
					badBoy = true;
					snprintf(buf, MAX_LEN, "*!%s@%s", tmp.ident, tmp.host);
					enforceBan(buf, me, config.limitreason);
				}
			}
			else if(!(over->flags & HAS_N))
			{
				p->setReason(config.limitreason);
				toKick.sortAdd(p);
				badBoy = true;

				snprintf(buf, MAX_LEN, "*!%s@%s", over->ident, over->host);
				enforceBan(buf, me, config.limitreason);
			}

			if(badBoy == true && (int) chset->IDIOTS)
			{
				char *a = push(NULL, (const char*) "invite ", mask, NULL);

				if(myTurn(chset->PUNISH_BOTS, p->hash32()))
				{
					if(userlist.isMain(userlist.me()))
						userlist.addIdiot((const char *) ME.overrider, (const char *) name, a, 1);
					else
					{
						if(net.hub.fd && net.hub.isMain())
							net.hub.send(S_ADDIDIOT, " ", (const char *) ME.overrider, " ", (const char *) name, " 1 ", a, NULL);
						else
							for(int i=0; i<net.max_conns; ++i)
							{
								if(net.conn[i].isMain() && net.conn[i].fd)
								{
									net.conn[i].send(S_ADDIDIOT, " ", (const char *) ME.overrider, " ", (const char *) name, " 1 ", a, NULL);
									break;
								}
							}
					}
				}
				free(a);
			}
		}
		ME.overrider = "";
	}

	if(p->flags & HAS_K)
	{
		p->setReason(config.kickreason);
		toKick.sortAdd(p);
		badBoy = true;

		if(synced() && myTurn(chset->PUNISH_BOTS, p->hash32()))
			kick(p, config.kickreason);
	}
	else if(!(p->flags & (HAS_V | HAS_O)) && chset->KEEPOUT)
	{
		if(synced())
		{
			if(!hasFlag('i') && myTurn(chset->GUARDIAN_BOTS, p->hash32()))
			{
				modeQ[PRIO_HIGH].add(NOW, "+i");
				modeQ[PRIO_HIGH].flush(PRIO_HIGH);
			}

			if(myTurn(chset->PUNISH_BOTS, p->hash32()))
				kick(p, config.keepoutreason);

			badBoy = true;
		}
	}
	else if(((chset->TAKEOVER ? 1 : !(def_flags & NET_JOINED)) && chset->ENFORCE_LIMITS
		&& limit && users.entries() > limit && !(p->flags & HAS_F)))
	{
		if(synced())
		{
			p->setReason(config.limitreason);
			toKick.sortAdd(p);
			if(myTurn(chset->PUNISH_BOTS, p->hash32()))
				kick(p, config.limitreason);

			badBoy = true;
		}
	}
	else if((s = checkShit(p, mask)))
	{
		p->setReason(s->fullReason());
		//toKick.sortAdd(p);
		badBoy = true;

		if(myTurn(chset->PUNISH_BOTS, p->hash32()))
		{
			modeQ[PRIO_HIGH].add(NOW, "+b", s->mask);
			modeQ[PRIO_HIGH].flush(PRIO_HIGH);

			//kick(p, p->reason);
		}
		else modeQ[PRIO_LOW].add(NOW+2, "+b", s->mask);
	}
	else if((p->flags & (HAS_O | HAS_A | IS_OP)) == (HAS_O | HAS_A))
	{
		if(p->flags & HAS_B) botsToOp.sortAdd(p);
		else toOp.sortAdd(p);

		if(synced() && opedBots.entries() && me->flags & IS_OP)
		{
			//if it is a bot
			if(p->flags & HAS_B)
			{
				if(chset->BOT_AOP_MODE == 2 || (chset->BOT_AOP_MODE == 1 && toKick.entries() > 4) || chset->TAKEOVER)
				{
					if(net.findConn(p->nick) && myTurn(chset->BOT_AOP_BOTS, p->hash32()))
					{
						if(chset->TAKEOVER ? 1 : !(def_flags & NET_JOINED))
							op(p);

						modeQ[PRIO_HIGH].add(NOW+2, "+o", p->nick);
					}
				}
			}
			else if(myTurn(chset->AOP_BOTS, p->hash32()))
			{
				if(chset->TAKEOVER ? 1 : !(def_flags & NET_JOINED))
					op(p);

				modeQ[PRIO_LOW].add(NOW+2, "+o", p->nick);
			}
		}
	}
	else if((p->flags & (HAS_V | HAS_A | IS_VOICE)) == (HAS_V | HAS_A))
	{
		if(synced() && opedBots.entries())
		{
			if(myTurn(chset->AOP_BOTS, p->hash32()))
			{
				modeQ[PRIO_LOW].add(NOW+2, "+v", p->nick);
			}
		}
	}

	if(p->flags & IS_OP)
	{
		if(!(p->flags & HAS_O))
		{
			if(chset->TAKEOVER || (userlist.chanlist[channum].allowedOps &&
				!userlist.chanlist[channum].allowedOps->remove(p)))
					toKick.sortAdd(p);
		}

		if(p->flags & HAS_B)
		{
			opedBots.sortAdd(p);
			if(!initialOp) initialOp = NOW;
		}
	}
	if(chset->CYCLE && botsToOp.entries() == users.entries() && config.listenport && synced())
		reOp();

	//if(!(def_flags & NET_JOINED))
	//	wasop->remove(p);

	badBoy |= checkClone(p);

#ifdef HAVE_ADNS
	if(!badBoy)
		resolver->resolv(p->host);
#endif

	if(!(p->flags & HAS_F) || synced() < 9)
		p->flags |= CHECK_SHIT;

	if(penalty < 10)
		modeQ[PRIO_HIGH].flush(PRIO_HIGH);

	if(synced())
	{
		HOOK(join, join(p, this, mask, def_flags & NET_JOINED));
		stopParsing=false;
	}

	return p;
}

/* Constructor */
chan::chan()
#ifdef HAVE_MODULES
	: CustomDataStorage()
#endif
{
	sentKicks = flags = limit = status = synlevel = 0;
	me = NULL;
	initialOp = 0;
	since = NOW;
	nextlimit = -1;
	users.removePtrs();
	hostClones.removePtrs();
	identClones.removePtrs();
	proxyClones.removePtrs();
	modeQ[0].setChannel(this);
	modeQ[1].setChannel(this);

	HOOK( new_chan, new_chan( this ) );
}

/* Destruction derby */
chan::~chan()
{
}

/* class chanuser */

chanuser::chanuser(const char *str)
#ifdef HAVE_MODULES
	: CustomDataStorage()
#endif
{
	char *a = strchr(str, '!');
	if(a) mem_strncpy(nick, str, (int) abs(str - a) + 1);
	else mem_strcpy(nick, str);

	flags = 0;
	ident = NULL;
	host = NULL;
	hash = ::hash32(nick);
	handle = NULL;
	reason = NULL;
	ip4 = NULL;
	ip6 = NULL;

	HOOK( new_chanuser, new_chanuser( this ) );
}

chanuser::chanuser(const char *m, const chan *ch, const int f, const bool scan)
#ifdef HAVE_MODULES
	: CustomDataStorage()
#endif
{
	char *a = strchr(m, '!');
	char *b = strchr(m, '@');

	reason = NULL;

	if(!a || !b)
	{
		memset(this, 0, sizeof(chanuser));
		return;
	}

	mem_strncpy(nick, m, (int) abs(m - a) +1);
	mem_strncpy(ident, a+1, (int) abs(a - b));
	mem_strcpy(host, b+1);
	flags = f;
	if(scan)
		flags |= userlist.getFlags(m, ch);

	hash = ::hash32(nick);
	handle = NULL;

	switch(isValidIp(host))
	{
		case 4:
			dnsinfo = HOST_IPV4;
			mem_strcpy(ip4, host);
			mem_strcpy(ip6, "");
			break;

		case 6:
			dnsinfo = HOST_IPV6;
			mem_strcpy(ip4, "");
			mem_strcpy(ip6, host);
			break;

		default:
			dnsinfo = HOST_DOMAIN;
			mem_strcpy(ip4, "");
			mem_strcpy(ip6, "");
			break;
	}

	clones_to_check = CLONE_HOST | CLONE_IPV6 | CLONE_IPV4 | CLONE_IDENT | CLONE_PROXY;

	if(ch)
	{
		HOOK(chanuserConstructor, chanuserConstructor(ch, this));
		stopParsing=false;
	}

	HOOK( new_chanuser, new_chanuser( this ) );
}

chanuser::~chanuser()
{
#ifdef HAVE_MODULES
/*	if(host && customDataDestructor)
		customDataDestructor(this);*/
#endif
	if(nick) free(nick);
	if(ident) free(ident);
	if(ip4) free(ip4);
	if(ip6) free(ip6);
	if(host)
	{
		free(host);
	}

	if(reason) free(reason);
}

int chanuser::operator==(const chanuser &c) const
{
	return (hash == c.hash) && !strcmp(nick, c.nick);
}

int chanuser::operator<(const chanuser &c) const
{
	return strcmp(nick, c.nick) < 0 ? 1 : 0;
}

void chanuser::getFlags(const chan *ch)
{
	char *m = push(NULL, nick, "!", ident, "@", host, NULL);
	flags &= ~(HAS_ALL);
	flags |= userlist.getFlags(m, ch);
	free(m);
}

unsigned int chanuser::hash32() const
{
	return hash;
}

#ifdef HAVE_DEBUG
void chanuser::display()
{
	char *tmp, buf[MAX_LEN];
	if(flags & IS_OP)
	{
		strcpy(buf, "@");
		tmp = buf + 1;
	}
	else tmp = buf;
	userlist.flags2str(flags, tmp);

	printf("%s!%s@%s (%d) [%s] [%s] [%s]\n", nick, ident, host, hash, buf, ip4, ip6);
}
#endif

void chanuser::setReason(const char *r)
{
	if(reason)
		free(reason);

	mem_strcpy(reason, r);
}

int chanuser::matches(const char *mask) const
{
	static char buf[MAX_LEN];

	snprintf(buf, MAX_LEN, "%s!%s@%s", nick, ident, host);
	return match(mask, buf);
}

int chanuser::matchesBan(const char *mask) const
{
	return matchBan(mask, this);
}

bool chanuser::ok() const
{
	return nick && host && ident;
}

#ifdef HAVE_ADNS
int chanuser::updateDNSEntry()
{
	adns::host2ip *info = resolver->getIp(host);

	if(info)
	{
		if(ip4)
			free(ip4);
		if(ip6)
			free(ip6);

		mem_strcpy(ip4, info->ip4);
		mem_strcpy(ip6, info->ip6);

		if(*ip4)
			dnsinfo |= HOST_IPV4;
		if(*ip6)
			dnsinfo |= HOST_IPV6;

		DEBUG(printf(">>> Updating: %s (%s, %s)\n", nick, ip4, ip6));
		return 1;
	}

	return 0;
}
#endif

/** Searchs for people that are not added with +v or +o and kicks them out.
 * It makes also sure that the channel is either +k or +i.
 * If the channel was +k, the bot will set +i so that the guy cannot rejoin.
 *
 * FIXME: Isn't it enough to check this when someone is joining?
 *        This function is executed too often. It can slow down the bot.
 *        But then we need another way to kick users that lost their flags
 *        and to kick users when keepout has just been enabled.
 *
 * \author patrick <patrick@psotnic.com>
 */

void chan::checkKeepout()
{
	if(synced() < 9)
		return;

	if(!hasFlag('i') && !(*key))
	{
		modeQ[PRIO_HIGH].add(NOW, "+i");
		modeQ[PRIO_HIGH].flush(PRIO_HIGH);
	}

	ptrlist<chanuser>::iterator u = users.begin();

	while(u)
	{
		if(!(u->flags & (HAS_V | HAS_O)))
		{
			if(*key && !hasFlag('i'))
			{
				modeQ[PRIO_HIGH].add(NOW, "+i");
				modeQ[PRIO_HIGH].flush(PRIO_HIGH);
			}

			u->setReason(config.keepoutreason);
			toKick.sortAdd(u);
		}

		u++;
	}
}

void chan::checkProtectedChmodes()
{
	unsigned int i;
	bool pos = true;
	const char *modes=chset->MODE_LOCK.getValue();
	char mode[3];
	time_t flush_time = NOW;
	int PRIO = PRIO_LOW;

	if(synced() < 9 || !(me->flags & IS_OP))
		return;

	if(myTurn(chset->GUARDIAN_BOTS))
	{
	 	PRIO = PRIO_HIGH;
	}
	else
	{
	 	flush_time += (rand() % 16);
	}

	for(i=0; modes[i] && modes[i] != ' ' ; i++)
	{
		switch(modes[i])
		{
			case '+' : pos=true; break;
			case '-' : pos=false; break;
			default  :
				if((pos && !hasFlag(modes[i])) || (!pos && hasFlag(modes[i])) ||
						(pos && (modes[i] == 'k' && strcmp((const char *) key, chset->MODE_LOCK.getKey()) && hasFlag(modes[i])
							 || (modes[i] == 'l' && limit != chset->MODE_LOCK.getLimit()) && hasFlag(modes[i]))))
				{
					mode[0]=pos?'+':'-';
					mode[1]=modes[i];
					mode[2]='\0';

					if(modes[i] == 'k')
					{
						if(pos == false)
							modeQ[PRIO].add(flush_time, mode, key);
						else
							modeQ[PRIO].add(flush_time, mode, chset->MODE_LOCK.getKey());
					}
					else if(pos == true && modes[i] == 'l')
					{
						modeQ[PRIO].add(flush_time, mode, itoa(chset->MODE_LOCK.getLimit()));
					}
					else
					{
						modeQ[PRIO].add(flush_time, mode);
					}
				}
		}
	}
}

/** Tells the type of a channel mode.
 *  Available types are: A, B, C, D which are described in chanModeRequiresArgument()
 *
 *  \author patrick <patrick@psotnic.com>
 *  \param mode Can be any channel mode
 *  \return Type of the channel mode as char or '-' if an error occurred
 *  \sa getTypeOfChanMode()
 */

char chan::getTypeOfChanMode(char mode)
{
    int i, len, type=0;

    if(!mode || !ME.server.isupport.chanmodes)
        return '-';

    len=strlen(ME.server.isupport.chanmodes);

    for(i=0; i < len; i++)
    {
        if(ME.server.isupport.chanmodes[i] == mode)
        {
            switch(type)
            {
                case 0 : return 'A';
                case 1 : return 'B';
                case 2 : return 'C';
                case 3 : return 'D';
                default : return '-';
            }
        }

        if(ME.server.isupport.chanmodes[i] == ',')
            type++;
    }

    return '-';
}

/** Checks if a channel mode requires an argument.
 *
 *  \author patrick <patrick@psotnic.com>
 *  \param sign Can be '+' or '-'
 *  \param mode Can be any channel mode
 *  \return true if the channel mode requires an argument, otherwise false
 */

bool chan::chanModeRequiresArgument(char sign, char mode)
{
    char type;

    if(!mode)
        return false;

    // 'o', 'v'
    if(strchr(ME.server.isupport.chan_status_flags, mode))
        return true;

    type=getTypeOfChanMode(mode);

    switch(type)
    {
        case '-' : // type not found
                   return false;
        case 'A' :
                   /* "Type A: Modes that add or remove an address to or from a list.
                               These modes MUST always have a parameter when sent from the server
                               to a client."
                   */

                   return true;
        case 'B' : 
                   /* "Type B: Modes that change a setting on a channel.  These modes
                               MUST always have a parameter."
                   */

                   return true;
        case 'C' :
                   /* "Type C: Modes that change a setting on a channel.  These modes
                               MUST have a parameter when being set, and MUST NOT have a
                               parameter when being unset."
                   */

                   if(sign == '+')
                       return true;
                   else
                       return false;
        case 'D' :
                   /* "Type D: Modes that change a setting on a channel.  These modes
                               MUST NOT have a parameter."
                   */

                   return false;
    }

    return false;
}

/** Checks if a given string is a channel.
 *
 *  \author patrick <patrick@psotnic.com>
 *  \param _name Can be any string
 *  \return true if it is a channel, otherwise false
 */

bool chan::isChannel(const char *_name)
{
    int i, len;
    const char *chantypes=ME.server.isupport.find("CHANTYPES");

    if(!_name || !*_name)
        return false;

    if(!chantypes)
        return false;

    len=strlen(chantypes);

    for(i=0; i < len; i++)
    {
        if(chantypes[i] == _name[0])
            return true;
    }

    return false;
}
