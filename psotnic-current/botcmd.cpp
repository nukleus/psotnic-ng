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

void botnetcmd(const char *from, const char *cmd)
{
	char arg[10][MAX_LEN];

	HOOK(botnetcmd, botnetcmd(from, cmd));
	if(stopParsing)
	{
		stopParsing=false;
		return;
	}

	str2words(arg[0], cmd, 10, MAX_LEN);

	if(!strcmp(arg[1], "pset"))
	{
		HANDLE *h = userlist.findHandle(arg[0]);

		if(h && h->flags[GLOBAL] & HAS_X)
			pset.parseUser(arg[0], arg[2], arg[3], "pset");
		else
			net.sendOwner(arg[0], S_NOPERM, NULL);
	}
	else if(!strcmp(arg[1], "cfg"))
	{
		HANDLE *h = userlist.findHandle(arg[0]);
		if(h && h->flags[GLOBAL] & HAS_X)
			config.parseUser(arg[0], arg[2], srewind(cmd, 3), "cfg");
		else
			net.sendOwner(arg[0], S_NOPERM, NULL);

	}
	else if(!strcmp(arg[1], "cfg-save"))
	{
		HANDLE *h = userlist.findHandle(arg[0]);

		if(h && h->flags[GLOBAL] & HAS_X)
		{
			options::event *e = config.save();
			net.sendOwner(arg[0], "cfg-save: ", (const char *) e->reason, NULL);
		}
		else
			net.sendOwner(arg[0], S_NOPERM, NULL);
	}
	else if(!strcmp(arg[1], "modules"))
	{
		HANDLE *h = userlist.findHandle(arg[0]);

		if(h && h->flags[GLOBAL] & HAS_X)
		{
			int n = 0;
			ptrlist<module>::iterator i = modules.begin();
			while(i)
			{
				net.sendOwner(arg[0], "module: ", (const char *) i->file, " (v", (const char *) i->version, " by ", (const char *) i->author, ")", NULL);
				i++;
				n++;
			}
			net.sendOwner(arg[0], itoa(n), " modules has been found", NULL);
		}
		else
			net.sendOwner(arg[0], S_NOPERM, NULL);
	}
	else if(!strcmp(arg[1], "rehash"))
	{
		HANDLE *h = userlist.findHandle(arg[0]);
		if(h && h->flags[GLOBAL] & HAS_X)
		{
			ptrlist<module>::iterator i = modules.begin();

			int num = 0;

			while(i)
			{
				if(!*arg[2] || !strcmp(arg[2], i->file))
				{
					net.send(HAS_N, "rehasing module: ", (const char *) i->file, NULL);

					++num;

				}
				i++;
			}
			HOOK(rehash, rehash());
		}
		else
			net.sendOwner(arg[0], S_NOPERM, NULL);
	}
}
