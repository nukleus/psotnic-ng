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

#ifndef ADNSFIREDNS_HPP
#define ADNSFIREDNS_HPP 

#ifdef HAVE_ADNS_FIREDNS

#include "Adns.hpp"

class adns_firedns : public adns
{
	private:
	time_t last_check;
	bool shouldWeCheck();

	public:
	virtual void resolv(const char *host);
	virtual host2ip *getIp(const char *host);
	adns_firedns();
	virtual ~adns_firedns();
	
	void expire(time_t t, time_t now);

#ifdef HAVE_DEBUG
	void display();
#endif

	int fillFDSET(fd_set *set);
	void processResultSET(fd_set *set);
	void closeAllConnections();
};
#endif

#endif /* ADNSFIREDNS_HPP */
