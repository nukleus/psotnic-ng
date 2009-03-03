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

#include <cstdarg> // va_*
#include <errno.h> // for CONFIG::save()
#include <fcntl.h> // O_*, S_*, for save()
#include <pwd.h> // getpwuid, passwd

#include "global-var.h"
#include "match.h"
#include "Inet.hpp"
#include "random.hpp"

//options::event ent::_event;

/*
 * class options
 */
options::options()
{
	maxVarLen = 0;
}

void options::registerObject(const ent &e)
{
	int i = strlen(e.getName());

	if(i > maxVarLen)
		maxVarLen = i;

	list.addLast(&e);
}


/*
 * options::event
 */

options::event _event;
options::event *options::setVariable(const char *var, const char *value)
{
	ptrlist<ent>::iterator o = list.begin();
	event *e;

	while(o)
	{
		e = o->setValue(var, value);
		if(e)
			return e;

		o++;
	}
	_event.setNotFound("no such variable %s", var);
	return &_event;
}

const char *options::getValue(const char *var)
{
	ptrlist<ent>::iterator o = list.begin();

	while(o)
	{
		if(!strcmp(var, o->getName()))
		    return o->getValue();
		o++;
	}
	return NULL;
}

void options::sendToOwner(const char *owner, const char *var, const char *prefix)
{
	ptrlist<ent>::iterator o = list.begin();
	int i=0;

	while(o)
	{
		if(o->isPrintable() && (!*var || !strncmp(o->getName(), var, strlen(var))))
		{
			net.sendOwner(owner, prefix, ": ", o->print(maxVarLen), NULL);
			++i;
		}

		o++;
	}
}

bool options::parseUser(const char *from, const char *var, const char *value, const char *prefix, const char *prefix2)
{
	if((!value || !*value) && *var != '+' && *var != '-')
	{
		sendToOwner(from, var, prefix);
	}
	else
	{
		options::event *e = setVariable(var, value);

		if(e->ok)
		{
			net.sendCmd(from, prefix2, prefix, " ", var, " ", e->entity->getValue(), NULL);
			net.sendOwner(from, prefix, ": ", (const char *) e->reason, NULL);
			return 1;
		}
		net.sendOwner(from, prefix, ": ", (const char *) e->reason, NULL);

	}
	return 0;
}


void options::reset()
{
	ptrlist<ent>::iterator o = list.begin();

	while(o)
	{
		o->reset();
		o++;
	}
}

void options::sendToFile(inetconn *c, pstring<> prefix)
{
	ptrlist<ent>::iterator i = list.begin();

	while(i)
	{
		if(!i->isDefault() && i->isPrintable())
			c->send((const char *) prefix, " ", i->print(), NULL);
		i++;
	}
}

options::event::event()
{
	notFound = ok = 0;
}

void options::event::setOk(ent *e, const char *format, ...)
{
	char buf[MAX_LEN];
	va_list ap;
	va_start(ap, format);

	vsnprintf(buf, MAX_LEN, format, ap);
	reason = buf;
	ok = 1;
	notFound = 0;

	va_end(ap);

	entity = e;
}

void options::event::setError(ent *e, const char *format, ...)
{
	char buf[MAX_LEN];
	va_list ap;
	va_start(ap, format);

	vsnprintf(buf, MAX_LEN, format, ap);
	ok = 0;
	notFound = 0;
	reason = buf;

	va_end(ap);

	entity = e;
}

void options::event::setError(ent *e)
{
	ok = 0;
	notFound = 0;
	reason = "";
	entity = e;
}

void options::event::setNotFound(const char *format, ...)
{
	char buf[MAX_LEN];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, MAX_LEN, format, ap);
	va_end(ap);

	ok = 0;
	notFound = 1;
	reason = buf;
	entity = NULL;
}

#ifdef HAVE_DEBUG
void options::display()
{
	ptrlist<ent>::iterator i = list.begin();

	while(i)
	{
		if(i->isPrintable())
			printf("%s\n", i->print());
		i++;
	}
}
#endif

