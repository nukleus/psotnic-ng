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

#ifdef HAVE_ADNS

#include "prots.h"
#include "global-var.h"

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
}

adns::host2resolv::~host2resolv()
{
	free(host);
}

unsigned int adns::host2resolv::hash32() const
{
	return hash;
}

void adns::lock()
{
	pthread_mutex_lock(&mutex);
}

void adns::unlock()
{
	pthread_mutex_unlock(&mutex);
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

adns::host2ip *adns::getIp(const char *host)
{
	if(!th)
		return NULL;

	lock();
	host2ip *ip = __getIp(host);
	unlock();

	if(ip)
		ip->creat_t = NOW;
	return ip;
}

void adns::resolv(const char *host)
{
	if(!th)
		return;

	lock();
	if(!__getIp(host))
	{
		host2resolv *h = new host2resolv(host);
		if(resolving->find(*h) || todo->find(*h))
		{
			delete h;
			//DEBUG(printf(">>> %s is already RESOLVING\n", host));
			unlock();
			return;
		}

		//DEBUG(printf("adns::resolver::todo->add(\"%s\")\n", host));
		todo->add(h);
		pthread_cond_signal(&condition);
	}
	else
	{
		//DEBUG(printf(">>> %s hits CACHE\n", host));
	}
	unlock();
}

void adns::work()
{
	host2resolv *h;
	struct hostent ret;
	struct hostent *retptr;
	char buf4[MAX_LEN];
	char buf6[MAX_LEN];
	char resbuf[MAX_LEN];
	int error;

	while(1)
	{
		lock();
		h = todo->pop();

		if(h)
		{
			resolving->add(h);
			DEBUG(if(__getIp(h->host))
				printf(">>> DOUBLE RESOLVE of %s", h->host));
			strncpy(resbuf, h->host, MAX_LEN);
			unlock();

			if(die)
				break;

			*buf4 = '\0';
			*buf6 = '\0';

			if(!gethostbyname2_r(resbuf, AF_INET, &ret, buf4, MAX_LEN, &retptr, &error) && retptr)
			{
				inet_ntop(AF_INET, ret.h_addr, buf4, MAX_LEN);
				//DEBUG(printf(">>> %s resolved to: %s !!!\n", h->host, buf4));
			}
			else
				*buf4 = '\0';

			if(!gethostbyname2_r(resbuf, AF_INET6, &ret, buf6, MAX_LEN, &retptr, &error) && retptr)
			{
				inet_ntop(AF_INET6, ret.h_addr, buf6, MAX_LEN);
				//DEBUG(printf(">>> %s resolved to: %s !!!\n", h->host, buf6));

			}
			else
				*buf6 = '\0';

			if(die)
				break;

			lock();
			if(*buf4 || *buf6)
				cache->add(new host2ip(resbuf, buf4, buf6));
			//else
			//	DEBUG(printf(">>> unknown host: %s\n", h->host));

			resolving->remove(h);
			unlock();
		}
		else
		{
			//lock();
			pthread_cond_wait(&condition, &mutex);
			unlock();
		}
		if(die)
			break;
	}
}

void *__adns_work(void *me)
{
	((adns *) me) -> work();

	pthread_exit(NULL);
}

adns::adns()
{
	n = 0;
	th = NULL;
	die = false;
}

void adns::setTumberOfThreads(int threads)
{
	if(threads > 0)
	{
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		th = new pthread_t[threads];

		n = threads;

		todo = new hashlist<host2resolv>(1024);
		todo->removePtrs();
		cache = new hashlist<host2ip>(4096);
		cache->removePtrs();
		resolving = new hashlist<host2resolv>(threads * 4);
		resolving->removePtrs();

		int error;

		for(int i=0; i<n; ++i)
		{
			if((error = pthread_create(&th[i], &attr, __adns_work, this)))
			{
				printf("[-] AsyncDNS: cannot create resolving thread: %s\n", strerror(error));
				exit(1);
			}
		}
		pthread_attr_destroy(&attr);
	}
}

adns::~adns()
{
	//killThreads();
}

#ifdef HAVE_DEBUG
void adns::display()
{
	lock();
	if(todo)
	{
		printf(">>> todo\n");
		todo->displayStats();
	}
	if(cache)
	{
		printf(">>> cache\n");
		cache->displayStats();
	}
	unlock();
}
#endif

unsigned int adns::xorHash(const char *str)
{
	unsigned int hash = 5381;
	int c;

	while ((c = *str++))
		hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

	return hash;
}

void adns::expire(time_t t, time_t now)
{
	lock();
	if(cache)
		cache->expire(t, now);
	unlock();
}

void adns::closeThreads()
{
	//inform all threads that they have to die
	int status;

	for(int i=0; i<n; ++i)
	{
		DEBUG(printf("[*] AsyncDNS: joining thread %d\n", i));

		lock();
		die = 1;
		pthread_cond_broadcast(&condition);
		unlock();

		if(pthread_join(th[i], (void **)&status))
			printf("[-] AsyncDNS: pthread %d join failed\n", i);
		else
			DEBUG(printf("[+] AsyncDNS: pthread %d join SUCCESS\n", i));

	}

	if(todo)
		delete todo;
	if(cache)
		delete cache;
	if(resolving)
		delete resolving;

	if(th)
		delete [] th;

	todo = NULL;
	cache = NULL;
	resolving = NULL;
	th = NULL;
}

void adns::killThreads()
{
	for(int i=0; i<n; ++i)
		pthread_kill(th[i], SIGKILL);

			if(todo)
		delete todo;
	if(cache)
		delete cache;
	if(resolving)
		delete resolving;

	if(th)
		delete [] th;

	todo = NULL;
	cache = NULL;
	resolving = NULL;
	th = NULL;
}

#endif
