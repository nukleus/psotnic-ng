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

Pchar::Pchar(int s)
{
	step = s;
	alloced = s;
	len = 0;
	data = (char *) malloc(s);
	data[0] = '\0';
}

Pchar::~Pchar()
{
	if(data) free(data);
}

char *Pchar::push(const char c)
{
	char buf[2];
	buf[0] = c;
	buf[1] = '\0';
	return push(buf, 1);
}

char *Pchar::push(const char *str, int l)
{
	if(l == -1) l = strlen(str);
	if(l > 0)
	{
		if(l + len + 1 >= alloced)
		{
			alloced += step + l;
			data = (char *) realloc(data, alloced);
		}
		strncpy(data + len, str, l);
		len += l;
		data[len] = '\0';
	}
	return data;
}

void Pchar::clean()
{
	if(data)
	{
		if(step != alloced)
		{
			free(data);
			data = (char *) malloc(step);
		}
	}
	else
		data = (char *) malloc(step);

	alloced = step;
	len = 0;
	data[0] = '\0';
}
