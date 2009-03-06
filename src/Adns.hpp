/***************************************************************************
 *   Copyright (C) 2003-2007 by Grzegorz Rusin <grusin@gmail.com>          *
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

#ifndef ADNS_HPP
#define ADNS_HPP 

#ifdef HAVE_ADNS

#include "hashlist.h"

class adns
{
	public:
	class host2ip
	{
		public:
		char *host;
		char *ip4;
		char *ip6;
		time_t creat_t;

		unsigned int hash;

		host2ip(const char *h, const char *i4="", const char *i6="");
		~host2ip();
		unsigned int hash32() const;
		int operator==(const host2ip &h) const;
		time_t creation() { return creat_t; };
	};

	class host2resolv
	{
		public:
		char *host;
		unsigned int hash;
		int fd;
		int type;

		host2resolv(const char *h);
		~host2resolv();
		unsigned int hash32() const;
		int operator==(const host2resolv &h) const;
	};

	protected:
	hashlist<host2ip> *cache;
	hashlist<host2resolv> *resolving;
	hashlist<host2resolv> *todo;

	host2ip *__getIp(const char *host);
	
	public:
	virtual void resolv(const char *host) = 0;
	virtual host2ip *getIp(const char *host) = 0;
	virtual void expire(time_t t, time_t now) = 0;

	static unsigned int xorHash(const char *str);
	

	virtual ~adns();	
#ifdef HAVE_DEBUG
	void display();
#endif
};

extern adns *resolver;

#endif

#endif /* ADNS_HPP */
