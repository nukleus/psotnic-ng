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

ign::entry::entry(unsigned int IP)
{
	ip = IP;
	when = NOW;
	count = 1;
	nextConn = 0;
}

int ign::entry::operator==(const entry &e) const
{
	return ip == e.ip;
}

int ign::entry::operator==(const unsigned int &IP) const
{
	return ip == IP;
}

time_t ign::entry::creation()
{
	return when;
}

ign::ign()
{
	data.removePtrs();
	nextConn = 0;
}

ign::entry *ign::hit(unsigned int ip)
{
	entry *e = new entry(ip);
	ptrlist<entry>::iterator p = data.find(*e);

	if(p)
	{
		if(++p->count > set.PERIP_BURST_SIZE && p->nextConn <= NOW)
		{
			p->nextConn = NOW + set.PERIP_IGNORE_TIME;
			net.send(HAS_N, "[!] Ignoring ", inet2char(ntohl(ip)), " for ", itoa(set.PERIP_IGNORE_TIME), " seconds", NULL);
		}
		delete e;
		e = p;
	}
	else
	{
		data.add(e);
	}

	++count;

	if(count >= set.SYNFLOOD_MAX_CONNS && nextConn <= NOW)
	{
		net.send(HAS_N, "Synflood detected, not accepting conections for ", itoa(set.SYNFLOOD_IGNORE_TIME), " seconds", NULL);
		nextConn = NOW + set.SYNFLOOD_IGNORE_TIME;
	}

	return e;
}

void ign::expire()
{
	data.expire(set.PERIP_BURST_TIME, NOW);
	calcCount();
}

void ign::removeHit(unsigned int ip)
{
	entry e(ip);
	ptrlist<entry>::iterator p = data.find(e);


	if(p && --p->count <= 0)
		data.removeLink(p);

	calcCount();
}


void ign::calcCount()
{
	count = 0;

	ptrlist<entry>::iterator p = data.begin();

	while(p)
	{
		count += p->count;
		p++;
	}
}

