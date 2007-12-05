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

#ifdef HAVE_ADNS_PTHREAD

#include "prots.h"
#include "global-var.h"

void adns_pthread::lock_data()
{
	pthread_mutex_lock(&data_mutex);
}

void adns_pthread::unlock_data()
{
	pthread_mutex_unlock(&data_mutex);
}

adns_pthread::host2ip *adns_pthread::getIp(const char *host)
{
	if(!th)
		return NULL;

	pthread_mutex_lock(&data_mutex);
	host2ip *ip = __getIp(host);

	if(ip)
		ip->creat_t = NOW;
	pthread_mutex_unlock(&data_mutex);
	return ip;
}

void adns_pthread::resolv(const char *host)
{
	if(!th)
		return;

	DEBUG(printf(">>> Attempting to resolv: %s\n", host));
	pthread_mutex_lock(&data_mutex);
	if(!__getIp(host))
	{
		host2resolv *h = new host2resolv(host);
		if(resolving->find(*h) || todo->find(*h))
		{
			delete h;
			DEBUG(printf(">>> %s is already RESOLVING\n", host));
		}
		else
		{
			DEBUG(printf("adns_pthread::resolver::todo->add(\"%s\")\n", host));
			todo->add(h);

			pthread_cond_broadcast(&condition);
		}
	}
	else
	{
		DEBUG(printf(">>> %s hits CACHE\n", host));
	}
	pthread_mutex_unlock(&data_mutex);
}

void adns_pthread::work()
{
	host2resolv *h;
	struct hostent ret;
	struct hostent *retptr;
	char buf4[MAX_LEN];
	char buf6[MAX_LEN];
	char resbuf[MAX_LEN];
	char resbuf2[MAX_LEN];
	int error;

	while(1)
	{
		pthread_mutex_lock(&data_mutex);
		h = todo->pop();

		if(h)
		{
			resolving->add(h);
			DEBUG(if(__getIp(h->host))
				printf(">>> DOUBLE RESOLVE of %s", h->host));
			strncpy(resbuf, h->host, MAX_LEN);
			strncpy(resbuf2, h->host, MAX_LEN);
			pthread_mutex_unlock(&data_mutex);

			if(die)
				break;

			*buf4 = '\0';
			*buf6 = '\0';

			if(!gethostbyname2_r(resbuf, AF_INET, &ret, buf4, MAX_LEN, &retptr, &error) && retptr)
			{
				inet_ntop(AF_INET, ret.h_addr, buf4, MAX_LEN);
				DEBUG(printf(">>> %s resolved to: %s !!!\n", h->host, buf4));
			}
			else
				*buf4 = '\0';

			if(!gethostbyname2_r(resbuf, AF_INET6, &ret, buf6, MAX_LEN, &retptr, &error) && retptr)
			{
				inet_ntop(AF_INET6, ret.h_addr, buf6, MAX_LEN);
				DEBUG(printf(">>> %s resolved to: %s !!!\n", h->host, buf6));

			}
			else
				*buf6 = '\0';

			if(die)
				break;

			pthread_mutex_lock(&data_mutex);
			if(*buf4 || *buf6)
				cache->add(new host2ip(resbuf, buf4, buf6));
			//else
			//	DEBUG(printf(">>> unknown host: %s\n", h->host));

			host2resolv h2(resbuf2);
			resolving->remove(h2);
			pthread_mutex_unlock(&data_mutex);
		}
		else
		{
			pthread_mutex_unlock(&data_mutex);
			
			pthread_mutex_lock(&condition_mutex);
			pthread_cond_wait(&condition, &condition_mutex);
			
		}
		if(die)
			break;
	}
}

void *__adns_work(void *me)
{
	((adns_pthread *) me) -> work();

	pthread_exit(NULL);
}

adns_pthread::adns_pthread(int n)
{
	poolSize = 0;
	th = NULL;
	die = false;

	pthread_mutex_init(&data_mutex, NULL);
    pthread_mutex_init(&condition_mutex, NULL);
    pthread_cond_init(&condition, NULL);

	todo = NULL;
	cache = NULL;
	resolving = NULL;
	setupPool(n);
}

void adns_pthread::setupPool(int n)
{
	if(n > 0)
	{
		pthread_attr_t attr;

		pthread_attr_init(&attr);
		pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

		th = new pthread_t[n];

		poolSize = n;

		todo = new hashlist<host2resolv>(1024);
		todo->removePtrs();
		cache = new hashlist<host2ip>(4096);
		cache->removePtrs();
		resolving = new hashlist<host2resolv>(n * 4);
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
	else if(n == 0)
		killThreads();
}

adns_pthread::~adns_pthread()
{
	//killThreads();
}

#ifdef HAVE_DEBUG
void adns_pthread::display()
{
	pthread_mutex_lock(&data_mutex);
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
	pthread_mutex_unlock(&data_mutex);
}
#endif

void adns_pthread::expire(time_t t, time_t now)
{
	pthread_mutex_lock(&data_mutex);
	DEBUG(printf("[D] adns_pthread::expire start\n"));
	if(cache)
		cache->expire(t, now);
	DEBUG(printf("[D] ands::expire end\n"));
	pthread_mutex_unlock(&data_mutex);
}

void adns_pthread::removePool()
{
	//inform all threads that they have to die
	for(int i=0; i<poolSize; ++i)
	{
		DEBUG(printf("[*] AsyncDNS: joining thread %d\n", i));

		pthread_mutex_lock(&condition_mutex);
		die = 1;
		pthread_cond_broadcast(&condition);
		pthread_mutex_unlock(&condition_mutex);
		
		if(pthread_join(th[i], NULL))
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

void adns_pthread::killThreads()
{
	for(int i=0; i<poolSize; ++i)
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

	poolSize = 0;
}
#endif
