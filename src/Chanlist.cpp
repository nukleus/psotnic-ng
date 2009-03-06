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

#include "Chanlist.hpp"
#include "Chanset.hpp"
#include "global-var.h"
#include "Module.hpp"
#include "Protmodelist.hpp"
#include "Wasop.hpp"

CHANLIST::CHANLIST() :
#ifdef HAVE_MODULES
		CustomDataStorage(),
#endif
		status(0), nextjoin(0), updated(0), chset(0), wasop(0), allowedOps(0)
{
}

void CHANLIST::reset()
{
	if(chset)
		delete(chset);
	if(wasop)
		delete(wasop);
	if(protlist[BAN])
		delete(protlist[BAN]);
	if(protlist[INVITE])
		delete(protlist[INVITE]);
	if(protlist[EXEMPT])
		delete(protlist[EXEMPT]);
	if(protlist[REOP])
		delete(protlist[REOP]);
	if(allowedOps)
		 delete(allowedOps);

#ifdef HAVE_MODULES
	HOOK(onDelCHANLIST(this))
#endif

	chset = NULL;
	wasop = NULL;
	protlist[BAN] = NULL;
	protlist[INVITE] = NULL;
	protlist[EXEMPT] = NULL;
	protlist[REOP] = NULL;
	allowedOps = NULL;

	pass = "";
	name = "";

	status = nextjoin = updated = 0;
}

