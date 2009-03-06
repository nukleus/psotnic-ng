/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin <grusin@gmail.com>          *
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

#ifndef CHANSET_HPP
#define CHANSET_HPP 

#include "class-ent.h"

class chanset : public options
{
	public:
	entInt AOP_BOTS;
	entInt BOT_AOP_BOTS;
	entInt BOT_AOP_MODE;
	entPerc PUNISH_BOTS;
	entInt INVITE_BOTS;
	entInt GUARDIAN_BOTS;
	entBool LIMIT;
	entTime LIMIT_TIME;
	entTime LIMIT_TIME_UP;
	entTime LIMIT_TIME_DOWN;
	entInt LIMIT_OFFSET;
	entInt LIMIT_BOTS;
	entPerc LIMIT_TOLERANCE;
	entBool CHANNEL_CTCP;
	entInt ENFORCE_BANS;
	entBool ENFORCE_LIMITS;
	entBool STOP_NETHACK;
	entInt GETOP_BOTS;
	entTime OWNER_LIMIT_TIME;
	entBool TAKEOVER;
	entBool BITCH;
	entBool WASOPTEST;
	entBool CLONECHECK;
	entBool DYNAMIC_BANS;
	entBool DYNAMIC_EXEMPTS;
	entBool DYNAMIC_INVITES;
	entBool LOCKDOWN;
	entTime LOCKDOWN_TIME;
	entInt PROTECT_CHMODES;
	entChattr MODE_LOCK;
	entBool STRICT_BANS;
	entBool CHECK_SHIT_ON_NICK_CHANGE;
	entBool INVITE_ON_UNBAN_REQUEST;
	entBool KEEPOUT;
	entInt IDIOTS;
	entInt USER_BANS;
	entInt USER_INVITES;
	entInt USER_EXEMPTS;
	entInt USER_REOPS;
	entBool CYCLE;
	
	chanset();
	chanset &operator=(const chanset &chset);
};

#endif /* CHANSET_HPP */
