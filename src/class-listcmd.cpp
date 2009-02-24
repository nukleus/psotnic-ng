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

//  return values:
//  0 - local
//  1 - remote
// -1 - error
int listcmd(char what, const char *from, const char *arg1, const char *arg2, inetconn *c)
{
	char *tmp = NULL;
	int i;

	switch(what)
	{
		//remote
		case 'a':
		{
			if(!config.ctcptype)
			{
				net.sendOwner(from, "[a] N/A", NULL);
				return 1;
			}
			char buf[MAX_LEN];
			char elapsed[MAX_LEN], remaining[MAX_LEN];

			int2units(elapsed, MAX_LEN, antiidle.getET(), ut_time);
			int2units(remaining, MAX_LEN, antiidle.getRT(), ut_time);

			if(antiidle.away)
			{
				snprintf(buf, MAX_LEN, "[a] away: on, elapsed: %s, remaining: %s",
					elapsed, remaining);
			}
			else
			{
				char next[MAX_LEN];
				int2units(next, MAX_LEN, int(antiidle.nextMsg - NOW), ut_time);
				snprintf(buf, MAX_LEN, "[a] away: off, elapsed: %s, remaining: %s, nextmsg: %s",
					elapsed, remaining, next);
			}
			net.sendOwner(from, buf, NULL);
			return 1;
		}
		//local and remote :)
		case 'p':
		{
			//owner orders hub to ping bots
			if(!c)
			{
				net.propagate(NULL, S_LIST, " p ", from, " ", itoa(NOW), " ",
						itoa(nanotime()), NULL);
			}
			else
			{
				if(config.bottype == BOT_MAIN)
				{
					time_t NANO = nanotime();
					time_t T = atoi(arg1);
					time_t N = atoi(arg2);
					char buf[MAX_LEN];

					if(NANO >= N)
						snprintf(buf, MAX_LEN, "%.3f", float(NOW-T) + float((NANO - N)/1e6));
					else
						snprintf(buf, MAX_LEN, "%.3f", float(NOW - T -1) + float(((1e6 - N + NANO)/1e6)));

					net.sendOwner(from, "[p] reply from ", c->handle->name, " in: ", buf, " secs", NULL);
				}
				else
				{
					if(c->isMain())
						c->send(S_LIST, " p ", from, " ", arg1, " ", arg2, NULL);
					else
					{
						for(i=0; i<net.max_conns; ++i)
							if(net.conn[i].isMain())
								net.conn[i].send(S_LIST, " p ", from, " ", arg1, " ", arg2, NULL);
					}
				}
			}
			return 0;
		}

		//remote
		case 's':
		{
			pstring <128> str;

			str = "[s] current: ";
			str += net.irc.name ? net.irc.name : "none";
			str += ", in config: ";

			for(i=0; i<MAX_SERVERS; ++i)
			{
				if(!config.server[i].isDefault())
				{
					str += config.server[i].getHost().connectionString;
					str += ":";
					str += itoa(config.server[i].getPort());
					str += " ";
				}
			}
			net.sendOwner(from, (const char *) str, NULL);
			return 1;
		}
		//remote
		case 'v':
		{
			net.sendOwner(from, "[v] version: ", S_VERSION, "; SVN rev: ", SVN_REVISION, NULL);
			return 1;
		}

		//remote
		case 'U':
		{
			char buf[MAX_LEN];
			struct rusage r;

			int2units(buf, MAX_LEN, int(NOW - ME.startedAt), ut_time);

			if(getrusage(RUSAGE_SELF, &r))
				net.sendOwner(from, "[U] up for\002:\002 ", buf, ", usr\002:\002 n/a, sys\002:\002 n/a, mem\002:\002 n/a", NULL);
			else
			{
				int2units(buf, MAX_LEN, int(NOW - ME.startedAt), ut_time);
				net.sendOwner(from, "[U] up for\002:\002 ", buf,
					", usr: ", itoa(r.ru_utime.tv_sec), ".", itoa(r.ru_utime.tv_usec / 10000), "s",
					", sys: ", itoa(r.ru_stime.tv_sec), ".", itoa(r.ru_stime.tv_usec / 10000), "s", NULL);
					//", mem: ", itoa(r.ru_idrss + r.ru_isrss + r.ru_ixrss), "k", NULL);
			}
			return 1;
		}

		//local
		case 'd':
		{
			if(!strlen(ME.nick))
				tmp = push(NULL, config.handle, " ", NULL);

			for(i=0; i<net.max_conns; ++i)
				if(net.conn[i].isRegBot() && (!net.conn[i].name || !*net.conn[i].name))
					tmp = push(tmp, net.conn[i].handle->name, " ", NULL);

			if(tmp)
			{
				net.sendOwner(from, "[d] ", tmp, NULL);
				free(tmp);
			}
			else net.sendOwner(from, "[d] all bots are on irc", NULL);
			return 0;
		}
		//remote
		case 'c':
		{

			for(i=0; i<MAX_CHANNELS; ++i)
			{
				if(userlist.chanlist[i].name && userlist.isRjoined(i))
				{
					chan *ch = ME.findChannel(userlist.chanlist[i].name);
					tmp = push(tmp, ch ? (ch->synced() ?
						((ch->me->flags & IS_OP) ? (char *) "@" : (char *) "") : (char *) "?")
						: (char *) "-", (const char *) userlist.chanlist[i].name, " ", NULL);
				}
			}
			if(tmp)
			{
				net.sendOwner(from, "[c] ", tmp, NULL);
				free(tmp);
			}
			else net.sendOwner(from, "[c] no channels", NULL);
			return 1;
		}
		case 'u':
		{
			struct utsname u;
			if(!uname(&u))
			{
				net.sendOwner(from, "[u] ", u.sysname, " ", u.nodename, " ", u.release, " ",
						u.version, " ", u.machine, NULL);
			}
			else
			{
				net.sendOwner(from, "[u] ", strerror(errno), NULL);
			}
			return 1;
		}
		case 'i':
		{
			if(net.irc.isReg())
			{
				if(userlist.wildFindHost(userlist.me(), ME.mask) != -1)
					net.sendOwner(from, "[i] connected to ", net.irc.name, " as ", (const char *) ME.mask, NULL);
				else
					net.sendOwner(from, "[i] connected to ", net.irc.name, " as ", (const char *) ME.mask, " (\002not in userlist\002)", NULL);
			}
			else
			{
				net.sendOwner(from, "[i] not on irc", NULL);
			}
			return 1;
		}
	}
	return -1;
}

