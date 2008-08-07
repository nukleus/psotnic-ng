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
#include "ctcp.h"

int querylen(const char *str)
{
	int len = 0;
	while(*str)
	{
		if(*str == '*' || *str == '?') break;
		++len;
		++str;
	}
	if(len && isspace(*--str)) --len;
	return len;
}

char *fetchVersion(char *buf, int client)
{
	FILE *p;
	char line[MAX_LEN], arg[3][MAX_LEN];

	memset(line, 0, MAX_LEN);
	memset(arg, 0, sizeof(arg));

	switch(client)
	{
		case VER_EPIC:
		case VER_LICE:
		{
			p = popen("epic -v 2> /dev/null", "r");
			if(p)
			{
				fgets(line, MAX_LEN, p);
				str2words(arg[0], line, 2, MAX_LEN);
				strcpy(buf, arg[1]);
				fclose(p);
			}
			if(!p || !match("EPIC*", buf))
				strcpy(buf, "EPIC4-1.2.6");
			return buf;
		}

		case VER_IRSSI:
		{
			p = popen("irssi -v 2> /dev/null", "r");
			if(p)
			{
				fgets(line, MAX_LEN, p);
				str2words(arg[0], line, 2, MAX_LEN);
				strcpy(buf, arg[1]);
				fclose(p);
			}
			if(!p || !match("*.*.*", buf))
				strcpy(buf, "0.8.9");
			return buf;
		}
		case VER_BITCHX:
		{
			p = popen("BitchX -v 2> /dev/null", "r");
			if(p)
			{
				fgets(line, MAX_LEN, p);
				fgets(line, MAX_LEN, p);
				str2words(arg[0], line, 3, MAX_LEN);
				strcpy(buf, arg[2]);
				fclose(p);
			}
			if(!p || !match("BitchX*", buf))
				strcpy(buf, "BitchX-1.1-final");

			return buf;
		}
		default: break;
	}
	return NULL;
}



void parse_ctcp(char *mask, char *data, char *to)
{
	char arg[10][MAX_LEN], *a, buf[MAX_LEN], who[MAX_LEN];
	chan *ch = ME.findChannel(to);
	int n, p;

	if(!strlen(data)) return;

	a = strchr(mask, '!');
	memset(who, 0, MAX_LEN);
	strncpy(who, mask, abs(a-mask));
	str2words(arg[0], data, 10, MAX_LEN);

	/* dcc chat */
	if(!strcmp(arg[0], "DCC") && !strcmp(arg[1], "CHAT") && strlen(arg[4]))
	{
		if(isValidIp(buf) == 6)
			strncpy(buf, arg[3], MAX_LEN-1);
		else 
			sprintf(buf, "%s", inet2char(htonl(strtoul(arg[3], NULL, 10))));

		if(config.bottype == BOT_MAIN && (p = userlist.hasPartylineAccess(mask)))
		{
			switch(isValidIp(buf))
			{
				#ifdef HAVE_IPV6
				case 6:
					n = doConnect6(buf, atoi(arg[4]), 0, -1);
					break;
				#endif

				default:
					n = doConnect(buf, atoi(arg[4]), config.myipv4, -1);
			}

			if(n > 0)
			{
				inetconn *c = net.addConn(n);
				c->status = STATUS_SYNSENT + STATUS_PARTY;
				c->killTime = NOW + set.AUTH_TIME;
				c->tmpint = 0;
				mem_strcpy(c->tmpstr, mask);
			}
			else
			{
				net.irc.send("PRIVMSG ", who, " :Cannot establish connection (", strerror(errno), ")", NULL);
			}
		}
		else if(config.bottype == BOT_MAIN)
		{
			net.send(HAS_N, "[-] Illegal dcc chat from: ", mask, NULL);
		}
	}
	else if(ch ? ch->chset->CHANNEL_CTCP : set.PRIVATE_CTCP)
	{
		DEBUG(printf("ctcp: %s\n", data));

		HOOK(ctcp, ctcp(mask, to, data));

		if(stopParsing)
		{
			stopParsing=false;
			return;
		}

		CTCPREPLY *db;

		switch(config.ctcptype)
		{
			case VER_PSOTNIC: db = ctcp_psotnic; break;
			case VER_IRSSI: db = ctcp_irssi; break;
			case VER_EPIC: db = ctcp_epic; break;
			case VER_LICE: db = ctcp_lice; break;
			case VER_BITCHX: db = ctcp_bitchx; break;
			case VER_DZONY: db = ctcp_dzony; break;
			case VER_LUZIK: db = ctcp_luzik; break;
			case VER_MIRC: db = ctcp_mirc; break;
			default: return;
		}

		int i;
		for(i=0; ;++i)
		{
			if(db[i].query)
			{
				if(match(db[i].query, data))
				{
					//printf("match: %s\n", db[i].query);
					n = querylen(db[i].query);

					if(config.ctcptype == VER_IRSSI || config.ctcptype == VER_PSOTNIC && n > 40)
							data[40] = '\0';

					expand(db[i].reply, buf, MAX_LEN, data + n + 1);
					ctcp.push("NOTICE ", who, " :\001", buf, "\001", NULL);
					if(!penalty)
					{
						ctcp.flush(&net.irc);
						penalty++;
					}
					break;
				}
			}
			else break;
		}

#ifdef HAVE_TCL
		snprintf(buf, MAX_LEN, "on_ctcp %s %s %s", mask, to, data);
		tclparser.eval(buf);
#endif
	}
}
