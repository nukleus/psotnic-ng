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

/* entry */
wasoptest::entry::entry(char *n, char *i, char *h)
{
	char *tmp = push(NULL, n, "!", i, "@", h, NULL);
	entry(tmp, 0);
}

wasoptest::entry::entry(char *m, int alloc)
{
	if(alloc) mem_strcpy(mask, m);
	else mask = m;
	when = NOW;
}

wasoptest::entry::~entry()
{
	free(mask);
}

wasoptest::wasoptest(int life)
{
	since = NOW;
	TOL = life;
	data.removePtrs();
}

/* add */
int wasoptest::add(chanuser *p)
{
	add(p->nick, p->ident, p->host);
	return data.entries();
}

int wasoptest::add(char *nick, char *ident, char *host)
{
	add(push(NULL, nick, "!", ident, "@", host, NULL), 0);
	return data.entries();
}

int wasoptest::add(char *mask, int alloc)
{
	data.addLast(new entry(mask, alloc));
	DEBUG(printf("### add@: %s\n", mask));
	return data.entries();
}

int wasoptest::remove(chanuser *user)
{
	return remove(user->nick, user->ident, user->host);
}

int wasoptest::remove(char *nick, char *ident, char *host)
{
	char *mask = push(NULL, nick, "!", ident, "@", host, NULL);
	int ret = remove(mask);
	free(mask);
	return ret;
}

int wasoptest::remove(char *mask)
{
	ptrlist<entry>::iterator p = data.begin();

	while(p)
	{
		if(!strcmp(p->mask, mask))
		{
			data.removeLink(p);
			return 1;
		}
		p++;
	}
	return 0;
}

int wasoptest::checkSplit(const char *str)
{
	char arg[2][MAX_LEN];
	str2words(arg[0], str, 2, MAX_LEN);

	if(match("*.* *.*", str) && strlen(arg[0]) + strlen(arg[1]) + 1 == strlen(str))
		return 1;
	else return 0;
}

void wasoptest::expire()
{
	ptrlist<entry>::iterator q, p = data.begin();

	while(p)
	{
		if(p->when + TOL <= NOW)
		{
			q = p;
			q++;;
			data.removeLink(p);
			p = q;
		}
		else p++;
	}
}

bool wasoptest::isEmpty()
{
	return data.entries() == 0;
}
