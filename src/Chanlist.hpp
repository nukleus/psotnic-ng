/***************************************************************************
 *   Copyright (C) 2003-2006 by Grzegorz Rusin <grusin@gmail.com           *
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

#ifndef CHANLIST_HPP
#define CHANLIST_HPP 

#include "pstring.h"
#include "defines.h"
#include "CustomDataStorage.hpp"

class chanset;
class wasoptest;
class protmodelist;

class CHANLIST
#ifdef HAVE_MODULES
	: public CustomDataStorage
#endif
{
	public:
	pstring<> name;
	pstring<> pass;
	int status;
	int nextjoin;
	char updated;
	chanset *chset;
	wasoptest *wasop;
	wasoptest *allowedOps;
	protmodelist *protlist[4];

	CHANLIST();
	void reset();
};

#endif /* CHANLIST_HPP */
