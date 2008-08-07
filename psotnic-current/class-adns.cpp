/***************************************************************************
 *   Copyright (C) 2003-2007 by Grzegorz Rusin                             *
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

#ifdef HAVE_ADNS

adns::~adns()
{
}

adns::host2ip *adns::__getIp(const char *host)
{
	host2ip tmp(host);
	ptrlist<host2ip>::iterator h = cache->find(tmp);

	if(h)
		return &h;
	else
		return NULL;
}

int adns::host2ip::operator==(const host2ip &h) const
{
	return hash == h.hash && !strcmp(host, h.host);
}

int adns::host2resolv::operator==(const host2resolv &h) const
{
	return hash == h.hash && !strcmp(host, h.host);
}

adns::host2ip::host2ip(const char *h, const char *i4, const char *i6)
{
	mem_strcpy(host, h);
	mem_strcpy(ip4, i4);
	mem_strcpy(ip6, i6);
	hash = xorHash(h);
	creat_t = NOW;
}

adns::host2ip::~host2ip()
{
	free(host);
	free(ip4);
	free(ip6);
}

unsigned int adns::host2ip::hash32() const
{
	return hash;
}

adns::host2resolv::host2resolv(const char *h)
{
	mem_strcpy(host, h);
	hash = xorHash(h);

	fd = 0;
	type = 0;
}

adns::host2resolv::~host2resolv()
{
	free(host);
}

unsigned int adns::host2resolv::hash32() const
{
	return hash;
}

unsigned int adns::xorHash(const char *str)
{
	unsigned int hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

#endif
