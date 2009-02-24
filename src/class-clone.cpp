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

/*
** Host clones
*/

clone_host::clone_host(chanuser *u, int t)
{
	cr = NOW;
	user = u;
	type = t;
}

time_t clone_host::creation()
{
	return cr;
}

int clone_host::operator==(const clone_host &c)
{
	return *c.user == *user;
}

int clone_host::operator&(const clone_host &c)
{
	if(c.type != type)
		return 0;

	assert(c.user != NULL);
	assert(user != NULL);
	
	if(c.type == HOST_IPV4 && (user->dnsinfo & HOST_IPV4) && (c.user->dnsinfo & HOST_IPV4) && ipcmp(user->ip4, c.user->ip4, '.', 3))
	{
		DEBUG(printf("[D] DNS: HOST_IPV4 matches %s!%s@%s and %s!%s@%s\n", user->nick, user->ident, user->ip4, c.user->nick, c.user->ident, c.user->ip4)); 
		return 1;
	}
	
	if(c.type == HOST_IPV6 && (user->dnsinfo & HOST_IPV6) && (c.user->dnsinfo & HOST_IPV6) && ipcmp(user->ip6, c.user->ip6, ':', 4))
	{
		DEBUG(printf("[D] DNS: HOST_IPV6 matches %s!%s@%s and %s!%s@%s\n", user->nick, user->ident, user->ip6, c.user->nick, c.user->ident, c.user->ip6));
		return 1;
	}
	
	if(c.type == HOST_DOMAIN && (user->dnsinfo & HOST_DOMAIN) && (c.user->dnsinfo & HOST_DOMAIN) && !strcmp(c.user->host, user->host))
	{
		DEBUG(printf("[D] DNS: HOST_DOMAIN matches %s!%s@%s and %s!%s@%s\n", user->nick, user->ident, user->host, c.user->nick, c.user->ident, c.user->host));
		return 1;
	}
	
	return 0;
}

/*
** Ident clones
*/

clone_ident::clone_ident(chanuser *u)
{
	cr = NOW;
	user = u;
}

time_t clone_ident::creation()
{
	return cr;
}

int clone_ident::operator==(const clone_ident &c)
{
	return *c.user == *user;
}

int clone_ident::operator&(const clone_ident &c)
{
	char *p, *q;

	if(isPrefix(*user->ident)) p = user->ident + 1;
	else p = user->ident;

	if(isPrefix(*c.user->ident)) q = c.user->ident + 1;
	else q = c.user->ident;

	return !strcmp(p, q);
}

/*
** Proxy clones
*/

clone_proxy::clone_proxy(chanuser *u)
{
	cr = NOW;
	user = u;
}

time_t clone_proxy::creation()
{
	return cr;
}

int clone_proxy::operator==(const clone_proxy &c)
{
	return *c.user == *user;
}

int clone_proxy::operator&(const clone_proxy &c)
{
	return !domaincmp(user->host, c.user->host, 2);
}
