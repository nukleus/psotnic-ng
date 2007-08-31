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

static char arg[10][MAX_LEN];
static char *reason;

void parse_bot(inetconn *c, char *data)
{
	reason = NULL;

	if(!strlen(data)) return;
	str2words(arg[0], data, 10, MAX_LEN);

	/* REGISTER CONNECTION */
	if(!(c->status & STATUS_REGISTERED))
	{
		switch(c->tmpint)
		{
			case 1:
			{
				if(!strcmp(arg[0], config.botnetword))
				{
					c->enableCrypt((const char *) config.botnetword, strlen(config.botnetword));
					c->tmpstr = (char *) malloc(AUTHSTR_LEN + 1);
					++c->tmpint;
					MD5CreateAuthString(c->tmpstr, AUTHSTR_LEN);
					c->tmpstr[32] = '\0';
					c->send(c->tmpstr, NULL);
					return;
				}
				
				/* maybe that's owner */
				if(config.bottype == BOT_MAIN && set.TELNET_OWNERS && MD5Validate(config.ownerpass, arg[0], strlen(arg[0])))
				{
                    c->status |= STATUS_CONNECTED | STATUS_PARTY | STATUS_TELNET;
					c->status &= ~STATUS_BOT;
					c->tmpint = 1;
					c->killTime = NOW + set.AUTH_TIME;
					if(creation)
					{
						c->send("Welcome ", c->getPeerIpName(), " to the constructor", NULL);
						//c->echo(1);
#ifdef HAVE_SSL
						if(c->isSSL())
							SSL_write(c->ssl, "new login: ", strlen("new login: "));
						else
#endif
							write(c->fd, "new login: ", strlen("new login: "));
					}
					else
					{
						//c->echo(1);
						c->send("Welcome ", c->getPeerIpName(), NULL);
#ifdef HAVE_SSL
						if(c->isSSL())
							SSL_write(c->ssl, "login: ", strlen("login: "));
						else
#endif
							write(c->fd, "login: ", strlen("login: "));

					}
					return;
				}
				DEBUG(printf("[D] telnet creep: %s [%d]\n", arg[0], strlen(arg[0])));
				reason = push(NULL, "telnet creep", NULL);
				break;
			}
			case 2:
			{
				if(strlen(arg[1]))
				{
					struct sockaddr_in peer;
					HANDLE *h = userlist.findHandle(arg[0]);

					if(h && net.findConn(h))
					{
                    				reason = push(NULL,  arg[0], ": duplicate connection",NULL);
						break;
					}
					if(h && config.bottype == BOT_MAIN && !userlist.isSlave(h))
					{
						reason = push(NULL, arg[0], ": not a slave", NULL);
						break;
					}

					if(h && config.bottype == BOT_SLAVE && !userlist.isLeaf(h))
					{
						reason = push(NULL, arg[0], ": not a leaf", NULL);
						break;
					}

					socklen_t peersize = sizeof(struct sockaddr_in);
                    getpeername(c->fd, (sockaddr *) &peer, &peersize);

                    if(!h)
                    {
						if(isRealStr(arg[0]) && strlen(arg[0]) <= MAX_HANDLE_LEN)
							reason = push(NULL, arg[0], ": not a bot", NULL);
                    	else
							reason = push(NULL, "(crap here): not a bot", NULL);

						break;
					}
                    if(!h->pass)
                    {
						reason = push(NULL, arg[0], ": no password set", NULL);

						break;
					}
					if(!h->ip || peer.sin_addr.s_addr == h->ip)
					{
   						if(MD5HexValidate(arg[1], c->tmpstr, strlen(c->tmpstr), h->pass, 16))
						{
							++c->tmpint;
							c->handle = h;
							free(c->tmpstr);
							c->tmpstr = NULL;
							return;
						}
						else
						{
							reason = push(NULL, arg[0], ": wrong botpass", NULL);
                        	break;
						}
					}
					else
					{
						reason = push(NULL, arg[0], ": invalid botip", NULL);
                    	break;
					}
				}
				reason = push(NULL, "This should not happen (1)", NULL);
				break;
			}
			case 3:
			{
				if(strlen(arg[0]))
				{
					char hash[33];

					++c->tmpint;
					MD5HexHash(hash, arg[0], AUTHSTR_LEN, c->handle->pass, 16);
					c->send(config.handle, " ", userlist.first->next->creation->print(), " ", hash, NULL);
					return;
				}
				reason = push(NULL, "This should not happen (2)", NULL);
				break;
			}
			case 4:
			{
				/* S_REGISTER <S_VERSION> <userlist.SN> [ircnick [irc server] ] */
				if(!strcmp(arg[0], S_REGISTER) && strlen(arg[2]))
				{
					if(strcmp(arg[1], S_VERSION))
					{
						net.send(HAS_N, "[!] ", c->handle->name, " has different version: ", arg[1], NULL);
					}

					mem_strcpy(c->name, arg[3]);
					mem_strcpy(c->origin, arg[4]);
					c->status |= STATUS_CONNECTED | STATUS_REGISTERED | STATUS_BOT;
					c->tmpint = 0;
					c->killTime = NOW + set.CONN_TIMEOUT;
					c->lastPing = NOW;

					c->send(S_REGISTER, " ", (const char *) ME.nick, NULL);

					c->enableCrypt(c->handle->pass, 16);

					/* update ul */
					if(userlist.SN != strtoull(arg[2], NULL, 10))
					{
						net.send(HAS_N, "[*] ", c->handle->name, " is linked (sending userlist)", NULL);
						userlist.send(c);
					}
					else net.send(HAS_N, "[+] ", c->handle->name, " is linked and operational", NULL);


					/* send list of bots */
					net.sendBotListTo(c);
					net.propagate(c, S_BJOIN, " ", c->name, " ", c->origin, NULL);

					/* check bot host */
					if(config.bottype == BOT_MAIN)
						c->send(S_CHKHOST, " *", NULL);

					ignore.removeHit(c->getPeerIp4());
					return;
				}
                
                if(!strcmp(arg[0], S_IUSEMODULES))
                    return;

				reason = push(NULL, "This should not happen (3)", NULL);
				break;
			}
			default: break;
		}
		/* HUH */
		if(!reason)
			reason = push(NULL, "Unknown error", NULL);
		c->close(reason);
		free(reason);
		return;
	}

	/* PARSE DATA FROM REGISTERED BOT */
	c->killTime = NOW + set.CONN_TIMEOUT;

	if(!strcmp(arg[0], S_UL_UPLOAD_START))
	{
		c->close("Go fuck yourself");
		return;
	}
	if(!strcmp(arg[0], S_ULOK))
	{
		net.send(HAS_N, "[+] ", c->handle->name, " is operational", NULL);
		return;
	}
	parse_botnet(c, data);
}
