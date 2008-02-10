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
#include "topics.h"

void client::checkMyHost(const char *to,  bool justConnected)
{
	static char buf[MAX_LEN];

	if(!net.irc.isReg())
		return;

	if(ident[0] == '+' || ident[0] == '-' || ident[0] == '=')
	{
		int2units(buf, MAX_LEN, ircConnFailDelay, ut_time);
		net.sendOwner(to, "[!] My connection is \002restricted\002, reconnecting in \002", buf, "\002", NULL);

		if(justConnected)
		{
			if(nextReconnect <= NOW)
				nextReconnect = NOW + ircConnFailDelay;

			ircConnFailDelay *= 3;
			if(ircConnFailDelay > 2*3600) ircConnFailDelay = 2*3600;
		}
		joinDelay = -1;
	}
	else
	{
		if(userlist.wildFindHost(userlist.me(), mask) == -1)
		{
			joinDelay = -1;

			if(ident[0] == '^' || ident[0] == '~')
			{
				snprintf(buf, MAX_LEN, "%s!%s@%s", (const char *) nick, (const char *) ident+1, (const char *) host);
				if(userlist.wildFindHost(userlist.me(), buf) != -1)
				{
					int2units(buf, MAX_LEN, ME.ircConnFailDelay, ut_time);
					net.sendOwner(to, "[!] I lost my \002ident\002, reconnecting in \002", buf, "\002", NULL);

					if(justConnected)
					{
						if(nextReconnect <= NOW)
							nextReconnect = NOW + ircConnFailDelay;

						ircConnFailDelay *= 3;
						if(ircConnFailDelay > 2*3600) ircConnFailDelay = 2*3600;
					}
					return;
				}
			}
			net.sendOwner(to, "[!] My host `\002", (const char *) mask, "\002' is not added, not joining channels", NULL);
		}
		else
		{
			ircConnFailDelay = 15;
			joinDelay = NOW + penalty;
		}
	}
}

void client::sendStatus(const char *name)
{
	net.sendOwner(name, "\002-\002 about me\002:", NULL);
	net.sendOwner(name, "Hi. I'm ", (const char *) config.handle, " and I'm running psotnic ", S_VERSION, NULL);

	char buf[MAX_LEN];
	int min, max, sum;
	double avg;

	int2units(buf, MAX_LEN, int(NOW - ME.startedAt), ut_time);
	net.sendOwner(name, "Up for\002:\002 ", buf, NULL);
	if(net.irc.isReg())
		net.sendOwner(name, "Connected to `\002", net.irc.origin, "\002' as \002`", (const char *) ME.mask, "\002'", NULL);

	net.sendOwner(name, "I have \002", itoa(net.bots()), "\002 bots and \002",
			itoa(net.owners()), "\002 owners on-line", NULL);

	if(ME.channels)
	{
		net.sendOwner(name, "\002-\002 my channels\002:\002 ", NULL);
		chan *ch = ME.first;
		while(ch)
		{
			ch->users.stats(min, avg, max, sum);
			snprintf(buf, MAX_LEN, "[hash: %d/%g/%d/%d]", min, avg, max, sum);
			net.sendOwner(name, ch->name, " (", ch->getModes(), ", ", itoa(ch->chops()),
					" ops, ", itoa(ch->users.entries()), " total) ", buf, NULL);
			ch = ch->next;
		}
	}
#ifdef HAVE_ADNS_FIXME
	//TODO: fix statistics
	net.sendOwner(name, "Resolving threads: \002", itoa(resolver.n), "\002", NULL);

	if(resolver.n)
	{
		resolver.lock_data();
		resolver.todo->stats(min, avg, max, sum);
		snprintf(buf, MAX_LEN, "\002-\002 resolver::todo::stats (min/avg/max/total): %d/%g/%d/%d", min, avg, max, sum);
		net.sendOwner(name, buf, NULL);

		resolver.resolving->stats(min, avg, max, sum);
		snprintf(buf, MAX_LEN, "\002-\002 resolver::resolving::stats (min/avg/max/total): %d/%g/%d/%d", min, avg, max, sum);
		net.sendOwner(name, buf, NULL);

		resolver.cache->stats(min, avg, max, sum);
		snprintf(buf, MAX_LEN, "\002-\002 resolver::cache::stats (min/avg/max/total): %d/%g/%d/%d", min, avg, max, sum);
		net.sendOwner(name, buf, NULL);
		resolver.unlock_data();
	}
#else
	net.sendOwner(name, "Resolving threads: \002not supported\002", NULL);
#endif

}

entServer *client::getRandomServer()
{
	//count servers
	int i, num;

#ifdef HAVE_SSL
	for(num=i=0; i<MAX_SERVERS*2; i++)
#else
	for(num=i=0; i<MAX_SERVERS; i++)
#endif
		if(!config.server[i].isDefault())
			++num;


	if(!num)
		return 0;

	DEBUG(printf("[D] got irc servers: %d\n", num));
	//get random one
	num = (rand() % num + 1);
	DEBUG(printf("[D] connecting to: %d\n", num));

	for(i=0; ;++i)
	{
		if(!config.server[i].isDefault())
			--num;

		if(!num)
			return &config.server[i];
	}
}

void client::autoRecheck()
{
	if(nextRecheck && nextRecheck <= NOW)
	{
		recheckFlags();
		nextRecheck = 0;
	}
}

void client::newHostNotify()
{
	if(hostNotify)
	{
        //lame check ;-)
        if(!(userlist.me()->flags[GLOBAL] & HAS_P))
        {
            hostNotify = 0;
            return;
        }

        int i;

		if(config.bottype == BOT_MAIN)
		{
            userlist.addHost(userlist.first->next, mask, NULL, 0, MAX_HOSTS-1);
			net.send(HAS_B, S_PROXYHOST, " ", (const char *) config.handle, " ", (const char *) mask, NULL);
			++userlist.SN;
			hostNotify = 0;
		}
		else
		{
			for(i=0; i<net.max_conns; ++i)
			{
				if(net.conn[i].isMain() && net.conn[i].fd)
				{
					net.conn[i].send(S_PROXYHOST, " ", (const char *) config.handle, " ", (const char *) mask, NULL);
					hostNotify = 0;
					break;
				}
			}
		}
		if(net.hub.fd && net.hub.isMain())
		{
			net.hub.send(S_PROXYHOST, " ", (const char *) config.handle, " ", (const char *) mask, NULL);
			hostNotify = 0;
		}
	}
}

void client::restart()
{
	char buf[MAX_LEN];

	net.~inet();
#ifdef HAVE_ADNS
	delete resolver;
#endif
	snprintf(buf, MAX_LEN, "pid.%s", (const char *) config.nick);
	unlink(buf);

#ifdef HAVE_DEBUG
	execl(thisfile, thisfile, "-d", (const char *) config.file, NULL);
#else
	execl(thisfile, thisfile, (const char *) config.file, NULL);
#endif

	exit(1);
}

int client::jumps5(const char *proxyhost, int proxyport, const char *ircserver, int ircport, const char *owner)
{
	char buf[MAX_LEN];
	if(!ircport) ircport = 6667;

	if(!inet::gethostbyname(proxyhost, buf, AF_INET))
	{
		net.sendOwner(owner, "[-] Cannot resolve ", proxyhost, ": ", hstrerror(h_errno), NULL);
		memset(&socks5, 0, sizeof(socks5));
		return 0;
	}
	socks5.setup(buf, proxyport, ircserver, ircport);

	net.irc.send("QUIT :changing servers", NULL);
	net.irc.close("changing servers");
	nextReconnect = 0;
	return ME.connectToIRC();
}

int client::jump(const char *host, const char *port, const char *owner, int protocol)
{
	entServer s("server",
#ifdef HAVE_IPV6
	new entHost("host", (protocol == AF_INET ? entHost::ipv4 : protocol == AF_INET6 ? entHost::ipv6 : 0) | entHost::domain),
#else
	new entHost("host", (protocol == AF_INET ? entHost::ipv4 : 0) | entHost::domain),
#endif
	new entInt("port", 1, 65535, 0), new entWord("pass", 0, 256));

	options::event *e = s.set(host, (port && *port) ? port : "6667");
	if(!e || !e->ok)
	{
		net.sendOwner(owner, e->reason, NULL);
		return 0;
	}

	net.irc.send("QUIT :changing servers", NULL);
	net.irc.close("changing servers");

	net.send(HAS_N, "[*] Jumping to ", (const char *) s.getHost().connectionString, " port ", itoa(s.getPort()), NULL);
        nextReconnect = 0;
	return ME.connectToIRC(&s);
}

void client::inviteRaw(const char *str)
{
	char arg[3][MAX_LEN];

	str2words(arg[0], str, 3, MAX_LEN, 0);

	if(!strcmp(arg[0], "INVITE"))
	{
		chan *ch = ME.findChannel(arg[2]);
		if(ch && !ch->getUser(arg[1]))
			ch->invite(arg[1]);
	}
}

void client::registerWithNewNick(char *nick)
{
	magicNickCreator(nick);
	net.irc.send("NICK ", nick, NULL);
}

void client::rejoin(const char *name, int t)
{
	int i = userlist.findChannel(name);
	if(i != -1 && userlist.isRjoined(i) && joinDelay != -1)
	{
		userlist.chanlist[i].nextjoin = NOW + t;
		userlist.chanlist[i].status &= ~(JOIN_SENT | WHO_SENT);
		if(!t)
			rejoinCheck();
	}
}

void client::rejoinCheck()
{
	int i;
	if(!(net.irc.status & STATUS_REGISTERED && joinDelay != -1))
		return;

	for(i=0; i<MAX_CHANNELS && penalty < 10; ++i)
	{
		if(userlist.chanlist[i].name && userlist.chanlist[i].nextjoin <= NOW &&
			!(userlist.chanlist[i].status & JOIN_SENT) &&
			(!(userlist.chanlist[i].status & PRIVATE_CHAN) || userlist.isRjoined(i)))
		{
			if(!ME.findNotSyncedChannel(userlist.chanlist[i].name))
			{
				net.irc.send("JOIN ", (const char *) userlist.chanlist[i].name, " ", (const char *) userlist.chanlist[i].pass, NULL);
				userlist.chanlist[i].status |= JOIN_SENT;
				userlist.chanlist[i].status &= ~WHO_SENT;
				penalty += 2;
			}
		}
	}
}

void client::joinAllChannels()
{
    if(joinDelay ==  -1)
        return;

	int i, j;

	Pchar buf;

	buf.push("JOIN ");
	int n;

	for(j=i=0; i<MAX_CHANNELS; ++i)
	{
		if(userlist.chanlist[i].name && !*userlist.chanlist[i].pass && userlist.isRjoined(i) &&
			!(userlist.chanlist[i].status & JOIN_SENT) && !findNotSyncedChannel(userlist.chanlist[i].name))
		{
			userlist.chanlist[i].status |= JOIN_SENT;
			userlist.chanlist[i].status &= ~WHO_SENT;
			n = strlen(userlist.chanlist[i].name) + 1;
			if(userlist.chanlist[i].pass)
				n += strlen(userlist.chanlist[i].pass) + 1;

			if(buf.len + n > 500)
			{
				net.irc.send(buf.data, NULL);
				penalty += 2;
				buf.clean();
				buf.push("JOIN ");
				j = 0;
			}

			if(buf.len != 5)
				buf.push(",");
			buf.push((const char *) userlist.chanlist[i].name);
			/*
			if(userlist.chanlist[i].pass && *userlist.chanlist[i].pass)
			{
				buf.push(" ");
				buf.push(userlist.chanlist[i].pass);
			}
			*/
			++j;
		}
	}
	if(j)
		net.irc.send(buf.data, NULL);
	penalty += 2;
	buf.clean();

	buf.push("WHO ");
	for(j=i=0; i<MAX_CHANNELS; ++i)
	{
		if(userlist.chanlist[i].name && !*userlist.chanlist[i].pass && userlist.isRjoined(i) &&
			!(userlist.chanlist[i].status & WHO_SENT) && !findNotSyncedChannel(userlist.chanlist[i].name))
		{
			userlist.chanlist[i].status |= WHO_SENT;

			if(buf.len != 4)
				buf.push(",");
			buf.push((const char *) userlist.chanlist[i].name);
			++j;

			if(j == 10)
			{
				net.irc.send(buf.data, NULL);
				penalty += 11;
				buf.clean();
				buf.push("WHO ");
				j = 0;
			}
		}
	}

	if(j) net.irc.send(buf.data, NULL);
	penalty += j + 1;
}

void client::gotNickChange(const char *from, const char *to)
{
	char *a = strchr(from, '!');
	char *fromnick;
	chan *p = first;

	if(a) mem_strncpy(fromnick, from, abs(a - from) + 1);
	else mem_strcpy(fromnick, from);

	DEBUG(printf("[*] NICK change %s <-> %s\n", fromnick, to));
	while(p)
	{
		if(p->synced()) p->gotNickChange(from, to);
		p = p->next;
	}
	if(!strcmp(ME.nick, fromnick))
	{
		ME.nick = to;
		ME.mask = ME.nick;
		ME.mask += "!";
		ME.mask += ME.ident;
		ME.mask += "@";
		ME.mask += ME.host;

		net.propagate(NULL, S_CHNICK, " ", (const char *) ME.nick, NULL);
		net.send(HAS_N, "[*] I am known as ", to, NULL);
		hostNotify = 1;
	}

	HOOK(nick, nick(fromnick, to));

	free(fromnick);
}

#ifdef HAVE_DEBUG
void client::display()
{
	chan *p = first;

	printf("### Channels:\n");
	while(p)
	{
		printf("### '%s'\n", (const char *) p->name);
		p = p->next;
	}
}
#endif

void client::recheckFlags()
{
	chan *ch = ME.first;

	while(ch)
	{
		if(ch->synced()) ch->recheckFlags();
		ch = ch->next;
	}
}

void client::recheckFlags(const char *channel)
{
	chan *ch = findChannel(channel);
	if(ch) ch->recheckFlags();
}

int client::connectToHUB()
{
	int fd, i;

	if(config.hub.failures < 3)
	{
		config.currentHub = &config.hub;
		DEBUG(printf(">>> hub = hub\n"));
	}
	else
	{
		for(i=0; i<MAX_ALTS; ++i)
		{
			if(!config.alt[i].isDefault())
			{
				if(!config.currentHub)
				{
					config.currentHub = &config.alt[i];
					DEBUG(printf(">>> hub = alt[%d]\n", i));
				}
				else if(config.currentHub->failures > config.alt[i].failures)
				{
					config.currentHub = &config.alt[i];
					DEBUG(printf(">>> hub = alt[%d]\n", i));
				}
			}
		}
	}

	if(!config.currentHub) config.currentHub = &config.hub;

	DEBUG(printf("[*] Connecting to HUB: %s port %d\n",
		  (const char *) config.currentHub->getHost().connectionString, (int) config.currentHub->getPort()));

	fd = doConnect((const char *) config.currentHub->getHost().ip, config.currentHub->getPort(), config.myipv4, -1);

	if(fd > 0)
	{
		net.hub.fd = fd;
		net.hub.status = STATUS_SYNSENT;
		net.hub.killTime = set.AUTH_TIME + NOW;

#ifdef HAVE_SSL
        if(config.currentHub->getHost().isSSL())
        {
            net.hub.enableSSL();
            net.hub.status |= STATUS_SSL_WANT_CONNECT | STATUS_SSL_HANDSHAKING;
        }
#endif

		return fd;
	}
	++config.currentHub->failures;
	return -1;
}

int client::connectToIRC(entServer *s)
{
	int n;
	int opt = 0;

	net.irc.status = 0;
	if(socks5.use())
	{
		n = socks5.connect();
		opt = STATUS_SOCKS5;
	}
	else if(!config.bnc.isDefault())
	{
		DEBUG(printf("[D] Using bnc to connect: %s %d (%s)\n",
			(const char *) config.bnc.getHost(),
			(int) config.bnc.getPort(),
			(const char *) config.bnc.getValue()));
		n = doConnect((const char *) config.bnc.getHost().ip, config.bnc.getPort(), 0, -1);
		opt = STATUS_BNC;
	}
	else if(!config.router.isDefault())
	{
		n = doConnect((const char *) config.router.getHost().ip, config.router.getPort(), 0, -1);
		opt = STATUS_ROUTER;
	}
	else
	{
		if(!s)
		{
			s = getRandomServer();
			if(!s)
				return 0;
		}

		//hack oidentd
		if(!config.oidentd_cfg.isDefault())
		{
			FILE *f = fopen(config.oidentd_cfg, "w");
			if(f)
			{
				fprintf(f, "global { reply \"%s\"}\n", (const char *) config.ident);
				fclose(f);
			}
			else
				net.send(HAS_N, "[!] Cannot create ", (const char *) config.oidentd_cfg, ": ", strerror(errno), NULL);
		}
#ifdef HAVE_IPV6
		if(s->getHost().isIpv6())
			n = doConnect6(s->getHost().ip, s->getPort(), config.vhost, -1);
		else
#endif
			n = doConnect((const char *) s->getHost().ip, s->getPort(), config.vhost, -1);
	}

	if(n > 0)
	{
		const char *pass;
		memset(&net.irc, 0, sizeof(inetconn));
		net.irc.fd = n;
		net.irc.status |= STATUS_SYNSENT | opt;
		net.irc.killTime = set.AUTH_TIME + NOW;
		net.irc.pass = NULL;

		if(s)
		{
		    pass = (const char *) s->getPass();
		    if(pass && *pass)
			net.irc.pass = (char *) pass;
		}

#ifdef HAVE_SSL
		if(s->isSSL())
		{
			net.irc.enableSSL();
			net.irc.status |= STATUS_SSL_WANT_CONNECT | STATUS_SSL_HANDSHAKING;
		}
#endif
		HOOK(connecting, connecting());
		return n;
	}
	return 0;
}

void client::checkQueue()
{
	if(penalty >= 10) return;
	chan *ch = first;

	autoRecheck();
	protmodelist::expireAll();

	/*
	 *	-1 - waiting for my host
	 *   0 - normal rejoin mode
	 *  >0 - burst join
	 */

	if(joinDelay == -1)
	{
		if(userlist.wildFindHost(userlist.me(), ME.mask) != -1)
		{
			joinDelay = NOW + set.QUARANTINE_TIME;
			DEBUG(net.send(HAS_N, "[D] kk, thx", NULL));
		}
	}
	else if(!joinDelay)
	{
		rejoinCheck();
	}
	else if(joinDelay <= NOW)
	{
		joinDelay = 0;
		joinAllChannels();
	}

	while(ch)
	{
		if(ch->synced())
		{
#ifdef HAVE_ADNS
			ch->updateDnsEntries();
#endif
			ch->recheckShits();
			ch->checkList();

			if(ch->chset->KEEPOUT && (ch->me->flags & IS_OP))
				ch->checkKeepout();

			ch->checkProtectedChmodes();

			if(!(ch->me->flags & IS_OP))
			{
				if(!ch->opedBots.entries()) ch->initialOp = 0;
				if(ch->chset->BOT_AOP_MODE == 0 ||
					(ch->chset->BOT_AOP_MODE == 1 && ch->toKick.entries() <= 4) ||
					ch->initialOp <= NOW - set.ASK_FOR_OP_DELAY && ch->initialOp)
				{
					ch->requestOp();
					ch->initialOp = NOW + set.ASK_FOR_OP_DELAY;
				}
			}
			else
			{
				/*
				if(ch->toKick.entries() - ch->sentKicks)
				{
					j = getRandomItems(MultHandle, ch->toKick.start(), ch->toKick.entries() - ch->sentKicks, 4, KICK_SENT);
					ch->kick4(MultHandle, j);
					return;
				}
				*/
				if(!ch->flushKickQueue())
				{
					ch->updateLimit();
					ch->modeQ[PRIO_HIGH].flush(PRIO_HIGH);
					ch->modeQ[PRIO_LOW].flush(PRIO_LOW);
				}
			}

			if(set.CLONE_LIFE_TIME)
			{
				ch->proxyClones.expire(set.CLONE_LIFE_TIME, NOW);
				ch->hostClones.expire(set.CLONE_LIFE_TIME, NOW);
				ch->identClones.expire(set.CLONE_LIFE_TIME, NOW);
			}
			ch->wasop->expire();
		}
		ch = ch->next;
	}

	/* handle dynamic bans, invites and exempts */
	ch = first;
	ptrlist<masklist_ent>::link *m;
	while(ch)
	{
		if(ch->synced() && ch->me->flags & IS_OP)
		{

			if(ch->chset->DYNAMIC_BANS)
			{
				m = NULL;
				while((m = ch->list[BAN].expire(m)))
				{
					ch->modeQ[PRIO_LOW].add(NOW + penalty, "-b ", m->ptr()->mask);
					m = m->next();
					if(!m) break;
				}
			}
			if(ch->chset->DYNAMIC_INVITES)
			{
				m = NULL;
				while((m = ch->list[INVITE].expire(m)))
				{
					ch->modeQ[PRIO_LOW].add(NOW + penalty, "-I ", m->ptr()->mask);
					m = m->next();
					if(!m) break;
				}
			}
			if(ch->chset->DYNAMIC_EXEMPTS)
			{
				m = NULL;
				while((m = ch->list[EXEMPT].expire(m)))
				{
					ch->modeQ[PRIO_LOW].add(NOW + penalty, "-e ", m->ptr()->mask);
					m = m->next();
					if(!m) break;
				}
			}
			ch->modeQ[PRIO_LOW].flush(PRIO_LOW);

			if(ch->synced() >= 3 && ch->me->flags & IS_OP)
			{
				int n = ch->opedBots.entries() + ch->botsToOp.entries();

				if(!(ch->flags & (FLAG_I | SENT_I)) && n < set.CRITICAL_BOTS &&	ch->chset->LOCKDOWN)
				{
					ch->flags |= CRITICAL_LOCK;

					if(ch->myTurn(ch->chset->GUARDIAN_BOTS))
						ch->modeQ[PRIO_HIGH].add(NOW, "+i");
				}
				else if(ch->flags & CRITICAL_LOCK && n >= set.CRITICAL_BOTS)
				{
					if(ch->myTurn(ch->chset->GUARDIAN_BOTS))
						ch->modeQ[PRIO_HIGH].add(NOW, "-i");
					else
						ch->modeQ[PRIO_HIGH].add(NOW+ch->myPos()*set.BACKUP_MODE_DELAY, "-i");

				}
			}
		}
		ch = ch->next;
	}

	if(penalty) return;

	/* if nothing to do, gather channel information */
	int level, i;
	Pchar buf;
	for(level=1; level<10; level+=2)
	{
		ch = first;
		i = 0;
		while(ch)
		{
			if(ch->synlevel == level)
			{
				if(i) buf.push(",");
				buf.push((const char *) ch->name);
				++ch->synlevel;
				++i;
				if(i == 4) break;
			}
			ch = ch->next;
		}
		if(i)
		{
			switch(level)
			{
				case 1: net.irc.send("MODE ", buf.data, NULL); break;
				case 3: net.irc.send("MODE ", buf.data, " b", NULL); break;
				case 5: net.irc.send("MODE ", buf.data, " I", NULL); break;
				case 7: net.irc.send("MODE ", buf.data, " e", NULL); break;
				case 9: net.irc.send("MODE ", buf.data, " R", NULL); break;
				default: break;
			}
			penalty += i + 1;
			return;
		}
	}

	/* realy nothing to do ;p */
	ch = first;
	while(ch && !penalty)
	{
		if(ch->synced() && ch->me->flags & IS_OP &&
			userlist.chanlist[ch->channum].status & SET_TOPIC)
		{
			srand();
			net.irc.send("TOPIC ", (const char *) ch->name, " :[\002Psotnic\002]: ", topics[rand() % count(topics)], NULL);
			userlist.chanlist[ch->channum].status &= ~SET_TOPIC;
			penalty++;
			return;
		}
		ch = ch->next;
	}
}

void client::gotUserQuit(const char *mask, const char *reason)
{
	char *a, *nick;
	chan *ch = first;
	int netsplit = reason && wasoptest::checkSplit(reason);

	a = strchr(mask, '!');
	if(a) mem_strncpy(nick, mask, abs(a - mask) + 1);
	else mem_strcpy(nick, mask);

	while(ch)
	{
                if (ch->synced()) {
                  HOOK(pre_part, pre_part(mask, ch->name));
                  HOOK(pre_partMsg, pre_partMsg(mask, ch->name, reason, true));
		  ch->gotPart(nick, netsplit);
                  HOOK(post_part, post_part(mask, ch->name));
                  HOOK(post_partMsg, post_partMsg(mask, ch->name, reason, true));
                }
		ch = ch->next;
	}

	if(config.keepnick && !netsplit && !strcasecmp(nick, config.nick))
	{
		net.irc.send("NICK ", (const char *) config.nick, NULL);
		ME.nextNickCheck = 0;
	}

	free(nick);
}

void client::removeChannel(const char *name)
{
	chan *ch = first;
	int n;

	if(!channels) return;

	if(!strcasecmp(first->name, name))
	{
		first = first->next;
		if(first) first->prev = NULL;
		delete(ch);
		--channels;
		if(!channels) last = NULL;
	}
	else if(!strcasecmp(last->name, name))
	{
		ch = last->prev;
		ch->next = NULL;
		delete(last);
		--channels;
		last = ch;
	}
	else
	{
		ch = first->next;
		while(ch)
		{
			if(!strcasecmp(ch->name, name))
			{
				ch->prev->next = ch->next;
				if(ch->next) ch->next->prev = ch->prev;
				delete(ch);
				--channels;
				break;
			}
			ch = ch->next;
		}
	}
	current = first;

	n = userlist.findChannel(name);
	if(n != -1) userlist.chanlist[n].status &= ~(JOIN_SENT | WHO_SENT);
}

chan *client::findNotSyncedChannel(const char *name)
{

	if(current)
		if(!strcasecmp(current->name, name)) return current;

	current = first;
	while(current)
	{
		if(!strcasecmp(current->name, name)) return current;
		current = current->next;
	}
	return NULL;
}

chan *client::findChannel(const char *name)
{

	if(current)
		if(!strcasecmp(current->name, name) && current->synced()) return current;

	current = first;
	while(current)
	{
		if(current->synced() && !strcasecmp(current->name, name)) return current;
		current = current->next;
	}
	return NULL;
}


chan *client::createNewChannel(const char *name)
{
	int n = userlist.findChannel(name);

	if(n > -1 && userlist.isRjoined(n))
	{
		if(!channels)
		{
			first = current = last = new(chan);
			current->prev = current->next = NULL;
			current->name = name;
		}
		else
		{
			current = last->next = new(chan);
  			current->prev = last;
			current->next = NULL;
			current->name = name;
			last = current;
		}

		userlist.chanlist[n].status &= ~JOIN_SENT;
		current->chset = userlist.chanlist[n].chset;
		current->wasop = userlist.chanlist[n].wasop;
		current->protlist[BAN] = userlist.chanlist[n].protlist[BAN];
		current->protlist[INVITE] = userlist.chanlist[n].protlist[INVITE];
		current->protlist[EXEMPT] = userlist.chanlist[n].protlist[EXEMPT];
		current->protlist[REOP] = userlist.chanlist[n].protlist[REOP];

		current->channum = n;
		if(userlist.chanlist[n].allowedOps &&
			userlist.chanlist[n].allowedOps->since + 60 < NOW)
		{
			delete userlist.chanlist[n].allowedOps;
			userlist.chanlist[n].allowedOps = NULL;
		}
		current->key = userlist.chanlist[n].pass;

		++channels;
		return current;
	}
	return NULL;
}

/* Constructor */
client::client()
{
	NOW = time(NULL);
	first = last = current = NULL;
	overrider = uid = ircip = nick = ident = host = mask = "";
	nextReconnect = nextRecheck = nextNickCheck = 0;
	nextConnToIrc = nextConnToHub = startedAt = NOW;
	channels = joinDelay = hostNotify = 0;
	ircConnFailDelay = 15;
}

/* Destruction derby */
client::~client()
{
	chan *ch = first;
	chan *p;

	while(ch)
	{
		p = ch;
		ch = ch->next;
		delete p;
	}
}

void client::reset()
{
	chan *ch = first;
	chan *p;

	while(ch)
	{
		p = ch;
		ch = ch->next;

		//p->wasop is infact chanlist->wasop
		//so it will preserve itself across delete/new
		if(p->wasop && !p->wasop->isEmpty())
		{
			ptrlist<chanuser>::iterator u = p->users.begin();
			while(u)
			{
				if((u->flags & (IS_OP | HAS_F)) == IS_OP && !p->toKick.find(u))
					p->wasop->add(u);

				u++;
			}
		}
		delete p;
	}

	first = last = current = NULL;
	overrider = uid = ircip = nick = ident = host = mask = "";
	nextRecheck = nextNickCheck = 0;
	channels = joinDelay = hostNotify = 0;
	ircConnFailDelay = 15;

	for(int i=0; i<MAX_CHANNELS; ++i)
	{
		if(userlist.chanlist[i].allowedOps)
		{
			delete userlist.chanlist[i].allowedOps;
			userlist.chanlist[i].allowedOps = NULL;
		}
		userlist.chanlist[i].status &= PRIVATE_CHAN;
		//userlist.chanlist[i].nextjoin = 0;
	}
}
