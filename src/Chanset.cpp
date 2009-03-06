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

#include "Chanset.hpp"

chanset::chanset()
{
	registerObject(AOP_BOTS = entInt("aop-bots", 0, MAX_INT, 1));
	registerObject(BOT_AOP_BOTS = entInt("bot-aop-bots", 0, MAX_INT, 1));
	registerObject(BOT_AOP_MODE = entInt("bot-aop-mode", 0, 2, 1));
	registerObject(PUNISH_BOTS = entPerc("punish-bots", -100, 0, -30));
	registerObject(GETOP_BOTS = entInt("getop-bots", 0, MAX_INT, 1));
	registerObject(INVITE_BOTS = entInt("invite-bots", 0, MAX_INT, 2));
	registerObject(GUARDIAN_BOTS = entInt("guardian-bots", 0, MAX_INT, 2));
	registerObject(CHANNEL_CTCP = entBool("channel-ctcp", 1));
	registerObject(ENFORCE_BANS = entInt("enforce-bans", 0, 2, 1));
	registerObject(ENFORCE_LIMITS = entBool("enforce-limits", 0));
	registerObject(STOP_NETHACK = entBool("stop-nethack", 1));
	registerObject(LIMIT = entBool("limit", 1));
	registerObject(LIMIT_TIME = entTime("limit-time", 1, MAX_INT, 120));
	registerObject(LIMIT_TIME_UP = entTime("limit-time-up", 1, MAX_INT, 120));
	registerObject(LIMIT_TIME_DOWN = entTime("limit-time-down", 1, MAX_INT, 30));
	registerObject(LIMIT_OFFSET = entInt("limit-offset", 1, MAX_INT, 5));
	registerObject(LIMIT_BOTS = entInt("limit-bots", 1, 2, 1));
	registerObject(LIMIT_TOLERANCE = entPerc("limit-tolerance", -100, MAX_INT, -50));
	registerObject(OWNER_LIMIT_TIME = entTime("owner-limit-time", 0, MAX_INT, 15));
	registerObject(TAKEOVER = entBool("takeover", 0));
	registerObject(BITCH = entBool("bitch", 1));
	registerObject(WASOPTEST = entBool("wasoptest", 1));
	registerObject(CLONECHECK = entInt("clonecheck", 0, 2, 1));
	registerObject(DYNAMIC_BANS = entBool("dynamic-bans", 1));
	registerObject(DYNAMIC_INVITES = entBool("dynamic-invites", 0));
	registerObject(DYNAMIC_EXEMPTS = entBool("dynamic-exempts", 0));
	registerObject(LOCKDOWN = entBool("lockdown", 0));
	registerObject(LOCKDOWN_TIME = entTime("lockdown-time", 0, MAX_INT, 180));
	registerObject(PROTECT_CHMODES = entInt("protect-chmodes", 0, 2, 2));
	registerObject(MODE_LOCK = entChattr("mode-lock", "+nt"));
	registerObject(STRICT_BANS = entBool("strict-bans", 1));
	registerObject(CHECK_SHIT_ON_NICK_CHANGE = entBool("check-shit-on-nick-change", 0));
	registerObject(INVITE_ON_UNBAN_REQUEST = entBool("invite-on-unban-request", 0));
	registerObject(KEEPOUT = entBool("keepout", 0));
	registerObject(IDIOTS = entInt("idiots", 0, 5, 2));
	registerObject(USER_INVITES = entInt("user-invites", 0, 2, 1));
	registerObject(USER_EXEMPTS = entInt("user-exempts", 0, 2, 1));
	registerObject(USER_REOPS = entInt("user-reops", 0, 2, 1));
        registerObject(CYCLE = entBool("cycle", 1));
}

chanset &chanset::operator=(const chanset &chset)
{
	ptrlist<ent>::iterator a, b;
	a = list.begin();
	b = chset.list.begin();

	while(a && b)
	{
		//printf(">>> setting: %s = %s\n", (const char *) a->getName(), (const char *) b->getValue());
		a->set(b->getValue());
		a++;
		b++;
	}

	return *this;
}

