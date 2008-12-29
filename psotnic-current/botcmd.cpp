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
#include "module.h"

void botnetcmd(const char *from, const char *cmd)
{
	char arg[10][MAX_LEN];

	HOOK(onBotnetcmd(from, cmd));
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
			ptrlist<Module>::iterator i = modules.begin();
			while(i)
			{
				net.sendOwner(arg[0], "module: ", (const char *) i->name(), " (v", (const char *) i->version(), " by ", (const char *) i->author(), ")", NULL);
				i++;
				n++;
			}
			if(n != 1)
				net.sendOwner(arg[0], itoa(n), " modules has been found", NULL);
			else
				net.sendOwner(arg[0], "1 module has been found", NULL);
			net.sendOwner(arg[0], "Use \"modinfo\"to get further information about specific modules.", NULL);
		}
		else
			net.sendOwner(arg[0], S_NOPERM, NULL);
	}
	else if ( !strcmp( arg[1], "modinfo" ) )
	{
		HANDLE *h = userlist.findHandle(arg[0]);

		if (h && h->flags[GLOBAL] & HAS_X)
		{
			if ( strlen( arg[2] ) )
			{
				int n = 0;
				ptrlist<Module>::iterator i = modules.begin();
				while (i)
				{
					if (!strcmp( arg[2], (const char*) i->name() ) )
					{
						char buf[MAX_LEN];
						strftime( buf, MAX_LEN, "%X %x", localtime( i->loadDate() ) );
						net.sendOwner( arg[0], "Extended module information for module ", (const char*) i->name(), ":", NULL );
						net.sendOwner( arg[0], "\tname:     ", (const char*) i->name(), NULL );
						net.sendOwner( arg[0], "\tdesc:     ", (const char*) i->description(), NULL );
						net.sendOwner( arg[0], "\tversion:  ", (const char*) i->version(), NULL );
						net.sendOwner( arg[0], "\tloaded:   ", buf, NULL );
						net.sendOwner( arg[0], "\tauthor:   ", (const char*) i->author(), " <", (const char*) i->email(), ">", NULL );
						net.sendOwner( arg[0], "\tcompiled: ", (const char*) i->compileDate(), " ", (const char*) i->compileTime(), NULL );
						net.sendOwner( arg[0], "\tfile:     ", (const char*) i->file(), NULL );
						if ( i->md5sum() )
						{
							net.sendOwner( arg[0], "\tMD5 sum:  ", (const char*) i->md5sum(), NULL );
						}
						else
						{
							net.sendOwner( arg[0], "\tMD5 sum: Module loaded in debug mode, no MD5 checksum saved!", NULL );
						}
						break;
					}
					i++;
					n++;
				}
			}
			else
			{
				net.sendOwner( arg[0], "Wrong syntax, use \"modinfo <module name>\"", NULL );
			}
		}
		else
		{
			net.sendOwner(arg[0], S_NOPERM, NULL);
		}
	}
	else if(!strcmp(arg[1], "rehash"))
	{
		HANDLE *h = userlist.findHandle(arg[0]);
		if(h && h->flags[GLOBAL] & HAS_X)
		{
			ptrlist<Module>::iterator i = modules.begin();

			int num = 0;

			while(i)
			{
				if(!*arg[2] || !strcmp(arg[2], i->file()))
				{
					net.send(HAS_N, "rehasing module: ", (const char *) i->file(), NULL);

					++num;

				}
				i++;
			}
			HOOK(onRehash());
		}
		else
			net.sendOwner(arg[0], S_NOPERM, NULL);
	}
}
