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

void parse_hub(char *data)
{
	char arg[10][MAX_LEN];
	chan *ch;

	if(!strlen(data)) return;
	str2words(arg[0], data, 10, MAX_LEN);

	if(!(net.hub.status & STATUS_REGISTERED))
	{
		switch(net.hub.tmpint)
		{
			/*
			case 0:
			{
				//3 bytes for WILL ECHO OFF + 1 byte for NEW LINE
				++net.hub.tmpint;
				//enable encryption
				net.hub.enableCrypt((unsigned char *) config.botnetword, strlen(config.botnetword));
				return;
			}
			*/
			case 1:
			{
				if(strlen(arg[0]))
				{
					char hash[33];
					++net.hub.tmpint;
					//unsigned char *dupa = ((entMD5Hash *) &config.currentHub->getPass())->getHash();

					MD5HexHash(hash, arg[0], AUTHSTR_LEN, ((entMD5Hash *) &config.hub.getPass())->getHash(), 16);
					net.hub.send(config.handle, " ", hash, NULL);

					net.hub.tmpstr = (char *) malloc(AUTHSTR_LEN + 1);
					MD5CreateAuthString(net.hub.tmpstr, AUTHSTR_LEN);
					net.hub.send(net.hub.tmpstr, NULL);
                    return;
				}
				break;
			}
			case 2:
			{
				if(strlen(arg[3]))
				{
					if(MD5HexValidate(arg[3], net.hub.tmpstr, strlen(net.hub.tmpstr), ((entMD5Hash *) &config.hub.getPass())->getHash(), 16))
					{
						char buf[MAX_LEN];

						++net.hub.tmpint;
						userlist.addHandle(arg[0], 0, B_FLAGS | HAS_H | HAS_L, arg[1], arg[2], 0);
						net.hub.handle = userlist.findHandle(arg[0]);
						DEBUG(printf("[D] hub handle: %s\n", net.hub.handle->name));
						free(net.hub.tmpstr);
						net.hub.tmpstr = NULL;

						if(config.bottype != BOT_SLAVE)
							sprintf(buf, "%llu", userlist.SN);
						else
							strcpy(buf, "0");

						net.hub.send(S_REGISTER, " ", S_VERSION, " ", buf, " ", (const char *) ME.nick, " ", net.irc.origin, NULL);
						return;
					}
				}
				break;
			}
			case 3:
			{
				if(!strcmp(arg[0], S_REGISTER))
				{
					mem_strcpy(net.hub.name, arg[1]);
					net.hub.tmpint = 0;
					net.hub.status |= STATUS_CONNECTED | STATUS_REGISTERED | STATUS_BOT;
					net.hub.killTime = NOW + set.CONN_TIMEOUT;
					net.hub.lastPing = NOW;

					net.hub.enableCrypt(((entMD5Hash *) &config.hub.getPass())->getHash(), 16);

					net.sendBotListTo(&net.hub);
					net.propagate(&net.hub, S_BJOIN, " ", net.hub.name, NULL);
					config.currentHub->failures = 0;
					net.propagate(NULL, S_CHNICK, " ", (const char *) ME.nick, " ", net.irc.origin, NULL);
					return;
				}
			}
			default: break;
		}
		/* HUH */
		net.hub.close("Access Denied");
	}

	/* REGISTERED HUB */
	net.hub.killTime = NOW + set.CONN_TIMEOUT;

	if(!strcmp(arg[0], S_UL_UPLOAD_START))
	{
		if(userlist.ulbuf)
		{
			net.send(HAS_N, "[!] Double UL download, this should not happen", NULL);
			sleep(5);
			net.send(HAS_N, "[!] Terminating.", NULL);
			exit(1337);
		}
		userlist.ulbuf = new Pchar(64*1024);
		return;
	}
	if(!strcmp(arg[0], S_UL_UPLOAD_END))
	{
		if(!userlist.ulbuf)
		{
			net.send(HAS_N, "[!] Update userlist is empty", NULL);
			net.send(HAS_N, "[-] Disconnecting", NULL);
			net.hub.close("Userlist is empty");
			return;
		}
		
		userlist.update();
		if(userlist.me()->flags[GLOBAL] & HAS_P)
			hostNotify = 1;
		else
			hostNotify = 0;

		userlist.sendToAll();
		return;
	}
	if(userlist.ulbuf)
	{
		userlist.ulbuf->push(data);
		userlist.ulbuf->push("\n");
		return;
	}

	if(!strcmp(arg[0], S_CYCLE) && strlen(arg[1]))
	{
		if(ME.findChannel(arg[1]))
		{
			net.irc.send("PART ", arg[1], " :", (const char *) config.cyclereason, NULL);
			ME.rejoin(arg[1], set.CYCLE_DELAY);

			if(strlen(arg[2]))
				net.send(HAS_N, "[*] Doing cycle on ", arg[1], NULL);

		}
		net.propagate(&net.hub, data, NULL);
		return;
	}
	if(!strcmp(arg[0], S_MKA) && strlen(arg[1]))
	{
		ch = ME.findChannel(arg[1]);
		if(ch) ch->massKick(MK_ALL, !strcmp(arg[3], "close") || !strcmp(arg[3], "lock"));
		return;
	}
	if(!strcmp(arg[0], S_MKO) && strlen(arg[1]))
	{
		ch = ME.findChannel(arg[1]);
		if(ch) ch->massKick(MK_OPS, !strcmp(arg[3], "close") || !strcmp(arg[3], "lock"));
		return;
	}
	if(!strcmp(arg[0], S_MKN) && strlen(arg[1]))
	{
		ch = ME.findChannel(arg[1]);
		if(ch) ch->massKick(MK_NONOPS, !strcmp(arg[3], "close") || !strcmp(arg[3], "lock"));
		return;
	}
	if(!strcmp(arg[0], S_UNLINK) && strlen(arg[1]))
	{
		HANDLE *h = userlist.findHandle(arg[1]);

		if(h && userlist.isBot(h))
		{
			inetconn *bot = net.findConn(h);
			if(bot) bot->close("Forced unlink");
		}
		return;
	}
	if(!strcmp(arg[0], S_NICK) && strlen(arg[1]))
	{
		net.irc.send("NICK ", arg[1], NULL);
		ME.nextNickCheck = NOW + set.KEEP_NICK_CHECK_DELAY;
		return;
	}
	if(!strcmp(arg[0], S_JUMP) && strlen(arg[2]))
	{
		ME.jump(arg[2], arg[3], arg[1]);
		return;
	}
#ifdef HAVE_IPV6
	if(!strcmp(arg[0], S_JUMP6) && strlen(arg[2]))
	{
		ME.jump(arg[2], arg[3], arg[1], AF_INET6);
		return;
	}
#endif
	if(!strcmp(arg[0], S_JUMPS5) && strlen(arg[5]))
	{
		ME.jumps5(arg[2], atoi(arg[3]), arg[4], atoi(arg[5]), arg[1]);
		return;
	}

	if(!strcmp(arg[0], S_RDIE) && strlen(arg[1]))
	{
		net.send(HAS_N, "[!] ", DIE_REASON, NULL);
		net.irc.send("QUIT :", arg[1], " ", DIE_REASON2, NULL);
		safeExit();
	}
	if(!strcmp(arg[0], S_NAMES) && strlen(arg[2]))
	{
		ch = ME.findChannel(arg[2]);
		if(ch)
			ch->names(arg[1]);
		else net.sendOwner(arg[1], "Invalid channel", NULL);
		return;
	}

	if(!strcmp(arg[0], S_CWHO) && strlen(arg[2]))
	{
		ch = ME.findChannel(arg[2]);
		if(ch)
			ch->cwho(arg[1], arg[3]);
		else net.sendOwner(arg[1], "Invalid channel", NULL);
		return;
	}
	if(!strcmp(arg[0], S_PSOTUPDATE))
	{
		psotget.forkAndGo(arg[1]);
		return;
	}
	if(!strcmp(arg[0], S_STOPUPDATE))
	{
		psotget.end();
		return;
	}
	if(!strcmp(arg[0], S_RESTART))
	{
		ME.restart();
		return;
	}
	if(!strcmp(arg[0], S_ULSAVE))
	{
		userlist.save(config.userlist_file);
		ME.nextRecheck = NOW + 5;
		net.propagate(&net.hub, data, NULL);
		return;
	}
	if(!strcmp(arg[0], S_RJOIN) && strlen(arg[2]))
	{
		userlist.rjoin(arg[1], arg[2]);
		net.propagate(&net.hub, data, NULL);
		++userlist.SN;
		return;
	}
	if(!strcmp(arg[0], S_RPART) && strlen(arg[2]))
	{
		userlist.rpart(arg[1], arg[2], arg[3]);
		net.propagate(&net.hub, data, NULL);
		++userlist.SN;
		return;
	}
	if(!strcmp(arg[0], S_STATUS) && strlen(arg[1]))
	{
		ME.sendStatus(arg[1]);
		return;
	}
	if(!strcmp(arg[0], S_CHKHOST) && strlen(arg[1]))
	{
		ME.checkMyHost(arg[1]);
		return;
	}

	if(parse_botnet(&net.hub, data)) return;

	if(userlist.parse(data))
	{
		++userlist.SN;

		//some things should not be propagated
		if(config.bottype == BOT_SLAVE)
		{
			if(!strcmp(S_ADDBOT, arg[0]))
			{
				net.propagate(&net.hub, S_ADDBOT, " ", arg[1], " ", arg[2], " ", arg[3], " ", S_SECRET, NULL);
				return;
			}
			if(!strcmp(S_PASSWD, arg[0]) && userlist.isBot(arg[1]))
			{
				net.propagate(&net.hub, S_PASSWD, " ", arg[1], " ", "00000000000000000000000000000000", NULL);
				return;
			}
			if(!strcmp(S_ADDR, arg[0]) && userlist.isBot(arg[1]))
			{
				net.propagate(&net.hub, S_ADDR, " ", arg[1], " ", "0.0.0.0", NULL);
				return;
			}
			if(!strcmp(S_ADDOFFENCE, arg[0])) // leaf dont need infos about offence-history
				return;

		}
		net.propagate(&net.hub, data, NULL);
		return;
	}
}
