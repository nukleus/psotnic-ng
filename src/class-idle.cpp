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
#include "away.h"

idle::idle()
{
	away = NULL;
	lastMsg = NOW;
	lastStatus = NOW;
	if(!config.ctcptype) return;
}

void idle::init()
{
	load();
	calcNextStatus();
}

void idle::togleStatus()
{
	if(!config.ctcptype) return;
	if(away)
	{
		free(away);
		away = NULL;
	}
	else if(config.ctcptype == VER_BITCHX)
		away = push(NULL, getRandAwayAdd(), getRandAwayMsg(), NULL);
	else mem_strcpy(away, getRandAwayMsg());

	calcNextStatus();


	chan *ch = NULL;

	if(set.PUBLIC_AWAY && strlen(ME.nick))
	{
		ch = ME.first;
		int n = rand() % ME.channels;

		while(n-- && ch)
			ch = ch->next;
	}

	char buf[MAX_LEN];

	if(away)
	{
		int2units(buf, MAX_LEN, nextStatus - NOW, ut_time);
		//net.send(OWNERS, "[+] Entering AWAY mode for ", buf, NULL);
		if(strlen(ME.nick)) net.irc.send("AWAY :", away, NULL);
		if(ch)
		{
			if(config.ctcptype == VER_BITCHX)
				ctcp.push("PRIVMSG ", (const char *) ch->name, " :\001ACTION ", away, "\001", NULL);
			else ctcp.push("PRIVMSG ", (const char *) ch->name, " :\001ACTION ", getRandAwayAdd(), away, "\001", NULL);
		}

	}
	else
	{
		int2units(buf, MAX_LEN, nextStatus - NOW, ut_time);
		//net.send(OWNERS, "[+] Entering CHAT mode for ", buf, NULL);
		if(strlen(ME.nick)) net.irc.send("AWAY :", NULL);
		if(ch)
		{
			ctcp.push("PRIVMSG ", (const char *) ch->name, " :\001ACTION ", getRandBackAdd(), getRandBackMsg(), "\001", NULL);
		}
	}
}

int idle::spread(int x)
{
	if(!config.ctcptype)
		 return 0;
	srand();
	x = (rand() % abs(x * set.RANDOMNESS/100)) * ((rand() % 2 == 1) ? -1 : 1);
	return x;
}

void idle::calcNextStatus()
{
	if(!config.ctcptype) return;
	int t = (away) ? set.AWAY_TIME : set.CHAT_TIME;
	nextStatus = NOW + t + spread(t);
	lastStatus = NOW;
	if(!away) calcNextMsg();
}

void idle::calcNextMsg()
{
	if(!config.ctcptype) return;
	int t = set.BETWEEN_MSG_DELAY + spread(set.BETWEEN_MSG_DELAY);
	if(t < 10) t = 10;
	nextMsg = NOW + t;
}

void idle::sendMsg()
{
	if(!config.ctcptype) return;
	if(strlen(ME.nick))
	{
		net.irc.send("PRIVMSG ", (const char *) ME.nick, " :NULL", NULL);
		lastMsg = NOW;
	}
	calcNextMsg();
}

void idle::eval()
{
	if(!config.ctcptype) return;
	if(nextStatus <= NOW)
		togleStatus();
	if(!away && nextMsg <= NOW)
		sendMsg();
}

int idle::getIdle()
{
	if(!config.ctcptype) return 0;
	return abs(NOW - lastMsg);
}

int idle::getET()
{
	if(!config.ctcptype) return 0;
	return abs(NOW - lastStatus);
}

int idle::getRT()
{
	if(!config.ctcptype) return 0;
	return abs(nextStatus - NOW);
}

void idle::load()
{
	if(!config.ctcptype) return;

	switch(config.ctcptype)
	{
		case VER_PSOTNIC:
		{
			awayReasons = psotnic_away;
			backReasons = psotnic_back;
			awayAdd = psotnic_away_add;
			backAdd = psotnic_back_add;
			break;
		}
		case VER_IRSSI:
		{
			awayReasons = irssi_away;
			backReasons = irssi_back;
			awayAdd = irssi_away_add;
			backAdd = irssi_back_add;
			break;
		}
		case VER_EPIC:
		{
			awayReasons = epic_away;
			backReasons = epic_back;
			awayAdd = epic_away_add;
			backAdd = epic_back_add;
			break;
		}
		case VER_LICE:
		{
			awayReasons = lice_away;
			backReasons = lice_back;
			awayAdd = lice_away_add;
			backAdd = lice_back_add;
			break;
		}
		case VER_BITCHX:
		{
			awayReasons = bitchx_away;
			backReasons = bitchx_back;
			awayAdd = bitchx_away_add;
			backAdd = bitchx_back_add;
			break;
		}
		case VER_DZONY:
		{
			awayReasons = dzony_away;
			backReasons = dzony_back;
			awayAdd = dzony_away_add;
			backAdd = dzony_back_add;
			break;
		}
		case VER_LUZIK:
		{
			awayReasons = luzik_away;
			backReasons = luzik_back;
			awayAdd = luzik_away_add;
			backAdd = luzik_back_add;
			break;
		}
		case VER_MIRC:
		{
			awayReasons = mirc_away;
			backReasons = mirc_back;
			awayAdd = mirc_away_add;
			backAdd = mirc_back_add;
			break;
		}
		default:
		{
			awayReasons = NULL;
			backReasons = NULL;
			awayAdd = NULL;
			backAdd = NULL;
		}
	}
}

const char *idle::getRandAwayMsg()
{
	if(!config.ctcptype) return NULL;
	return awayReasons[rand() % count(awayReasons)];
}
const char *idle::getRandBackMsg()
{
	if(!config.ctcptype) return NULL;
	return backReasons[rand() % count(backReasons)];
}

const char *idle::getRandAwayAdd()
{
	if(!config.ctcptype) return NULL;
	return awayAdd[rand() % count(awayAdd)];
}

const char *idle::getRandBackAdd()
{
	if(!config.ctcptype) return NULL;
	return backAdd[rand() % count(backAdd)];
}
