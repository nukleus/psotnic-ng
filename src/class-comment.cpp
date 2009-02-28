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
#include "functions.hpp"

Comment::entry::entry(const char *k, const char *v)
{
	mem_strcpy(value, v);
	mem_strcpy(key, k);
}

Comment::entry::~entry()
{
	free(key);
	free(value);
}

int Comment::entry::operator==(const entry &ent) const
{
	return !strcmp(key, ent.key);
}

int Comment::entry::operator<(const entry &e) const
{
	return strcmp(key, e.key) < 0 ? 1 : 0;
}

Comment::Comment()
{
	data.removePtrs();
}

int Comment::add(char *key, char *value)
{
	if(strlen(value) > 50 || !isRealStr(value)) return 0;
	if(strlen(key) > 10 || !isRealStr(key)) return 0;

	entry *e = new entry(key, value);

	while(data.find(*e))
		data.remove(*e);

	data.sortAdd(e);
	return 1;
}

int Comment::del(char *key)
{
	entry e(key, "*");

	return data.remove(e);
}

char *Comment::get(char *key)
{
	entry e(key, "*");
	ptrlist<entry>::iterator ret = data.find(e);

	if(ret)
		return ret->value;
	else
		return NULL;
}
