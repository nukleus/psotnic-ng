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

#ifndef ADNSPTHREAD_HPP
#define ADNSPTHREAD_HPP 

#ifdef HAVE_ADNS_PTHREAD

#include "Adns.hpp"

class adns_pthread : public adns
{
	private:
	bool die;

	pthread_mutex_t data_mutex;
	pthread_mutex_t condition_mutex;
	pthread_cond_t condition;

	pthread_t *th;
	int poolSize;

	void work();
	void removePool();
	void killThreads();

	public:
	virtual void resolv(const char *host);
	virtual host2ip *getIp(const char *host);
	void setupPool(int n);
	adns_pthread(int n);
	virtual ~adns_pthread();
	
	void expire(time_t t, time_t now);
	void lock_data();
	void unlock_data();

#ifdef HAVE_DEBUG
	void display();
#endif

	friend void *__adns_work(void *);
	friend class client;
};
#endif

#endif /* ADNSPTHREAD_HPP */
