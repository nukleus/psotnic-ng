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

int parse_owner_join(inetconn *c, char *name, char *key, int delay, char *flags, char *bot=NULL)
{
	int n;
	
	if(c && !c->checkFlag(HAS_S))
	{
		c->send(S_NOPERM, NULL);
		return 0;
	}

	if(!chan::valid(name))
	{
		if(c) c->send("Invalid channel name", NULL);
		return 0;
	}

	//.mjoin, .tkmjoin
	if(!bot)
	{
		bool sendDset = false;
		bool alreadyAdded = (userlist.findChannel(name)!=-1)?true:false;

//		n = userlist.findChannel(name);		

//		if(n == -1)
//		{
			n = userlist.addChannel(name, key, flags);
			
			if(n < 0)
			{
				if(c)
					c->send("Channel list is full", NULL);
				return 0;
			}

			if(!alreadyAdded)
			{			
				*userlist.chanlist[n].chset = *userlist.dset;
				sendDset = true;
			}
//		}

		if(strchr(flags, 'P'))
			net.sendCmd(c, "+chan ", name, " ", key, " ", delay ? itoa(delay) : NULL, NULL);
		else if(strchr(flags, 'T'))
		{
			net.sendCmd(c, "tkmjoin ", name, " ", key, " ", delay ? itoa(delay) : NULL, NULL);
			userlist.chanlist[n].chset->TAKEOVER = 1;
		}
			
		else if(strchr(flags, '*'))
			net.sendCmd(c, "mjoin ", name, " ", key, " ", delay ? itoa(delay) : NULL, NULL);
		else
			net.sendCmd(c, "bugjoin ", name, " ", key, " ", itoa(delay), NULL);

		if(c)
			c->send("Channel `\002", name, "\002' has been added to channel list", NULL);

		if(delay < 0)
		{
			net.send(HAS_B, S_ADDCHAN, " ", flags, " ", name, " ", key, " ", itoa(-delay), NULL);
			if(sendDset)
				userlist.sendDSetsToBots(name);
			ME.rejoin(name, -delay);
			if(c)
				c->send("Delaying mass join by ", itoa(-delay), " seconds", NULL);
		}
		else if(delay > 0)
		{
			int j = delay;
			for(int i=0; i<net.max_conns; ++i)
			{
				if(net.conn[i].isRegBot())
				{
					if(net.conn[i].name && *net.conn[i].name && userlist.isRjoined(n, net.conn[i].handle))
					{
						net.conn[i].send(S_ADDCHAN, " ", flags, " ", name, " ", key, " ", itoa(j), NULL);
						j += delay;
					}
					else
						net.conn[i].send(S_ADDCHAN, " ", flags, " ", name, " ", key, " ", NULL);
				}
			}
			if(sendDset)
				userlist.sendDSetsToBots(name);
			if(c)
				 c->send("Setting delay between bots joins to ", itoa(delay), " seconds", NULL);
			
			ME.rejoin(name, 0);
		}
		else
		{
			net.send(HAS_B, S_ADDCHAN, " ", flags, " ", name, " ", key, NULL);
			if(sendDset)
				userlist.sendDSetsToBots(name);
			ME.rejoin(name, 0);
		}
		++userlist.SN;
		userlist.nextSave = NOW + SAVEDELAY;
		return 1;
	}
	// .rjoin
	else
	{
		if(!userlist.isBot(bot))
		{
			if(c)
				c->send("Invalid bot", NULL);
			return 0;
		}
		if(userlist.findChannel(name) == -1)
		{
			n = userlist.addChannel(name, key, flags);
			if(n < 0)
			{
				if(c) c->send("Channel list is full", NULL);
				return 0;
			}
			net.send(HAS_B, S_ADDCHAN, " ", flags, " ", name, " ", key, NULL);
			*userlist.chanlist[n].chset = *userlist.dset;
			userlist.sendDSetsToBots(name);
		}
		
		if(c)
		{
			if(strchr(flags, 'T'))
				net.sendCmd(c, "tkrjoin ", bot, " ", name, " ", key, NULL);
			else if(strchr(flags, 'P'))
				net.sendCmd(c, "rjoin ", bot, " ", name, " ", key, NULL);
			//else if(strchr(flags, '*')) net.sendCmd(c, "mjoin ", bot, " ", name, " ", key, NULL);
		}

		userlist.rjoin(bot, name);
		net.send(HAS_B, S_RJOIN, " ", bot, " ", name, NULL);
		++userlist.SN;
		userlist.nextSave = NOW + SAVEDELAY;
		return 1;
	}
}

void parse_owner_sjoin(inetconn *c, char *name, char *key, int delay, char *flags, char *bot)
{
	if(!c->checkFlag(HAS_S))
	{
		c->send(S_NOPERM, NULL);
		return;
	}
	if(!chan::valid(name))
	{
		c->send("Invalid channel name", NULL);
		return;
	}
	
	inetconn *slave = net.findConn(userlist.findHandle(bot));
	if(slave && slave->isSlave())
	{
		int j=0;
		if(parse_owner_join(NULL, name, key, delay < 0 ? delay : 0, flags, bot))
		{
			net.sendCmd(c, "sjoin ", bot, " ", name, " ", key, NULL);
			for(int i=0; i<net.max_conns; ++i)
				if(net.conn[i].isLeaf() && net.conn[i].fd == slave->fd)
					parse_owner_join(NULL, name, key, delay < 0 ? delay : j+=delay, flags, net.conn[i].handle->name);
		}
	}
	else c->send("Invalid slave", NULL);
}

int parse_owner_rpart(inetconn *c, char *name, char *bot)
{
	if(c && !c->checkFlag(HAS_S))
	{
		c->send(S_NOPERM, NULL);
		return 0;
	}
	switch(userlist.rpart(bot, name))
	{
		case -1:
		{
			if(c) c->send("Invalid bot", NULL);
			return 0;
		}
		case -2:
		{
			if(c) c->send("Invalid channel", NULL);
			return 0;
		}
		default:
		break;
	}

	if(c)
	{
		net.sendCmd(c, "rpart ", bot, " ", name, NULL);
	}
	net.send(HAS_B, S_RPART, " ", bot, " ", name, NULL);
	++userlist.SN;
	userlist.nextSave = NOW + SAVEDELAY;
	return 1;
}

void parse_owner_spart(inetconn *c, char *name, char *bot)
{
	if(!c->checkFlag(HAS_S))
	{
		c->send(S_NOPERM, NULL);
		return;
	}
	inetconn *slave = net.findConn(userlist.findHandle(bot));
	if(slave && slave->isSlave())
	{
		if(parse_owner_rpart(c, name, bot))
		{
			net.sendCmd(c, "spart ", bot, " ", name, NULL);
			for(int i=0; i<net.max_conns; ++i)
				if(net.conn[i].isLeaf() && net.conn[i].fd == slave->fd)
					parse_owner_rpart(NULL, name, net.conn[i].handle->name);
		}
	}
	else c->send("Invalid slave", NULL);
}

void parse_owner(inetconn *c, char *data)
{
	char arg[10][MAX_LEN], buf[MAX_LEN], *a, *b, *reason = NULL;
	HANDLE *h;
	int i, n;

	if(!strlen(data)) return;
	memset(buf, 0, MAX_LEN);
	str2words(arg[0], data, 10, MAX_LEN);

	/* NOT REGISTERED OWNER */

	if(!(c->status & STATUS_REGISTERED))
	{
		if(!(c->status & STATUS_TELNET))
		{
			switch(c->tmpint)
			{
				case 1:
				{
					if(MD5Validate(config.ownerpass, arg[0], strlen(arg[0])))
					{
						c->killTime = NOW + set.AUTH_TIME;
						++c->tmpint;
						c->send("Enter user password: ", NULL);
						return;
					}
					else
					{
						reason = push(NULL, c->tmpstr, ": bad ownerpass", NULL);
						break;
					}
				}
				case 2:
				{
					if(strlen(arg[0]) && (h = userlist.matchPassToHandle(arg[0], c->tmpstr)))
					{
						if(!(h->flags[GLOBAL] & HAS_P))
						{
							reason= push(NULL, c->tmpstr, ": no partyline privileges", NULL);
							break;
						}

						c->status |= STATUS_REGISTERED;
						c->tmpint = 0;
						c->killTime = 0;
						c->handle = h;
						mem_strcpy(c->name, h->name);
						sendLogo(c);
						return;
					}
					else
					{
						reason = push(NULL, c->tmpstr, ": bad userpass", NULL);
						break;
					}
				}
				default: break;
			}
	       	free(c->tmpstr);
	        c->tmpstr = NULL;
		}
		else
		{
			switch(c->tmpint)
			{
				case 1:
				{
					mem_strcpy(c->name, arg[0]);
					++c->tmpint;
					if(creation)
#ifdef HAVE_SSL
					if(c->isSSL())
						SSL_write(c->ssl, "future password: ", strlen("future password: "));
					else
#endif
						write(c->fd, "future password: ", strlen("future password: "));
					else
					{
						//c->echo(0);
#ifdef HAVE_SSL
					if(c->isSSL())
						SSL_write(c->ssl, "password: ", strlen("password: "));
					else
#endif
						write(c->fd, "password: ", strlen("password: "));
					}
					return;
				}
                case 2:
				{
					if(creation && strlen(arg[0]))
					{
						if(strlen(arg[0]) < 8)
						{
							c->send("Password must be at least 8 characters long", NULL);
							c->send("Better luck next time", NULL);
							c->close();
							return;
						}
						else
						{
							h = userlist.addHandle(c->name, 0, 0, 0, 0, c->name);
							if(h)
							{
								userlist.changePass(c->name, arg[0]);
                						userlist.changeFlags(c->name, "tpx", "");
								c->send("Account created", NULL);
								printf("[*] Account created\n");

                						userlist.save(config.userlist_file);
#ifdef HAVE_DEBUG
								if(!debug)
#endif
                    						    lurk();
                						creation = 0;
                						++userlist.SN;
							}
						}
					}
					if((h = userlist.checkPartylinePass(c->name, arg[0], HAS_P)))
					{
						if(set.TELNET_OWNERS == 2)
						{
							snprintf(buf, MAX_LEN, "*!*@%s", c->getPeerIpName());
							if(userlist.wildFindHostExt(h, buf) == -1)
							{
								reason = push(NULL, c->name, ": invalid ip", NULL);
								break;
							}
						}

						c->status |= STATUS_CONNECTED | STATUS_PARTY | STATUS_REGISTERED | STATUS_TELNET;
						c->tmpint = 0;
						c->killTime = 0;
						c->handle = h;
						if(c->tmpstr) free(c->tmpstr);
						c->tmpstr = NULL;
						//c->echo(1);
						sendLogo(c);
						ignore.removeHit(c->getPeerIp4());
						return;
					}
					else
					{
						if((h = userlist.findHandle(c->name)))
						{
							if(!(h->flags[GLOBAL] & HAS_P))
								reason = push(NULL, c->name, ": no partyline privileges", NULL);
							else
								reason = push(NULL, c->name, ": wrong user password", NULL);
						}
						else
							reason = push(NULL, c->name, ": invalid handle", NULL);

						break;
					}
				}
				default: break;
			}
		}
		if(!reason)
			reason = push(NULL, "Unknown error", NULL);
		c->close(reason);
		free(reason);
		return;
	}
	/* REGISTERED OWNER */

        if(arg[0][0] == '.')
        {
		HOOK(partylineCmd, partylineCmd(c->name, c->handle->flags[GLOBAL], arg[0], srewind(data, 1)));

		if(stopParsing)
		{
			stopParsing=false;
                	return;
		}
        }

	if(!strcmp(arg[0], ".bye") || !strcmp(arg[0], ".exit") || !strcmp(arg[0], ".quit"))
	{
		a = srewind(data, 1);
		c->close(a ? a : c->handle->name);
		return;
	}
	if(!strcmp(arg[0], ".+user") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(!isRealStr(arg[1]))
		{
			c->send("Invalid handle", NULL);
			return;
		}
		if(strlen(arg[1]) > MAX_HANDLE_LEN)
		{
			c->send("Too long handle name", NULL);
			return;
		}
		if(strlen(arg[2]))
		{
		    if(!extendhost(arg[2], buf, MAX_LEN))
		    {
			c->send("Invalid hostname", NULL);
			return;
		    }
		    
		    if((h = userlist.addHandle(arg[1], 0, 0, 0, 0, c->name)))
		    {
			userlist.addHost(h, buf, c->name, NOW);
			net.sendCmd(c, "+user ", arg[1], " ", buf, NULL);
			c->send("Adding user `\002", arg[1], "\002' with host `\002", buf, "\002'", NULL);
			net.send(HAS_B, S_ADDUSER, " ", arg[1], " ", h->creation->print(), NULL);
			net.send(HAS_B, S_ADDHOST, " ", arg[1], " ", buf, NULL);
			//ME.recheckFlags();
			userlist.SN += 2;
			userlist.nextSave = NOW + SAVEDELAY;			
		    }
		    else c->send("Handle exists", NULL);		
		}
		else
		{
		    if((h = userlist.addHandle(arg[1], 0, 0, 0, 0, c->name)))
		    {
			c->send("Adding user `\002", arg[1], "\002'", NULL);
			net.sendCmd(c, "+user ", arg[1], NULL);
			net.send(HAS_B, S_ADDUSER, " ", arg[1], " ", h->creation->print(), NULL);
			++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
		    }
		    else c->send("Handle exists", NULL);
		}
		return;
	}
	if(!strcmp(arg[0], ".-user") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}

		h = userlist.findHandle(arg[1]);
		if(!h || userlist.isBot(h))
		{
			c->send("Invalid handle", NULL);
			return;
		}
		switch(userlist.hasWriteAccess(c, arg[1]))
		{
			case -1:
			{
				c->send("Invalid user", NULL);
				break;
			}
			case 0:
			{
				c->send(S_NOPERM, NULL);
				break;
			}
			case 1:
			{
				if(userlist.removeHandle(arg[1]) == -1)
				{
					c->send("This handle is immortal", NULL);
					return;
				}
				net.sendCmd(c, "-user ", arg[1], NULL);
				c->send("Removing user `\002", arg[1], "\002'", NULL);
				net.send(HAS_B, S_RMUSER, " ", arg[1], NULL);
				//ME.recheckFlags();
				++userlist.SN;
				userlist.nextSave = NOW + SAVEDELAY;
				break;
			}
		}
		return;
	}
	if(!strcmp(arg[0], ".users"))
	{
		net.sendCmd(c, "users", NULL);
		if(!userlist.users) c->send("No users in userlist", NULL);
		else userlist.sendUsers(c);
		return;
	}
	if(!strcmp(arg[0], ".+host") && strlen(arg[2]))
	{
		if(!extendhost(arg[2], buf, MAX_LEN))
		{
			c->send("Invalid hostname", NULL);
			return;
		}
		h = userlist.findHandle(arg[1]);
		if(h)
		{
			if(userlist.hasWriteAccess(c, arg[1]) == 1)
			{
				if(userlist.addHost(h, buf, c->name, NOW) != -1)
				{
					net.sendCmd(c, "+host ", arg[1], " ", buf, NULL);
					c->send("Adding host `\002", buf, "\002' to handle `\002", arg[1], "\002'", NULL);
					net.send(HAS_B, S_ADDHOST, " ", arg[1], " ", buf, NULL);
					//ME.recheckFlags();
					++userlist.SN;
					userlist.nextSave = NOW + SAVEDELAY;
				}
				else c->send("Host exists", NULL);
			}
			else c->send(S_NOPERM, NULL);
		}
		else c->send("Invalid handle", NULL);
		return;
	}
	if(!strcmp(arg[0], ".-host") && strlen(arg[2]))
	{
		h = userlist.findHandle(arg[1]);
		if(h)
		{
			if(userlist.hasWriteAccess(c, arg[1]) == 1)
			{
				if((n = userlist.findHost(h, arg[2])) != -1)
				{
					net.sendCmd(c, "-host ", h->name, " ", h->host[n], NULL);
					c->send("Removing host `\002", h->host[n], "\002' from handle `\002", h->name, "\002'", NULL);
					net.send(HAS_B, S_RMHOST, " ", h->name, " ", h->host[n], NULL);
					userlist.removeHost(h, arg[2]);
					++userlist.SN;
					userlist.nextSave = NOW + SAVEDELAY;
				}
				else c->send("Invalid host", NULL);
			}
			else c->send(S_NOPERM, NULL);
		}
		else c->send("Invalid handle", NULL);
		return;
	}
	if((!strcmp(arg[0], ".wii") || !strcmp(arg[0], ".whois") || !strcmp(arg[0], ".wi")) && strlen(arg[1]))
	{
		h = userlist.findHandle(arg[1]);
		if(h)
		{
			if(userlist.hasReadAccess(c, h))
			{
				net.sendCmd(c, "whois ", arg[1], " ", arg[2], NULL);
				userlist.sendHandleInfo(c, h, arg[2]);
			}
			else c->send(S_NOPERM, NULL);
		}
		else c->send("Invalid handle", NULL);
		return;
	}
	

	if(!strcmp(arg[0], ".idiots"))
	{
		h = userlist.findHandle(userlist.first->name);
		if(h)
		{
			if(userlist.hasReadAccess(c, h))
			{
				net.sendCmd(c, "idiots", NULL);
				userlist.sendHandleInfo(c, h, arg[1]);
			}
			else c->send(S_NOPERM, NULL);
		}
		else c->send("Invalid handle", NULL);
		return;
	}
	
	if(!strcmp(arg[0], ".+idiot") && strlen(arg[1]))
	{
		if(!extendhost(arg[1], buf, MAX_LEN))
		{
			c->send("Invalid hostname", NULL);
			return;
		}
		h = userlist.findHandle(userlist.first->name);
		if(h)
		{
			if(userlist.hasWriteAccess(c, h->name) == 1)
			{
				if(userlist.addHost(h, buf, c->name, NOW) != -1)
				{
					net.sendCmd(c, "+idiot ", buf, NULL);
					c->send("Adding host `\002", buf, "\002' to handle `\002", h->name," \002'", NULL);
					net.send(HAS_B, S_ADDHOST, " ", h->name, " ", buf, NULL);
					//ME.recheckFlags();
					++userlist.SN;
					userlist.nextSave = NOW + SAVEDELAY;
				}
				else c->send("Host exists", NULL);
			}
			else c->send(S_NOPERM, NULL);
		}
		else c->send("Invalid handle", NULL);
		return;
	}
	if(!strcmp(arg[0], ".-idiot") && strlen(arg[1]))
	{
		h = userlist.findHandle(userlist.first->name);
		if(h)
		{
			if(userlist.hasWriteAccess(c, h->name) == 1)
			{
				if((n = userlist.findHost(h, arg[1])) != -1)
				{
					net.sendCmd(c, "-idiot ", h->host[n], NULL);
					c->send("Removing host `\002", h->host[n], "\002' from handle `\002", h->name, "\002'", NULL);
					net.send(HAS_B, S_RMHOST, " ", h->name, " ", h->host[n], NULL);
					userlist.removeHost(h, arg[1]);
					++userlist.SN;
					userlist.nextSave = NOW + SAVEDELAY;
				}
				else c->send("Invalid host", NULL);
			}
			else c->send(S_NOPERM, NULL);
		}
		else c->send("Invalid handle", NULL);
		return;
	}	
	if(!strcmp(arg[0], ".match") && strlen(arg[1]))
	{
		h = userlist.first;
		i = 0;
		int flags = userlist.str2userFlags(arg[2]);
		int num = GLOBAL;
    		bool req = false;
        
    		if(*arg[3])
        	    num = userlist.findChannel(arg[3]);
		    if(num == -1)
		    {
			c->send("Invalid channel", NULL);
			return;
		    }

    		if(!strcmp(arg[2], "*"))
        	    *arg[3] = '\0';
        
    		if(!strcmp(arg[2], "-"))
        	    req = true;
    		net.sendCmd(c, "match ", arg[1], " ", arg[2], " ", arg[3], NULL);

		while(h)
		{
			if(userlist.isBot(h) || !userlist.hasReadAccess(c, h))
			{
				h = h->next;
				continue;
			}

			if(*arg[2] && !flags)
			{
				if(h->flags[num])
				{
					h = h->next;
					continue;
				}
			}
			else if((h->flags[num] & flags) != flags)
			{
				h = h->next;
				continue;
			}
			if(match(arg[1], h->name) || userlist.wildFindHostExt(h, arg[1]) != -1)
			{
				if(i < set.MAX_MATCHES || !set.MAX_MATCHES)
            			{
                			if(i) c->send("---", NULL);
					userlist.sendHandleInfo(c, h, arg[1]);
				}
				++i;
			}
			h = h->next;
		}
		if(i >= set.MAX_MATCHES && set.MAX_MATCHES)
        	c->send("(more than ", itoa(set.MAX_MATCHES), " matches, list truncated)", NULL);

		if(!i) c->send("No matches has been found", NULL);
		else c->send("--- Found ", itoa(i), " match", i == 1 ? "" : "es", " for '", arg[1], "'", NULL);
    		return;
	}
	if(!strcmp(arg[0], ".bots") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		net.sendCmd(c, "bots ", arg[1], " ", arg[2], NULL);

		h = userlist.first;
		i = 0;
		int flags = (userlist.str2botFlags(arg[2]) & (HAS_L | HAS_H)) | HAS_B;

		while(h)
		{
			if((h->flags[GLOBAL] & HAS_B) && ((h->flags[GLOBAL] & (HAS_L | HAS_H | HAS_B)) == flags || !*arg[2]) && (match(arg[1], h->name)
				|| userlist.wildFindHostExt(h, arg[1]) != -1 || match(arg[1], inet2char(h->ip))))
			{
				if(i < set.MAX_MATCHES || !set.MAX_MATCHES)
                {
                	if(i) c->send("---", NULL);
					userlist.sendHandleInfo(c, h, arg[1]);
				}
				++i;
			}
			h = h->next;
		}
		if(i >= set.MAX_MATCHES && set.MAX_MATCHES)
        	c->send("(more than ", itoa(set.MAX_MATCHES), " matches, list truncated)", NULL);

		if(!i)
        	    c->send("No matches found", NULL);
		else
        	    c->send("--- Found ", itoa(i), " match", i == 1 ? "" : "es", " for '", arg[1], "'", NULL);
    		return;
	}

	if(!strcmp(arg[0], ".chattr") && strlen(arg[2]))
	{
		n = userlist.changeFlags(arg[1], arg[2], arg[3], c);
		switch(n)
		{
			case -1: c->send("Invalid handle", NULL);
			return;

			case -2: c->send("Invalid channel", NULL);
			return;

			case -3: c->send(S_NOPERM, NULL);
			return;

			case -4: c->send("Invalid channel flags", NULL);
			return;

			case -5: c->send("Invalid flags", NULL);
			return;

			case -6: c->send("Invalid global flags", NULL);
			return;
			
			case -7: c->send("Flags conflict", NULL); 
			return;

			default: break;
		}
		userlist.flags2str(n, buf);
		net.sendCmd(c, "chattr ", arg[1], " ", arg[2], " (now\002:\002 ", buf, ") ", arg[3], NULL);
		if(strlen(arg[3]))
		{
			c->send("Changing \002", arg[3], "\002 flags for `\002", arg[1], "\002' to `\002", buf, "\002'", NULL);
			
			if(set.PRE_0211_FINAL_COMPAT)
			{
			    net.send(HAS_B, S_CHATTR, " ", arg[1], " - ", arg[3], NULL);
			    net.send(HAS_B, S_CHATTR, " ", arg[1], " ", buf, " ", arg[3], NULL);
			    ++userlist.SN;
			}
			else
			    net.send(HAS_B, S_CHATTR, " ", arg[1], " ", arg[2], " ", arg[3], NULL);
			//ME.recheckFlags(arg[3]);
		}
		else
		{
			if(!userlist.isBot(arg[1]))
			{
				c->send("Changing global flags for `\002", arg[1], "\002' to '\002", buf, "\002'", NULL);
				if(!(n & HAS_P))
				{
					inetconn *u;

					while((u = net.findConn(arg[1])))
						u->close("Lost partyline privileges");
				}
			}
			else c->send("Changing botnet flags for `\002", arg[1], "\002' to `\002", buf, "\002'", NULL);
			
			if(set.PRE_0211_FINAL_COMPAT)
			{
			    net.send(HAS_B, S_CHATTR, " ", arg[1], " -", NULL);
			    net.send(HAS_B, S_CHATTR, " ", arg[1], " ", buf, NULL);
			    ++userlist.SN;
			}
			else
			    net.send(HAS_B, S_CHATTR, " ", arg[1], " ", arg[2], NULL);


			//ME.recheckFlags();
		}
		++userlist.SN;
		ME.nextRecheck = NOW + SAVEDELAY;
		userlist.nextSave = NOW + SAVEDELAY;
		return;
	}
	//inetconn *c, char *name, char *key, int delay, char *flags, char *bot)
	if(!strcmp(arg[0], ".+chan") && strlen(arg[1]))
	{
		parse_owner_join(c, arg[1], arg[2], 0, "P");
		return;
	}
	if(!strcmp(arg[0], ".mjoin") && strlen(arg[1]))
	{
     	parse_owner_join(c, arg[1], arg[2], atoi(arg[3]), "*");
		return;
	}
	if(!strcmp(arg[0], ".tkmjoin") && strlen(arg[1]))
	{
     	parse_owner_join(c, arg[1], arg[2], atoi(arg[3]), "T");
		return;
	}
	if(!strcmp(arg[0], ".rjoin") && strlen(arg[2]))
	{
     	parse_owner_join(c, arg[2], arg[3], 0, "P", arg[1]);
		return;
	}
	if(!strcmp(arg[0], ".tkrjoin") && strlen(arg[2]))
	{
     	parse_owner_join(c, arg[2], arg[3], 0, "TP", arg[1]);
		return;
	}
	if(!strcmp(arg[0], ".sjoin") && strlen(arg[2]))
	{
		parse_owner_sjoin(c, arg[2], arg[3], 0, "P", arg[1]);
		return;
	}
	if(!strcmp(arg[0], ".tksjoin") && strlen(arg[2]))
	{
		parse_owner_sjoin(c, arg[2], arg[3], 0, "PT", arg[1]);
		return;
	}
	if(!strcmp(arg[0], ".rpart") && strlen(arg[2]))
	{
		parse_owner_rpart(c, arg[2], arg[1]);
		return;
	}
	if(!strcmp(arg[0], ".spart") && strlen(arg[2]))
	{
		parse_owner_spart(c, arg[2], arg[1]);
		return;
	}

	if(!strcmp(arg[0], ".-chan") || !strcmp(arg[0], ".mpart") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(userlist.removeChannel(arg[1], arg[2]))
		{
			net.sendCmd(c, "mpart ", arg[2], NULL);
			c->send("Removing `\002", arg[2], "\002' from channel list", NULL);
			net.send(HAS_B, S_RMCHAN, " ", arg[2], NULL);
			net.irc.send("PART ", arg[2], " :", (const char *) config.partreason, NULL);
			++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
		}
		else c->send("Invalid channel", NULL);
		return;
	}
	if(!strcmp(arg[0], ".+bot") && strlen(arg[2]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(!isRealStr(arg[1]))
		{
			c->send("Invalid handle", NULL);
			return;
		}
		if(strlen(arg[1]) > MAX_HANDLE_LEN)
		{
			c->send("Too long handle name", NULL);
			return;
		}
		if((!isValidIp(arg[2]) || !match("*.*.*.*", arg[2])) && strcmp(arg[2], "-"))
		{
			c->send("Invalid IPv4 address", NULL);
			return;
		}
		if((h = userlist.addHandle(arg[1], !strcmp(arg[2], "-") ? 0 : inet_addr(arg[2]), B_FLAGS, 0, 0, c->name)))
		{
			net.sendCmd(c, "+bot ", arg[1], " ", arg[2], NULL);
			c->send("Adding new bot `\002", arg[1], "\002'", NULL);
			net.send(HAS_B, S_ADDBOT, " ", arg[1], " ", h->creation->print(), " ", arg[2], NULL);
			//ME.recheckFlags();
			++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
		}
		else c->send("Handle exits", NULL);
		return;
	}
	if(!strcmp(arg[0], ".-bot") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if((n = userlist.removeHandle(arg[1])) == 1)
		{
			net.sendCmd(c, "-bot ", arg[1], NULL);
			c->send("Removing handle `\002", arg[1], "\002'", NULL);
        		net.send(HAS_B, S_RMUSER, " ", arg[1], NULL);
			//ME.recheckFlags();
			++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
		}
		else if (n == -1)
		{
			c->send("This handle is immortal", NULL);
			return;
		}
		else c->send("Invalid handle", NULL);
		return;
	}
	if(!strcmp(arg[0], ".channels") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		i = userlist.findChannel(arg[1]);
		if(i != -1)
		{
			net.sendCmd(c, "channels ", arg[1], NULL);

			if(userlist.chanlist[i].status & PRIVATE_CHAN)
				c->send("Channel type\002:\002 private", NULL);
			else
				c->send("Channel type\002:\002 massjoinable", NULL);

			h = userlist.first;
			int total = 0;
			int down = 0;
			inetconn *bot;
			a = b = NULL;

			while(h)
			{
				if(userlist.isBot(h) && userlist.isRjoined(i, h))
				{
					++total;
					if((!(bot = net.findConn(h)) || !bot->name || !*bot->name) && h != userlist.first->next)
					{
						++down;
						b = push(b, b ? (char *) "\002,\002 " : (char *) " ", h->name, NULL);
					}
					else
						a = push(a, a ? (char *) "\002,\002 " : (char *) " ", h->name, NULL);

					/*
					if(h == userlist.first->next)
					{
						if(strlen(ME.nick))
							a = push(a, a ? (char *) "\002,\002 " : (char *) " ", h->name, NULL);
						else
						{
							++down;
							b = push(b, b ? (char *) "\002,\002 " : (char *) " ", h->name, NULL);
						}
					}
					*/

				}
				h = h->next;
			}

			if(a)
			{
				c->send("Supposingly joined(\002", itoa(total-down), "\002)\002:\002", a, NULL);
				free(a);
			}
			if(b)
			{
				c->send("Supposingly not there(\002", itoa(down), "\002)\002:\002", b, NULL);
				free(b);
			}
		}
		else c->send("Invalid channel", NULL);
		return;
	}

	if(!strcmp(arg[0], ".channels"))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		net.sendCmd(c, "channels", NULL);
		a = NULL;
		int total, down;

		for(n=i=0; i<MAX_CHANNELS; i++)
		{
			down = total = 0;
			if(userlist.chanlist[i].name)
			{
				if(userlist.chanlist[i].status & PRIVATE_CHAN)
				{
					h = userlist.first;
					total = down = 0;
					inetconn *bot;

					while(h)
					{
						if(userlist.isBot(h) && userlist.isRjoined(i, h))
						{
							++total;
							if((!(bot = net.findConn(h)) || !bot->name) && h != userlist.first->next)
								++down;
						}
						h = h->next;
					}


					if(userlist.chanlist[i].pass && userlist.chanlist[i].pass)
						snprintf(buf, MAX_LEN, "%s (n: %d/%d, key: %s)",
								 (const char *) userlist.chanlist[i].name, total-down, total, (const char *) userlist.chanlist[i].pass);
					else
						snprintf(buf, MAX_LEN, "%s (n: %d/%d)",
								 (const char *) userlist.chanlist[i].name, total-down, total);
				}
				else
				{
					if(userlist.chanlist[i].pass && userlist.chanlist[i].pass)
						snprintf(buf, MAX_LEN, "%s (key: %s)",
								 (const char *) userlist.chanlist[i].name, (const char *) userlist.chanlist[i].pass);
					else
						snprintf(buf, MAX_LEN, "%s",
								 (const char *) userlist.chanlist[i].name);
				}

				a = push(a, a ? (char *) "\002,\002 " : (char *) " ", buf, NULL);
				++n;
			}
		}
		if(n)
		{
			c->send("Channels\002:\002", a, NULL);
			free(a);
		}
		else c->send("No channels found", NULL);
		return;
	}
	if(!strcmp(arg[0], ".save"))
	{
		net.sendCmd(c, "save", NULL);
		c->send("Saving userlist", NULL);
		userlist.save(config.userlist_file);
		net.send(HAS_B, S_ULSAVE, NULL);
		ME.nextRecheck = NOW + 5;
		return;
	}
	if(!strcmp(arg[0], ".export") && strlen(arg[1]))
	{
		if(c->checkFlag(HAS_X))
		{
			if(strlen(arg[2]))
			{
		 		n = userlist.save(arg[1], 1, arg[2]);
				net.sendCmd(c, "export ", arg[1], " [something]", NULL);
			}
			else
			{
		 		n = userlist.save(arg[1], 0);
				net.sendCmd(c, "export ", arg[1], NULL);
			}
			if(n) c->send("[+] Exported", NULL);
		}
		else c->send(S_NOPERM, NULL);
		return;
	}
	if(!strcmp(arg[0], ".import") && strlen(arg[1]))
	{
		if(c->checkFlag(HAS_X))
		{
			userlist.reset();
			userlist.addHandle("idiots", 0, 0, 0, 0, config.handle);
			userlist.addHandle(config.handle, 0, B_FLAGS | HAS_H, 0, 0, config.handle);
			userlist.first->flags[MAX_CHANNELS] = HAS_D;

			if(strlen(arg[2]))
			{
		 		net.sendCmd(c, "import ", arg[1], " [something]", NULL);
				n = userlist.load(arg[1], 1, arg[2]);
			}
			else
			{
		 		net.sendCmd(c, "import ", arg[1], NULL);
				n = userlist.load(arg[1], 0);
			}

			if(n == 1)
			{
				net.irc.send("QUIT :Userfile imported", NULL);
				userlist.save(config.userlist_file);
			}
			else if(n == -1)
			{
				net.irc.send("QUIT :Broken userlist", NULL);
				exit(1);
			}
			else if(n == 0)
			{
				net.irc.send("QUIT :Cannot import: ", strerror(errno), NULL);
				exit(1);
			}
		}
		return;
	}

	if(!strcmp(arg[0], ".bots") && !strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		net.sendCmd(c, "bots", NULL);
		if(userlist.bots)
		{
        		h = userlist.first->next;
        		a = NULL;

			i = 0;

			while(h)
			{
				if(userlist.isBot(h))
				{
					a = push(a, h->name, " ", NULL);
					++i;
				}
				h = h->next;
			}
		   	c->send("Bots(", itoa(i), "): ", a, NULL);
			if(a) free(a);
		}
		else c->send("No bot handles present", NULL);
		return;
	}
	if(!strcmp(arg[0], ".owners"))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		a = NULL;
		net.sendCmd(c, "owners", NULL);
		for(i=0; i<net.max_conns; ++i)
		{
			if(net.conn[i].isRegOwner())
				a = push(a, net.conn[i].name, " ", NULL);
		}
		c->send("Owners on-line(\002", itoa(net.owners()), "\002)\002:\002 ", a, NULL);
		if(a) free(a);
		return;
	}
	if(!strcmp(arg[0], ".upbots") || !strcmp(arg[0], ".up"))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		net.sendCmd(c, "up", NULL);
		if(userlist.bots)
		{
			h = userlist.first->next->next;
        		a = NULL;
        		i = 0;
        		while(h)
        		{
				if(userlist.isBot(h) && net.findConn(h) && net.findConn(h)->isRegBot())
				{
					a = push(a, a ? (char *) "\002,\002 " : (char *) " ", h->name, NULL);
					++i;
				}
				h =h->next;
			}
			if(i)
			{
				sprintf(buf, "%d", i);
				c->send("Bots on-line(\002", buf, "\002)\002:\002", a, NULL);
			}
			else c->send("All bots are down", NULL);
        	if(a) free(a);
		}
		else c->send("No bot handles present", NULL);
		return;
	}
	if(!strcmp(arg[0], ".downbots") || !strcmp(arg[0], ".down"))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		net.sendCmd(c, "down", NULL);
		if(userlist.bots)
		{
			h = userlist.first->next->next;
        		a = NULL;
        		i = 0;

        		while(h)
        		{
				if(userlist.isBot(h) && (!net.findConn(h) || !net.findConn(h)->isRegBot()))
				{
					a = push(a, a ? (char *) "\002,\002 " : (char *) " " , h->name, NULL);
					++i;
				}
				h = h->next;
			}
			if(i)
			{
				sprintf(buf, "%d", i);
				c->send("Bots off-line(\002", buf, "\002)\002:\002", a, NULL);
			}
			else c->send("All bots are up", NULL);
        		if(a) free(a);
		}
		else c->send("No bot handles present", NULL);
		return;
	}
	if(!strcmp(arg[0], ".set"))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}

		if(set.parseUser(c->name, arg[1], arg[2], "set"))
		{
			net.send(HAS_B, S_SET, " ", arg[1], " ", arg[2], NULL);
			++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
		}
		return;
	}

	if(!strcmp(arg[0], ".dset"))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		a = srewind(data, 2);
		if(userlist.dset->parseUser(c->name, arg[1], a ? a : "", "dset"))
			userlist.nextSave = NOW + SAVEDELAY;
		return;
	}

	if((!strcmp(arg[0], ".chset") || !strcmp(arg[0], ".chanset") || !strcmp(arg[0], ".cset")) && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		i = userlist.findChannel(arg[1]);
		if(i != -1)
		{
			a = srewind(data, 3);
			if(userlist.chanlist[i].chset->parseUser(c->name, arg[2], a ? a : "", arg[1], "chset "))
			{
				net.send(HAS_B, S_CHSET, " ", arg[1], " ", arg[2],  " ", userlist.chanlist[i].chset->getValue(arg[2]), NULL);
				++userlist.SN;
				userlist.nextSave = NOW + SAVEDELAY;
			}
		}
		else c->send("Invalid channel", NULL);
		return;
	}

	if(!strcmp(arg[0], ".gset") && strlen(arg[2]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		a = srewind(data, 2);
		i = -1;
		if(userlist.globalChset(c, arg[1], a, &i))
		{
			// COMPATYBYLITY REASONS 
			net.send(HAS_B, S_GCHSET, " ", arg[1], " ", userlist.chanlist[i].chset->getValue(arg[1]), NULL);
			++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
		}
		return;
	}

	if(!strcmp(arg[0], ".mcycle") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(userlist.findChannel(arg[1]) != -1)
		{
			if(ME.findChannel(arg[1]))
			{
				net.irc.send("PART ", arg[1], " :", (const char *) config.cyclereason, NULL);
				ME.rejoin(arg[1], set.CYCLE_DELAY);
			}
			net.send(HAS_B, S_CYCLE, " ", arg[1], NULL);
			net.sendCmd(c, "mcycle ", arg[1], NULL);
			c->send("[*] Doing mass cycle on `\002", arg[1], "\002'", NULL);
		}
		else c->send("Invalid channel", NULL);
		return;
	}
	if(!strcmp(arg[0], ".rcycle") && strlen(arg[2]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(userlist.findChannel(arg[2]) != -1)
		{
			net.sendCmd(c, "rcycle ", arg[1], " ", arg[2], NULL);
			if(!strcmp(arg[1], config.handle))
			{
				net.irc.send("PART ", arg[2], " :", (const char *) config.cyclereason, NULL);
				ME.rejoin(arg[1], set.CYCLE_DELAY);
				net.send(HAS_N, "[*] Doing cycle on `\002", arg[2], "\002'", NULL);
			}
			else
			{
				inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
				if(bot && bot->isRegBot()) bot->send(S_CYCLE, " ", arg[2], " ", S_FOO, NULL);
				else c->send("Invalid bot", NULL);
			}
		}
		else c->send("Invalid channel", NULL);
		return;
	}
	if(!strcmp(arg[0], ".rjump")&& strlen(arg[2]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(!strcmp(arg[1], config.handle))
		{
			net.sendCmd(c, "rjump ", arg[1], " ", arg[2], " ", arg[3], NULL);
			ME.jump(arg[2], arg[3], c->name);
			return;
		}

		inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
		if(bot && bot->isRegBot())
		{
			net.sendCmd(c, "rjump ", arg[1], " ", arg[2], " ", arg[3], NULL);
			bot->send(S_JUMP, " ", c->name, " ", arg[2], " ", arg[3], NULL);
		}
		else c->send("Invalid bot", NULL);

		return;
	}
	if(!strcmp(arg[0], ".rjumps5") && strlen(arg[4]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(!strcmp(arg[1], config.handle))
		{
			net.sendCmd(c, "rjumps5 ", arg[1], " ", arg[2], " ", arg[3], " ", arg[4], " ", arg[5], NULL);
			ME.jumps5(arg[2], atoi(arg[3]), arg[4], atoi(arg[5]), c->name);
			return;
		}

		inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
		if(bot && bot->isRegBot())
		{
			net.sendCmd(c, "rjumps5 ", arg[1], " ", arg[2], " ", arg[3], " ", arg[4], " ", arg[5], NULL);
			bot->send(S_JUMPS5, " ", c->name, " ", arg[2], " ", arg[3], " ", arg[4], " ", arg[5], NULL);
		}
		else c->send("Invalid bot", NULL);

		return;
	}
	if(!strcmp(arg[0], ".rjump6")&& strlen(arg[2]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(!strcmp(arg[1], config.handle))
		{
#ifdef HAVE_IPV6
			net.sendCmd(c, "rjump6 ", arg[1], " ", arg[2], " ", arg[3], NULL);
			ME.jump(arg[2], arg[3], c->name, AF_INET6);
#else
			c->send("No ipv6 support", NULL);
#endif
			return;
		}

		inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
		if(bot && bot->isRegBot())
		{
			net.sendCmd(c, "rjump6 ", arg[1], " ", arg[2], " ", arg[3], NULL);
			bot->send(S_JUMP6, " ", c->name, " ", arg[2], " ", arg[3], NULL);
		}
		else c->send("Invalid bot", NULL);

		return;
	}
	if(!strcmp(arg[0], ".chnick") && strlen(arg[2]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(!strcmp(arg[1], config.handle))
		{
			net.sendCmd(c, "chnick ", arg[1], " ", arg[2], NULL);
			net.irc.send("NICK ", arg[2], NULL);
			ME.nextNickCheck = NOW + set.KEEP_NICK_CHECK_DELAY;
		}
		else
		{
			inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
			if(bot && bot->isRegBot())
			{
				net.sendCmd(c, "chnick ", arg[1], " ", arg[2], NULL);
				bot->send(S_NICK, " ", arg[2], NULL);
			}
			else c->send("Invalid bot", NULL);
		}
		return;
	}
	if(!strcmp(arg[0], ".chpass") && strlen(arg[2]))
	{
        if((n = userlist.hasWriteAccess(c, arg[1])) == 1)
        {
			if(strlen(arg[2]) < 8)
			{
				c->send("Password must be at least 8 charactes long", NULL);
				return;
			}
			if(!(h = userlist.changePass(arg[1], arg[2]))) c->send("Invalid handle", NULL);
			else
			{
				net.sendCmd(c, "chpass ", arg[1], " [something]", NULL);
				c->send("Changing password for `\002", arg[1], "\002'", NULL);
				net.send(HAS_B, S_PASSWD, " ", arg[1], " ", quoteHexStr(h->pass, buf), NULL);
				++userlist.SN;
				userlist.nextSave = NOW + SAVEDELAY;
            }
    	}
		else if(n == -1) c->send("Invalid handle", NULL);
		else c->send(S_NOPERM, NULL);
		return;
	}
	if(!strcmp(arg[0], ".chaddr") && strlen(arg[2]))
	{
    		if(userlist.hasWriteAccess(c, arg[1]) == 1)
    		{
			if(!(h = userlist.changeIp(arg[1], arg[2]))) c->send("Invalid handle or ip", NULL);
			else
			{
				net.sendCmd(c, "chaddr ", arg[1], " [something]", NULL);
				c->send("Changing ip address of `\002", arg[1], "\002'", NULL);
				net.send(HAS_B, S_ADDR, " ", arg[1], " ", arg[2], NULL);
				++userlist.SN;
				userlist.nextSave = NOW + SAVEDELAY;
        		}
    		}
		else c->send(S_NOPERM, NULL);
		return;
	}
	if(!strcmp(arg[0], ".status") || !strcmp(arg[0], ".stat"))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		net.sendCmd(c, "status ", arg[1], " ", arg[2], NULL);

		if(strlen(arg[1]))
		{
			inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
			if(bot)
				bot->send(S_STATUS, " ", c->name, NULL);
			else
				c->send("Invalid bot", NULL);
		}
		else
			ME.sendStatus(c->name);


		return;
	}
	if(!strcmp(arg[0], ".mk") && strlen(arg[2]))
	{
		if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}

		if(userlist.findChannel(arg[2]) == -1)
		{
			c->send("Invalid channel", NULL);
			return;
		}

		int lock = !strcmp(arg[3], "close") || !strcmp(arg[3], "lock");

		chan *ch = ME.findChannel(arg[2]);
		if(!strcmp(arg[1], "a") || !strcmp(arg[1], "all"))
		{
			net.sendCmd(c, "mk ", arg[1], " ", arg[2], " ", arg[3], NULL);

			if(ch)
				ch->massKick(MK_ALL, lock);

			net.propagate(NULL, S_MKA, " ", arg[2], " ", arg[3], NULL);
			c->send("Doing mass kick all on ", arg[2], NULL);
		}
		else if(!strcmp(arg[1], "o") || !strcmp(arg[1], "ops"))
		{
			net.sendCmd(c, "mk ", arg[1], " ", arg[2], " ", arg[3], NULL);

			if(ch)
				ch->massKick(MK_OPS, lock);

			net.propagate(NULL, S_MKO, " ", arg[2], NULL);
			c->send("Doing mass kick ops on ", arg[2], NULL);
		}
		else if(!strcmp(arg[1], "n") || !strcmp(arg[1], "nonops") || !strcmp(arg[1], "lames"))
		{
			net.sendCmd(c, "mk ", arg[1], " ", arg[2], " ", arg[3], NULL);

			if(ch)
				ch->massKick(MK_NONOPS, lock);

			net.propagate(NULL, S_MKN, " ", arg[2], NULL);
			c->send("Doing mass kick nonops on ", arg[2], NULL);
		}
		else c->send("No such user class", NULL);
		return;
	}

	if(!strcmp(arg[0], ".list") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(strchr("apsvdcUui", arg[1][0]) && !arg[1][1])
		{
			net.sendCmd(c, "list ", arg[1], " ", arg[2], NULL);

			if(strlen(arg[2]))
			{
				if(!strcmp(arg[2], config.handle))
					listcmd(arg[1][0], c->name);
				else
				{
					inetconn *bot = net.findConn(userlist.findHandle(arg[2]));
					if(bot)
					{
						if(arg[1][0]=='p')
							bot->send(S_LIST, " ", arg[1], " ", c->name, " ", itoa(NOW), " ", itoa(nanotime()), NULL);
						else
							bot->send(S_LIST, " ", arg[1], " ", c->name, NULL);
					}
					else
						c->send("Invalid bot", NULL);
				}
			}
			else if(listcmd(arg[1][0], c->name) == 1)
				net.propagate(NULL, S_LIST, " ", arg[1], " ", c->name, NULL);
		}
		else
			c->send("Invalid option", NULL);

		return;
	}
	if(!strcmp(arg[0], ".verify"))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		
		int _test;
		if(strlen(arg[1]))
		{
		    if(strstr(arg[1], "-a"))
			_test = 0;
		    else if(strstr(arg[1], "-p"))
			_test = 1;
		    else if(strstr(arg[1], "-h"))
			_test = 2;
		    else if(strstr(arg[1], "-c"))
			_test = 3;
		    else
			_test = -1;
		}
		else
		    _test = 0;
		
		if(_test == -1)
		{
		    c->send("Usage: .verify [-a|-p|-h|-c]", NULL);
		    return;
		}
		
		net.sendCmd(c, "verify ", arg[1], NULL);
		/* verify passwds */
		a = NULL;
		char *_a = NULL, *__a = NULL;
		int _i = 0, __i = 0;
		int j, _j;
		h = userlist.first->next;
		i = 0;

		while(h)
		{
			if(isNullString((char *) h->pass, 16) && (_test == 0 || _test == 1))
			{
				a = push(a, h->name, " ", NULL);
				++i;
			}
			if((!h->host[0] || !*h->host[0]) && (_test == 0 || _test == 2))
			{
			    _a = push(_a, h->name, " ", NULL);
			    _i++;
			}
			
			if(!h->flags[GLOBAL] && (_test == 0 || _test == 3))
			{
			    for(j = 0, _j = -1; _j == -1 && j < MAX_CHANNELS; j++)
				if(h->flags[j] && userlist.chanlist[j].name)
				    _j = j;
			    
			    if(_j == -1)
			    {
				__a = push(__a, h->name, " ", NULL);
				__i++;
			    }
			}
			h = h->next;
		}
		if(i)
		{
			sprintf(buf, "%d", i);
			c->send("Found ", buf, " handle", i == 1 ? "" : "s", " with no password set: ", a, NULL);
			free(a);
		}

		if(_i)
		{
			sprintf(buf, "%d", _i);
			c->send("Found ", buf, " handle", _i == 1 ? "" : "s", " with no hosts set: ", _a, NULL);
			free(_a);
		}
		if(__i)
		{
			sprintf(buf, "%d", __i);
			c->send("Found ", buf, " handle", __i == 1 ? "" : "s", " with no flags set: ", __a, NULL);
			free(__a);
		}
    		return;
	}

	if(!strcmp(arg[0], ".bottree") || !strcmp(arg[0], ".bt"))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		net.sendCmd(c, "bt", NULL);
		userlist.sendBotTree(c);
		return;
	}
	if(!strcmp(arg[0], ".unlink") && strlen(arg[1]))
	{
        if(!c->checkFlag(HAS_S))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		h = userlist.findHandle(arg[1]);
    		if(h)
    		{
			if(userlist.isSlave(h) || userlist.isLeaf(h))
			{
				inetconn *bot = net.findConn(h);
				if(bot)
				{
					net.sendCmd(c, "unlink ", arg[1], NULL);
					if(bot->status & STATUS_REDIR)
					{
						inetconn *slave = net.findRedirConn(bot);
						if(slave) slave->send(S_UNLINK, " ", arg[1], NULL);
					}
					else bot->close("Forced unlink");
				}
				else c->send("Bot is down", NULL);
    			}
      			else c->send("Invalid bot", NULL);
			return;
		}
		else c->send("No such handle", NULL);
    		return;
	}
	if(!strcmp(arg[0], ".boot") && strlen(arg[2]))
	{
		h = userlist.findHandle(arg[1]);
		if(h)
		{
			inetconn *o = net.findConn(h);
			if(o && o->isReg() && o->checkFlag(HAS_P))
			{
				if((o->checkFlag(HAS_X) && !c->checkFlag(HAS_X)) ||
				   (o->checkFlag(HAS_S) && !c->checkFlag(HAS_S)) ||
				   (o->checkFlag(HAS_N) && !c->checkFlag(HAS_N)))
				{
					c->send(S_NOPERM, NULL);
					return;
				}

				net.sendCmd(c, "boot ", arg[1], NULL);
				a = srewind(data, 2);
				if(a)
				{
					snprintf(buf, MAX_LEN, "(Booted by %s) %s", c->name, a);
					o->close(buf);
				}
				else o->close("Booted");
			}
			else c->send("Invalid owner", NULL);
		}
		else c->send("Invalid owner", NULL);
		return;
	}
	if(!strcmp(arg[0], ".rdie") && strlen(arg[1]))
	{
		net.sendCmd(c, data+1, NULL);
		if(!strcmp(arg[1], config.handle))
		{
			if(!c->checkFlag(HAS_X))
			{
				c->close("What? You need `\002.help\002'?");
				return;
			}
			userlist.autoSave(1);
			net.send(HAS_N, "[!] ", DIE_REASON, NULL);
			net.irc.send("QUIT :", c->handle->name, " ", DIE_REASON2, NULL);
			safeExit();
		}
		inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
		if(bot)
		{
			if(!c->checkFlag(HAS_X))
			{
				c->close("What? You need `\002.help\002'?");
				return;
			}
			bot->send(S_RDIE, " ", c->handle->name, NULL);
		}
		return;
	}

/*
	if(!strcmp(arg[0], ".reset") && !strcmp(arg[1], "idle"))
	{
		if(!c->checkFlag(HAS_X))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		net.sendCmd(c, "reset idle", NULL);
		net.propagate(NULL, S_RESET, " I", NULL);
		antiidle.reset();
		c->send("Reseting antiidle timers", NULL);
		return;
	}
*/

	if(!strcmp(arg[0], ".abuse"))
	{
		net.sendCmd(c, "abuse", NULL);
		if(c->checkFlag(HAS_X))
		{
			net.send(HAS_N, "\002* ", (const char *) config.handle, "\002 slaps ", c->name, " with Unix User Manual", NULL);
			return;
		}
		c->close("You have just been abused");
		return;
	}
	if(!strcmp(arg[0], ".update") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_X))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		
		if(!strcmp(arg[1], "*")) // .mupdate
		{
		    net.sendCmd(c, "update * ", arg[2], NULL);
		    // hub shouldnt be updated with the rest of bnet 
		    //psotget.forkAndGo(arg[2]);
		    
                    for(int i=0; i<net.max_conns; ++i)
                    {
                        if(net.conn[i].fd > 0 && net.conn[i].isRegBot())
                        {
                            net.conn[i].send(S_PSOTUPDATE, " ", arg[2], NULL);
                        }
                    }		    
		    
		    return;
		}

		if(!strcmp(arg[1], config.handle))
		{
			net.sendCmd(c, "update ", arg[1], " ", arg[2], NULL);
			psotget.forkAndGo(arg[2]);
			return;
		}

		inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
		if(bot)
		{
			net.sendCmd(c, "update ", arg[1], " ", arg[2], NULL);
			bot->send(S_PSOTUPDATE, " ", arg[2], NULL);
		}
		else c->send("Invalid bot", NULL);
		return;
	}
	if(!strcmp(arg[0], ".stopupdate") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_X))
		{
			c->send(S_NOPERM, NULL);
			return;
		}

		if(!strcmp(arg[1], "*")) // .mupdate
		{
		    net.sendCmd(c, "stopupdate *", NULL);
		    psotget.end();
		    
                    for(int i=0; i<net.max_conns; ++i)
                    {
                        if(net.conn[i].fd > 0 && net.conn[i].isRegBot())
                        {
                            net.conn[i].send(S_STOPUPDATE, " ", arg[2], NULL);
                        }
                    }		    
		    
		    return;
		}


		if(!strcmp(arg[1], config.handle))
		{
			net.sendCmd(c, "stopupdate ", arg[1], NULL);
			psotget.end();
			return;
		}

		inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
		if(bot)
		{
			net.sendCmd(c, "stopupdate ", arg[1], NULL);
			bot->send(S_STOPUPDATE, NULL);
		}
		else c->send("Invalid bot", NULL);
		return;
	}

	if(!strcmp(arg[0], ".restart") && strlen(arg[1]))
	{
		if(!c->checkFlag(HAS_X))
		{
			c->send(S_NOPERM, NULL);
			return;
		}

		if(!strcmp(arg[1], config.handle))
		{
			net.sendCmd(c, "restart ", arg[1], NULL);
			userlist.autoSave(1);
			ME.restart();
		}
		else
		{
			inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
			if(bot)
			{
				net.sendCmd(c, "restart ", arg[1], NULL);
				bot->send(S_RESTART, " ", c->name, NULL);
			}
			else c->send("Invalid bot", NULL);
		}
		return;
	}
	if((!strcmp(arg[0], ".names") || !strcmp(arg[0], ".n") && strlen(arg[2])))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(!strcmp(arg[1], config.handle))
		{
			chan *ch = ME.findChannel(arg[2]);
			if(ch)
			{
				net.sendCmd(c, "names ", arg[1], " ", arg[2], NULL);
				ch->names(c->name);
			}
			else c->send("Invalid channel", NULL);
		}
		else
		{
			inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
			if(bot)
			{
				net.sendCmd(c, "names ", arg[1], " ", arg[2], NULL);
				bot->send(S_NAMES, " ", c->name, " ", arg[2], NULL);
			}
			else c->send("Invalid bot", NULL);
		}
		return;
	}
	if((!strcmp(arg[0], ".cwho") || !strcmp(arg[0], ".chusers") && strlen(arg[2])))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if(!strcmp(arg[1], config.handle))
		{
			chan *ch = ME.findChannel(arg[2]);
			if(ch)
			{
				net.sendCmd(c, "cwho ", arg[1], " ", arg[2], " ", arg[3], NULL);
				ch->cwho(c->name, arg[3]);
			}
			else c->send("Invalid channel", NULL);
		}
		else
		{
			inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
			if(bot)
			{
				net.sendCmd(c, "cwho ", arg[1], " ", arg[2], " ", arg[3], NULL);
				bot->send(S_CWHO, " ", c->name, " ", arg[2], " ", arg[3], NULL);
			}
			else c->send("Invalid bot", NULL);
		}
		return;
	}

	if(!strcmp(arg[0], ".chhandle") && strlen(arg[2]))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}
		if((n = userlist.hasWriteAccess(c, arg[1])) == 1)
		{
			HANDLE *h = userlist.findHandle(arg[1]);
			if(h == userlist.first)
			{
				c->send("Let those idiots alone ;-)", NULL);
				return;
			}
			if(userlist.isBot(h))
			{
				c->send("Cannot change bot's handle name", NULL);
				return;
			}
			if(userlist.findHandle(arg[2]))
			{
				c->send("Destnation handle exists", NULL);
				return;
			}
			if(!isRealStr(arg[2]))
			{
				c->send("Invalid destination handle", NULL);
				return;
			}

			net.sendCmd(c, "chhandle ", arg[1], " ", arg[2], NULL);

			int changed = 0;
			for(int i=0; i<net.max_conns; ++i)
			{
				if(net.conn[i].name && !strcmp(net.conn[i].name, arg[1]))
				{
					free(net.conn[i].name);
					mem_strcpy(net.conn[i].name, arg[2]);
					changed = 1;
				}
			}

			if(changed)
				net.send(HAS_N, "[*] ", arg[1], " is now known as ", arg[2], NULL);

			free(h->name);
			mem_strcpy(h->name, arg[2]);
			++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
			net.send(HAS_B, S_CHHANDLE, " ", arg[1], " ", arg[2], NULL);
		}
		else c->send(S_NOPERM, NULL);
		return;
	}
	if(!strcmp(arg[0], ".+info") && strlen(arg[3]))
	{
		HANDLE *h = userlist.findHandle(arg[1]);
		if(h)
		{
			if(userlist.hasWriteAccess(c, arg[1]) == 1)
			{
				if(h->info && h->info->data.entries() > 5)
				{
					c->send("Too many entries", NULL);
					return;
				}

				char *a;
				a = srewind(data, 3);
				if(!h->info)
					h->info = new comment;
				if(h->info->add(arg[2], a))
				{
					net.sendCmd(c, data+1, NULL);
					c->send("Info updated", NULL);
					userlist.nextSave = NOW + SAVEDELAY;
				}
				else c->send("Invalid data format", NULL);

			}
			else c->send(S_NOPERM, NULL);
		}
		else c->send("Invalid handle", NULL);
		return;
	}

	if(!strcmp(arg[0], ".info") && strlen(arg[1]))
	{
		HANDLE *h = userlist.findHandle(arg[1]);
		if(h)
		{
			if(!userlist.hasReadAccess(c, h))
			{
				c->send(S_NOPERM, NULL);
				return;
			}
			if(!h->info || !h->info->data.entries())
			{
				c->send("No info available", NULL);
				return;
			}

			ptrlist<comment::entry>::iterator e = h->info->data.begin();
			a = NULL;

			while(e)
			{
				a = push(a, a ? (char *) "\002,\002 " : (char *) " ", e->key, "\002:\002 ",
					 e->value, NULL);
				e++;
			}
			net.sendCmd(c, "info ", arg[1], NULL);
			c->send(h->name, "'s info\002:", a, NULL);
			free(a);
		}
		else c->send("Invalid handle", NULL);
		return;
	}
	if(!strcmp(arg[0], ".-info") && strlen(arg[2]))
	{
		HANDLE *h = userlist.findHandle(arg[1]);
		if(h)
		{
			if(userlist.hasWriteAccess(c, arg[1]) == 1)
			{
				if(!h->info || !h->info->del(arg[2]))
					c->send("Invalid key", NULL);
				else
				{
					net.sendCmd(c, "-info ", arg[1], " ", arg[2], NULL);
					c->send("Info removed", NULL);
					userlist.nextSave = NOW + SAVEDELAY;
				}
			}
			else c->send(S_NOPERM, NULL);
		}
		else c->send("Invalid handle", NULL);
		return;
	}

	if(!strcmp(arg[0], ".whom") || !strcmp(arg[0], ".who"))
	{
		if(!c->checkFlag(HAS_N))
		{
			c->send(S_NOPERM, NULL);
			return;
		}

		int i, f;

		if(!strcmp(arg[0], ".whom"))
			net.sendCmd(c, "whom", NULL);
		else
			net.sendCmd(c, "who", NULL);

		for(i=n=0; i<net.max_conns; ++i)
		{
			if(net.conn[i].fd && net.conn[i].isReg() && net.conn[i].checkFlag(HAS_P) && !net.conn[i].isBot())
			{
				f = net.conn[i].handle->flags[GLOBAL];
				snprintf(buf, MAX_LEN, "[#%02d] ", n+1);
				c->send(buf, net.conn[i].handle->name, " (",
						f & HAS_X ? "perm" : f & HAS_S ? "super owner" : f & HAS_N ? "owner" : "channel owner",
						"\002,\002 ",
						net.conn[i].getPeerIpName(), " port ", net.conn[i].getPeerPortName(), "\002,\002 ",
						net.conn[i].status & STATUS_TELNET ?
							(net.conn[i].status & STATUS_SSL ? "via SSL enabled telnet)" : "via telnet)") : "via dcc chat)",
						NULL);
				++n;
			}
		}
		if(strcmp(arg[0], ".who")) return;
	}
	if(!strcmp(arg[0], ".whob") || !strcmp(arg[0], ".who"))
	{
		int flags[2];

		int i;

		if(!c->checkFlag(HAS_S))
			return;

		if(!strcmp(arg[0], ".who"))
		{
			flags[1] = flags[0] = STATUS_PARTY;
			net.sendCmd(c, "who", NULL);
			
		}
		else
		{
			flags[0] = (STATUS_BOT | STATUS_REDIR);
			flags[1] = STATUS_BOT;
			net.sendCmd(c, "whob", NULL);
		}

		n = 0;
		
		for(i=0; i<net.max_conns; ++i)
		{
			if(net.conn[i].fd >0 && net.conn[i].isRegBot() &&
				(net.conn[i].status & flags[0]) == flags[1])
			{
				snprintf(buf, MAX_LEN, "[#%02d] ", n+1);
				c->send(buf, net.conn[i].handle->name, " (slave\002,\002 ",
						net.conn[i].getPeerIpName(), " port ", net.conn[i].getPeerPortName(),
						")", NULL);
				++n;
			}
		}
		return;
	}
	if(!strcmp(arg[0], ".echo") && *arg[1])
	{
		if(!strcmp(arg[1], "on"))
		{
			net.sendCmd(c, "echo on", NULL);
			c->send("Your echo is now on", NULL);
			c->status &= ~STATUS_NOECHO;
		}
		else if(!strcmp(arg[1], "off"))
		{
			net.sendCmd(c, "echo off", NULL);
			c->send("Your echo is now off", NULL);
			c->status |= STATUS_NOECHO;
		}
		else
			c->send("Invalid argument", NULL);
		return;
	}
	int stick;
	if(((stick = !strcmp(arg[0], ".+stick")) || !strcmp(arg[0], ".+shit") || (stick=!strcmp(arg[0], ".+invite")) || (stick=!strcmp(arg[0], ".+exempt")) || (stick=!strcmp(arg[0], ".+reop"))) && strlen(arg[1]))
	{
		char *channel = NULL, *reason = NULL;
		char *expires = NULL;
		char *mask = NULL;
		int expTime = 0;
		int i;
		int chanNum = GLOBAL;
		int type;
		char *botnet_cmd;
		protmodelist *shit;
		protmodelist::entry *s;
		chan *ch;

		if(!strcmp(arg[0], ".+stick"))
		{
			type = BAN;
			botnet_cmd = S_ADDSTICK;
		}

		if(!strcmp(arg[0], ".+shit"))
		{
			type = BAN;
			botnet_cmd = S_ADDSHIT;
		}

		else if(!strcmp(arg[0], ".+invite"))
		{
			type = INVITE;
			botnet_cmd = S_ADDINVITE;
		}

		else if(!strcmp(arg[0], ".+exempt"))
		{
			type = EXEMPT;
			botnet_cmd = S_ADDEXEMPT;
		}

		else if(!strcmp(arg[0], ".+reop"))
		{
			type = REOP;
			botnet_cmd = S_ADDREOP;
		}

		shit = userlist.protlist[type];

		for(i=1; i<4 && *arg[i]; ++i)
		{
			//channel
			if(chan::valid(arg[i]))
			{
				if(channel)
				{
					c->send("Too many channels given", NULL);
					return;
				}
				else if(arg[i][1] && (chanNum = userlist.findChannel(arg[i])) != -1)
				{
					channel = arg[i];
					shit = userlist.chanlist[chanNum].protlist[type];
				}
				else
				{
					c->send("Invalid channel", NULL);
					return;
				}
			}

			//expiration time
			else if(*arg[i] == '%')
			{
				if(expires)
				{
					c->send("Too many expiration dates were given", NULL);
					return;
				}
				else if(arg[i][1] && units2int(arg[i]+1, ut_time, expTime) == 1 && expTime > 0)
				{
					expires = arg[i]+1;
				}
				else
				{
					c->send("Invalid expiration time", NULL);
					return;
				}
			}
			//mask
			else
			{
				if(extendhost(arg[i], buf, MAX_LEN))
					mask = buf;
				break;
			}
		}

		if(!mask || !strcmp(mask, "*!*@*"))
		{
			c->send("Invalid mask", NULL);
			return;
		}
		
		char *ex;
		if((ex = strchr(mask, '!')) - mask > 15)
		{
			c->send("Invalid mask: more than 15 chars before `!'", NULL);
			return;
		}
		
		if(strchr(mask, '@') - ex > 11)
		{
			c->send("Invalid mask: more than 10 chars between `!' and `@'", NULL);
			return;
		}
		
		/* look for conflicts */
		if((s = shit->conflicts(mask)))
		{
			c->send("Mask conflicts with `\002", s->mask, "\002'", NULL);
			return;
		}

		if(!c->checkFlag(HAS_N) && !c->checkFlag(HAS_N, chanNum))
		{
			c->send(S_NOPERM, NULL);
			return;
		}

		if(expTime)
			expTime += NOW;

		reason = srewind(data, i+1);

		s = shit->add(mask, c->handle->name, NOW, expTime, reason, stick);
		net.sendCmd(c, data+1, NULL);

		net.send(HAS_B, (char *) botnet_cmd, " ", channel ? channel : "*",  " ",
			 s->mask, " ", s->by,	" 0 ", itoa(s->expires), " ", s->reason, NULL);

		++userlist.SN;
		userlist.nextSave = NOW + SAVEDELAY;

		if(channel)
		{
			c->send("Added ", arg[0]+2, " `\002", mask, "'\002 on `\002", channel, "\002'", NULL);
			if(type == BAN && (ch = ME.findChannel(channel)))
				ch->applyShit(s);
		}

		else
		{
			c->send("Added ", arg[0]+2," `\002", mask, "\002'", NULL);

			if(type == BAN)
				foreachSyncedChannel(ch)
					ch->applyShit(s);
		}

		return;
	}
	if(!strcmp(arg[0], ".shits"))
	{
		net.sendCmd(c, data+1, NULL);
		protmodelist::sendShitsToOwner(c, BAN, arg[1], arg[2]);
		return;
	}
	if(!strcmp(arg[0], ".invites"))
	{
		net.sendCmd(c, data+1, NULL);
		protmodelist::sendShitsToOwner(c, INVITE, arg[1], arg[2]);
		return;
	}
	if(!strcmp(arg[0], ".exempts"))
	{ 
		net.sendCmd(c, data+1, NULL);
		protmodelist::sendShitsToOwner(c, EXEMPT, arg[1], arg[2]);
		return;
	}
	if(!strcmp(arg[0], ".reops"))
	{ 
		net.sendCmd(c, data+1, NULL);
		protmodelist::sendShitsToOwner(c, REOP, arg[1], arg[2]);
		return;
	} 

	if((!strcmp(arg[0], ".-shit") || !strcmp(arg[0], ".-invite") || !strcmp(arg[0], ".-exempt") || !strcmp(arg[0], ".-reop")) && strlen(arg[1]))
	{
		protmodelist *protlist;
		protmodelist::entry *s;
		chan *ch;
		int type = -1;
		char *botnet_cmd = NULL, mode[3];

		if(!strcmp(arg[0], ".-shit"))
		{
			type = BAN;
			botnet_cmd = S_RMSHIT;
		}

		else if(!strcmp(arg[0], ".-invite"))
		{
			type = INVITE;
			botnet_cmd = S_RMINVITE;
		}

		else if(!strcmp(arg[0], ".-exempt"))
		{
			type = EXEMPT;
			botnet_cmd = S_RMEXEMPT;
		}

		else if(!strcmp(arg[0], ".-reop"))
		{
			type = REOP;
			botnet_cmd = S_RMREOP;
		}

		if(strlen(arg[2]))
		{
			int i = userlist.findChannel(arg[2]);
			if(i != -1)
			{
				protlist = userlist.chanlist[i].protlist[type];
				if(!(c->handle->flags[i] & HAS_N) && !c->checkFlag(HAS_N))
				{
					c->send(S_NOPERM, NULL);
					return;
				}
			}
			else
			{
				c->send("Invalid channel", NULL);
				return;
			}
		}
		else
		{
			if(!(c->checkFlag(HAS_N)))
			{
				c->send(S_NOPERM, NULL);
				return;
			}
			protlist = userlist.protlist[type];
		}

		if((s=protlist->find(arg[1])))
		{
			mode[0] = '-';
			mode[1] = protlist->mode;
			mode[2] = '\0';

			if(strlen(arg[2]))
			{
				if((ch=ME.findChannel(arg[2])))
					if(ch->synced() && ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
						ch->modeQ[PRIO_LOW].add(NOW+5, mode, s->mask);
			}

			else
				foreachSyncedChannel(ch)
					if(ch->myTurn(ch->chset->GUARDIAN_BOTS, hash32(s->mask)))
						ch->modeQ[PRIO_LOW].add(NOW+5, mode, s->mask);
		}

		if(protlist->remove(arg[1]))
		{
			net.sendCmd(c, data+1, NULL);
			c->send(arg[0]+2, " has been removed", NULL);
			net.send(HAS_B, botnet_cmd, " ", arg[1], " ", arg[2], NULL);
			++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
		}
		else c->send("Invalid mask", NULL);

		return;
	}
	if((!strcmp(arg[0], ".botcmd") || !strcmp(arg[0], ".bc")) && strlen(arg[2]))
	{
		if(!strcmp(arg[1], config.handle))
		{
			net.sendCmd(c, data+1, NULL);
			pstring<> str(c->name);
			str += " ";
			str += srewind(data, 2);

			botnetcmd(config.handle, str);
		}
		else
		{
			inetconn *bot = net.findConn(userlist.findHandle(arg[1]));
			if(bot)
			{
				net.sendCmd(c, data+1, NULL);
				bot->send(S_BOTCMD, " ", c->name, " ", srewind(data, 2), NULL);
			}
			else
				c->send("Invalid bot", NULL);
		}
		return;
	}

	if(!strcmp(arg[0], ".help"))
	{
		net.sendCmd(c, "help", NULL);
		c->send("Available commands\002:", NULL);
		c->send(".+user   <handle> [host]          .-user    <handle>", NULL);
		c->send(".+host   <handle> <host>          .-host    <handle> <host>", NULL);
		c->send(".+bot    <handle> <ip>            .-bot     <handle>", NULL);
		c->send(".mjoin   <chan>   [key]   [delay] .tkmjoin  <chan>   [key]    [delay]", NULL);
		c->send(".rjoin   <bot>    <chan>  [key]   .tkrjoin  <bot>    <chan>", NULL);
		c->send(".sjoin   <slave>  <chan>  [key]   .tksjoin  <slave>  <chan>   [key]", NULL);
		c->send(".mpart   <chan>                   .rpart    <bot>    <chan>", NULL);
		c->send(".+chan   <chan>   [key]           .spart    <slave>  <chan>", NULL);
		c->send(".chattr  <handle> <flags> [chan]  .chpass   <handle> <pass>", NULL);
    		c->send(".chaddr  <handle> <ip>            .chhandle <handle> <new handle>", NULL);
    		c->send(".match   <expr>   [flags] [chan]  .whois    <handle>", NULL);
		c->send(".set     [var]    [value]         .gset     [var]    [value]", NULL);
		c->send(".chset   <chan>   [var]   [value] .chnick   <bot>    <nick>", NULL);
		c->send(".mcycle  <chan>                   .rcycle   <bot>    <chan>", NULL);
		c->send(".mk      <o|n|a>  <chan>  [lock]  .bye      [reason]", NULL);
    		c->send(".export  <file>   [pass]          .import   <file>   [pass]", NULL);
		c->send(".rjump   <bot>    <host>  [port]  .boot     <handle> <reason>", NULL);
		c->send(".rjump6  <bot>    <host>  [port]  .rflags   <handle> <chan>", NULL);
		c->send(".rjumps5 <bot>    <proxy> <port>  <server>  <port>", NULL);
		c->send(".list    <apcdsvuiU>      [bot]   .cwho     <bot>    <chan>    [flags]", NULL);
		c->send(".rdie    <bot>                    .names    <bot>    <chan>", NULL);
		c->send(".update  <bot>    [URL]           .restart  <bot>", NULL);
		c->send(".+info   <handle> <key>   <value> .-info    <handle> <key>", NULL);
		c->send(".info    <handle>                 .chhandle <handle> <handle>", NULL);
		c->send(".+shit   [#chan]  [%time] <mask>  [reason]", NULL);
		c->send(".+stick  [#chan]  [%time] <mask>  [reason]", NULL);
		c->send(".-shit   <mask>   [chan]          .shits    [chan]", NULL);
		c->send(".+exempt   [#chan]  [%time] <mask>  [reason]", NULL);
		c->send(".-exempt   <mask>   [chan]        .exempts    [chan]", NULL);
		c->send(".+invite   [#chan]  [%time] <mask>  [reason]", NULL);
		c->send(".-invite   <mask>   [chan]        .invites    [chan]", NULL);
		c->send(".+reop   [#chan]  [%time] <mask>  [reason]", NULL);
		c->send(".-reop   <mask>   [chan]          .reops    [chan]", NULL);
		c->send(".bots    [expr]   [flags]         .status   [bot]", NULL);
		c->send(".offences [handle]                .clearoffences [handle]", NULL);
		c->send(".verify [-a|-p|-h|-c]             ", NULL);
		c->send(".upbots .downbots .bottree  .who      .whom   .whob", NULL);
		c->send(".owners .channels .users    .save     .abuse ", NULL);
		c->send("allowed global flags: -aofmnxstrickedvqp (flag `d' overrides `aofm' flags)", NULL);
    		c->send("not allowed channel flags: -sxtp", NULL);
    		c->send("allowed bot flags: -lshp", NULL);
		c->send("Built-in handles: idiots", NULL);
		c->send("Read CHANGELOG for more details", NULL);
		return;
	}

	/* FIXME: Add support for restoring global flags */
	if((!strcmp(arg[0], ".rflags") || !strcmp(arg[0], ".rchattr")) && strlen(arg[2]))
	{
	    h = userlist.findHandle(arg[1]);
	    if(h && h != userlist.first)
	    {	    
		    if(!userlist.hasReadAccess(c, h))
		    {
			c->send(S_NOPERM, NULL);
			return;
		    }
		    if(!h->history || !h->history->data.entries())
		    {
			c->send("No offence history found; flags not restored", NULL);
			return;
		    }
		    
		    if(strlen(arg[2]))
		    {
			n = userlist.findChannel(arg[2]);
			if(n == -1)
			{
			    c->send("Channel not found; flags not restored", NULL);
			    return;
			}
		        
			ptrlist<offence::entry>::iterator o = h->history->data.begin();

			while(o)
			{
			    if(!strcmp(o->chan, arg[2]))
				break;
			    o++;
			}
			
			if(!o)
			{
			    c->send("No offence history found for ", arg[2], "; flags not restored", NULL);
			    return; 
			}
		    
			if((unsigned) h->flags[n] == (unsigned) o->fromFlags)
			{
			    c->send("Flags are allready the same; not changed", NULL);
			    return;
			}
			char flags1[32], flags2[32];
			
			userlist.flags2str(o->fromFlags, flags1);
			userlist.flags2str(h->flags[n], flags2);
		    
			h->flags[n] = o->fromFlags;

			net.sendCmd(c, "rflags ", arg[1], " ", arg[2],  NULL);
			c->send("Restoring \002", arg[1], "\002 flags for `\002", arg[2], "\002' to `\002", flags1, "\002'", NULL);
			
			if(set.PRE_0211_FINAL_COMPAT)
			{
		    	    net.send(HAS_B, S_CHATTR, " ", arg[1], " - ", arg[2], NULL);
		    	    net.send(HAS_B, S_CHATTR, " ", arg[1], " ", flags2, " ", arg[2], NULL);
		    	    ++userlist.SN;
			}
			else
			    net.send(HAS_B, S_CHATTR, " ", arg[1], " -", flags2, "+", flags1, " ", arg[2], NULL);
		    	++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
			return;		    
		}
	    }
	}
	
	if(!strcmp(arg[0], ".offences"))
	{
	    if(strlen(arg[1]))
	    {
		h = userlist.findHandle(arg[1]);
	    
		if(h)
		{	
			if(!userlist.hasReadAccess(c, h))
			{
				c->send(S_NOPERM, NULL);
				return;
			}
			if(!h->history || !h->history->data.entries())
			{
				c->send("No offence history available", NULL);
				return;
			}

			ptrlist<offence::entry>::iterator e = h->history->data.begin();
			char flags1[32], flags2[32];
			
			i = 1;
			net.sendCmd(c, "offences ", arg[1], NULL);
			c->send(h->name, "'s offence history: ", NULL);
			while(e)
			{
				userlist.flags2str(e->fromFlags, flags1);
				userlist.flags2str(e->toFlags, flags2);
				snprintf(buf, MAX_LEN, "[%3d]: %s(%d): %s\n       %s flags decreased from `%s' to `%s'\n       Created: %s", 
				i++, e->chan, e->count, e->mode, e->global ? "Global" : "Channel", flags1, flags2, timestr("%d/%m/%Y %T", e->time));
				c->send(buf, NULL);				
				e++;
			}
		}
		else
		{
		    c->send("Invalid handle", NULL);
		}
	    }
	    else
	    {
		userlist.reportNewOffences(c, true);
	    }
	    return;
	}

	if(!strcmp(arg[0], ".clearoffences"))
	{
	    if(strlen(arg[1]))
	    {
		h = userlist.findHandle(arg[1]);
	    
		if(h)
		{	
			if(!userlist.hasReadAccess(c, h))
			{
				c->send(S_NOPERM, NULL);
				return;
			}
			if(!h->history || !h->history->data.entries())
			{
				c->send("No offences history available", NULL);
				return;
			}
			net.sendCmd(c, "clearoffences ", arg[1], NULL);
			h->history->data.clear();
			
			c->send("Offences history removed", NULL);
			//++userlist.SN;
			userlist.nextSave = NOW + SAVEDELAY;
			
			delete h->history; // we should free unused memory ;)
			h->history = NULL;			
			return;
		}
		else
		{
		    c->send("Invalid handle", NULL);
		}
	    }
	    else
	    {
		i = n = 0;
		h = userlist.first;
		a = NULL;
		while(h)
		{
		    if(h->history && h->history->data.entries())
		    {
			if(userlist.hasReadAccess(c, h))
			{
			    h->history->data.clear();
			    i++;
			    a = push(a ? a : NULL, a ? (const char *) "\002,\002 " : (const char *) "", (const char *) h->name, NULL);
			    
			    delete h->history;
			    h->history = NULL;			    
			}
			n++;
		    }
		    h = h->next;
		}
		if(n)
		{
		    net.sendCmd(c, "clearoffences", NULL);
		    if(n == i) // all cleared
		    {
			c->send("Offences history removed", NULL);
		    }
		    else
		    {
			c->send("Offences history removed for handles(\002", itoa(i), "\002)\002:\002 ", a, NULL);
		    }
		    //++userlist.SN;
		    userlist.nextSave = NOW + SAVEDELAY;
		    
		    if(a) free(a);
		}
		else
		{
		    c->send("No offences history available", NULL);
		}
	    }
	    return;
	}

	if(!strcmp(arg[0], ".me") && strlen(arg[1]))
	{
	    a = srewind(data, 1);
	    if(a)
	    {
		for(i=0; i<net.max_conns; ++i)
		    if(net.conn[i].fd && net.conn[i].isRegUser())
			net.conn[i].send("\002* ", c->name, "\002 ", a, NULL);
	    }    
	    return;
	}

	if(arg[0][0] == '.')
	{
		c->send("What? You need `.help'?", NULL);
		return;
	}


	for(i=0; i<net.max_conns; ++i)
	{
		if(net.conn[i].fd && net.conn[i].isRegUser()
			&& ((c->status & STATUS_NOECHO) ? c != &net.conn[i] : 1))
			net.conn[i].send("<\002", c->name, "\002> ", data, NULL);
	}

	//for sake of compiler warnings
	n = 0;
}
