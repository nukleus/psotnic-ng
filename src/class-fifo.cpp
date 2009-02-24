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

fifo::fifo(int size, int delay)
{
	maxEnt = size;
	flushDelay = delay;
	lastFlush = NOW;
}

fifo::~fifo()
{
	data.removePtrs();
	data.clear();
}

char *fifo::pop()
{
	ptrlist<pstring<8> >::iterator i = data.begin();
	if(i)
	{
		char *tmp = i->getCString();
		data.removeLink(i);
		return tmp;
	}
	return NULL;
}

char *fifo::flush()
{
	if(lastFlush >= NOW + flushDelay) return 0;
	return pop();
}

int fifo::flush(inetconn *c)
{
	if(lastFlush + flushDelay >= NOW) return 0;
	char *tmp = pop();
	if(tmp)
	{
		c->send(tmp, NULL);
		free(tmp);
		lastFlush = NOW;
		return 1;
	}
	return 0;
}

int fifo::wildWisePush(char *lst, ...)
{
	if(!maxEnt || data.entries() <= maxEnt)
	{
		ptrlist<pstring<8> >::iterator p= data.begin();

		va_list ap;
		int len;
		
		va_start(ap, lst);
		len = va_getlen(ap, lst);
		va_end(ap);
		char *str;
		va_start(ap, lst);
		str = va_push(NULL, ap, lst, len + 1);
		va_end(ap);

		while(p)
		{
			if(::wildMatch(p.obj(), str))
			{
				free(str);
				return 0;
			}
			p++;
		}
		data.addLast(new pstring<8>(str));
		free(str);
		return 1;
	}
	return 0;
}

int fifo::wisePush(const char *lst, ...)
{
	if(!maxEnt || data.entries() <= maxEnt)
	{
		ptrlist<pstring<8> >::iterator p = data.begin();

		va_list ap;
		int len;
		va_start(ap, lst);
		len = va_getlen(ap, lst);
		va_end(ap);
		char *str;
		va_start(ap, lst);
		str = va_push(NULL, ap, lst, len + 1);
		va_end(ap);

		while(p)
		{
			if(!strcmp(p.obj(), str))
			{
				free(str);
				return 0;
			}
			p++;
		}

		data.addLast(new pstring<8>(str));
		free(str);
		return 1;
	}
	return 0;
}

int fifo::push(const char *lst, ...)
{
	if(!maxEnt || data.entries() <= maxEnt)
	{
		va_list ap;
		int len;
		
		va_start(ap, lst);
		len = va_getlen(ap, lst);
		va_end(ap);

		char *str;
		va_start(ap, lst);
		str = va_push(NULL, ap, lst, len + 1);
		va_end(ap);
		data.addLast(new pstring<8>(str));
		free(str);
		return 1;
	}
	else return 0;
}

