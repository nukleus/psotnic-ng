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

#ifdef HAVE_MODULES

void stop()
{
	stopParsing=true;
}

/*
 * Module loading related stuff
 */

#define _initCustomData(what, name) \
if(!strcmp(name, Class)) \
{ \
    memcpy(&what::customDataConstructor, &constructor, sizeof(void *)); \
    memcpy(&what::customDataDestructor, &destructor, sizeof(void *)); \
    return 1; \
}

FUNCTION (*chanuser::customDataConstructor)(chanuser *) = NULL;
FUNCTION (*chanuser::customDataDestructor)(chanuser *) = NULL;
FUNCTION (*CHANLIST::customDataConstructor)(CHANLIST *) = NULL;
FUNCTION (*CHANLIST::customDataDestructor)(CHANLIST *) = NULL;
FUNCTION (*chan::customDataConstructor)(chan *) = NULL;
FUNCTION (*chan::customDataDestructor)(chan *) = NULL;

int initCustomData(const char *Class, FUNCTION constructor, FUNCTION destructor)
{
	_initCustomData(chanuser, "chanuser");
	_initCustomData(CHANLIST, "CHANLIST");
	_initCustomData(chan, "chan");

	return 0;
}

//#include "register_module.h"

#endif
