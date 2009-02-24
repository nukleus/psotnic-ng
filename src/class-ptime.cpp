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

static char __dupa[256];

ptime::ptime()
{
	gettimeofday(&tv, NULL);
}

ptime::ptime(const char *s, const char *n)
{
	tv.tv_sec = strtoul(s, NULL, 10);
	tv.tv_usec = strtoul(n, NULL, 10);
}

ptime::ptime(time_t s, time_t n)
{
	tv.tv_sec = s;
	tv.tv_usec = n;
}

char *ptime::print()
{
	snprintf(__dupa, 256, "%lu %lu", tv.tv_sec, tv.tv_usec);
	return __dupa;
}

char *ptime::ctime()
{
	char *t = ::ctime((const time_t *) &tv.tv_sec);
	t[strlen(t) - 1] = '\0';
	return t;
}

/*
int ptime::operator==(ptime &p)
{
    int i = tv.tv_sec == p.tv.tv_sec;
    int j = tv.tv_usec == p.tv.tv_usec;
    printf("time: %s (%p)\ntime: %s (%p)\n--- %d %d\n", print(), this, p.print(), &p, i, j);
	return i && j;
}
*/

