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

masklist_ent::masklist_ent(char *m, char *w, time_t t)
{
	mem_strcpy(mask, m);
	mem_strcpy(who, w);
	when = NOW;
	srand(NOW + ME.startedAt + getpid());
	expire = t ? (NOW + t + (rand() % 600)) : 0;
	sent = 0;
}

masklist_ent::~masklist_ent()
{
	free(mask);
	free(who);
}

masklist::masklist()
{
	masks.removePtrs();
}

int masklist::add(char *mask, char *who, time_t t)
{
	if(find(mask)) return 0;

	masklist_ent *m = new masklist_ent(mask, who, t);
	masks.addLast(m);
	return 1;
}

int masklist::remove(char *mask)
{
	masklist_ent *m = find(mask);

	if(!m) return 0;

	masks.remove(m);
	return 1;
}

masklist_ent *masklist::find(char *mask)
{
	ptrlist<masklist_ent>::iterator m = masks.begin();

	while(m)
	{
		if(!strcmp(m->mask, mask)) return m;
		m++;
	}
	return NULL;
}

masklist_ent *masklist::wildMatch(char *mask)
{
	ptrlist<masklist_ent>::iterator m = masks.begin();

	while(m)
	{
		if(::wildMatch(mask, m->mask)) return m;
		m++;
	}
	return NULL;
}

masklist_ent *masklist::match(const char *mask)
{
	ptrlist<masklist_ent>::iterator m = masks.begin();

	while(m)
	{
		if(::match(m->mask, mask))
			return m;

		m++;
	}
	return NULL;
}

masklist_ent *masklist::matchBan(char *mask, char *ip, char *uid)
{
	ptrlist<masklist_ent>::iterator m = masks.begin();

	while(m)
	{
		if(::matchBanMask(m->mask, mask, 0, ip, uid))
			return m;
		m++;
	}
	return NULL;
}

ptrlist<masklist_ent>::iterator masklist::expire(ptrlist<masklist_ent>::iterator m)
{
	//we continue it
	if(!m)
		m = masks.begin();

	while(m)
	{
		if(m->expire && m->expire <= NOW)
			return m;
		m++;
	}
	return NULL;
}

int masklist::remove(masklist_ent *m)
{
	return masks.remove(m);
}

void masklist::clear()
{
	masks.clear();
}
