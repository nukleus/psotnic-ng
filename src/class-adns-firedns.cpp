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

#ifdef HAVE_ADNS_FIREDNS

#include "prots.h"
#include "global-var.h"

bool adns_firedns::shouldWeCheck()
{
	return (NOW != last_check);
}

int adns_firedns::fillFDSET(fd_set *set)
{
	if(!shouldWeCheck())
		return 0;
	
	int max = 0;

	ptrlist<host2resolv> *cont = resolving->getContainer();
	int n = resolving->getContainerSize();	

	for(int i=0; i<n; ++i)	
	{
		ptrlist<host2resolv>::iterator p = cont[i].begin();
		
		while(p)
		{
			DEBUG(assert(p->fd > 0));
			FD_SET(p->fd, set);
			max = MAX(p->fd, max);
			p++;
		}
	}
	
	return max;
}

void adns_firedns::processResultSET(fd_set *set)
{
	if(!shouldWeCheck())
		return;

	last_check = NOW;

	ptrlist<host2resolv> *cont = resolving->getContainer();
	int n = resolving->getContainerSize();

	struct in_addr addr4;
	struct in6_addr addr6;
	char *m;

	for(int i=0; i<n; ++i)
	{
		ptrlist<host2resolv>::iterator q, p = cont[i].begin();

		while(p)
		{
			q = p;
			p++;

			DEBUG(assert(q->fd > 0));
			if(FD_ISSET(q->fd, set))
			{
				m = firedns_getresult(q->fd);
				if(m)
				{
					host2ip *h = __getIp(q->host);
					if(!h)
					{
						h = new host2ip(q->host);
						cache->add(h);
					}
					
					switch(q->type)
					{
						case 4:
							memcpy(&addr4,m,sizeof(struct in_addr));
							free(h->ip4);	
							h->ip4 = strdup(firedns_ntoa4(&addr4));
							break;
						case 6:
							memcpy(&addr6,m,sizeof(struct in6_addr));
							free(h->ip6);
							h->ip6 = strdup(firedns_ntoa6(&addr6));
							break;
						default:
							DEBUG(printf("[D] FIREDNS :: processResult :: unknown type : %s\n", q->host));
							break;
					}
				}
				resolving->remove(q);
			}
		}
	}
}

adns::host2ip *adns_firedns::getIp(const char *host)
{
	return __getIp(host);
}


void adns_firedns::resolv(const char *host)
{
	DEBUG(printf(">>> Attempting to resolv: %s\n", host));
	
	if(!__getIp(host))
	{
		host2resolv *h = new host2resolv(host);
		if(resolving->find(*h))
		{
			DEBUG(printf(">>> %s is already RESOLVING\n", host));
			delete h;
			return;
		}

		h->fd = firedns_getip4(host);
		if(h->fd != -1)
		{
			h->type = 4;
			resolving->add(h);
			h = new host2resolv(host);
		}
		
		h->fd = firedns_getip6(host);
		if(h->fd != -1)
		{
			h->type = 6;
			resolving->add(h);
		}
		else
			delete h;
	}
}

adns_firedns::adns_firedns()
{
	cache = new hashlist<host2ip>(4096);
	cache->removePtrs();
	resolving = new hashlist<host2resolv>(4096);
	resolving->removePtrs();

	last_check = NOW;
}

adns_firedns::~adns_firedns()
{
	delete cache;
	delete resolving;
}

#ifdef HAVE_DEBUG
void adns_firedns::display()
{
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
}
#endif

void adns_firedns::expire(time_t t, time_t now)
{
	DEBUG(printf("[D] adns_firedns::expire start\n"));
	if(cache)
		cache->expire(t, now);
	DEBUG(printf("[D] ands_firedns::expire end\n"));
}

#endif
