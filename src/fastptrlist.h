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

#ifndef PKS_FASTPTRLIST_H
#define PKS_FASTPTRLIST_H 1

#include "iterator.h"
#include "tiny_ptrlist.h"

template <typename T>
class fastptrlist
{
	private:
	typedef typename ptrlist<T>::iterator iterator;
	typedef typename tiny_ptrlist<T>::link linkT;
	ptrlist<T> pl;
	tiny_ptrlist<linkT> *c;
	int n;

	tiny_ptrlist<linkT> &cont(const T *p) const	{ return c[p->hash32() %n]; };
	tiny_ptrlist<linkT> &cont(const T &p) const	{ return c[p.hash32() % n]; };

	public:
	fastptrlist(int num=512)
	{
		c = new tiny_ptrlist<linkT>[num];
		n = num;
	}

	~fastptrlist()				{ delete [] c; };
	void removePtrs()			{ pl.removePtrs(); }
	int entries() const			{ return pl.entries(); };
	iterator begin() const			{ return pl.begin(); };
	iterator getItem(const int num) const	{ return pl.getItem(num); };
	iterator find(const T &obj) const
	{
		typename tiny_ptrlist<linkT>::iterator i = cont(obj).begin();
		while(i)
		{
			if(*i->ptr() == obj)
				return iterator(i);
			i++;
		}
		return iterator(NULL);
	}
	iterator find(const T *ptr) const
	{
		typename tiny_ptrlist<linkT>::iterator i = cont(ptr).begin();
		while(i)
		{
			if(i->ptr() == ptr)
				return iterator(i);
			i++;
		}
		return iterator(NULL);
	}
	void removeLink(linkT *p, bool rm=1)
	{
		tiny_ptrlist<linkT> *l = &cont(p->ptr());
		typename tiny_ptrlist<linkT>::iterator i = l->begin();
		while(i)
		{
			if(i == p)
			{
				l->removeLink(i);
				pl.removeLink(p, rm);
				break;
			}
			i++;
		}
		assert(i);
	}
	int remove(T *ptr, bool rm=1)
	{
		tiny_ptrlist<linkT> *l = &cont(ptr);
		typename ptrlist<linkT>::iterator i = l->begin();
		while(i)
		{
			if(i->ptr() == ptr)
			{
				linkT *p = i.lnk()->ptr();
				l->removeLink(i.lnk());
				pl.removeLink(p, rm);
				return 1;
			}
			i++;
		}
		return 0;
	}
	iterator sortAdd(const T *ptr)
	{
		iterator i = pl.sortAdd(ptr);
		if(i)
		{
			cont(ptr).add(i);
			return i;
		}
		else
			return iterator(NULL);
	}
	iterator add(const T *ptr)
	{
		iterator i = pl.add(ptr);
		cont(ptr).add(i);
		return i;

	}
	iterator addLast(const T *ptr)
	{
		iterator i = pl.addLast(ptr);
		cont(ptr).add(i);
		return i;
	}
	void clear()
	{
		pl.clear();
		for(int i=0; i<n; ++i)
			c[i].clear();
	}
	void stats(int &min, double &avg, int &max, int &sum)
	{
		min = max = sum = 0;
		avg = 0;

		int num;

		for(int i=0; i<n; ++i)
		{
			iterator_type<linkT> it = c[i].begin();
			for(num=0; it; num++, it++) ;

			if(num < min)
				min = num;
			else if(num > max)
				max = num;
			sum += num;
		}
		avg = (double) sum/(double) n;
	}

#ifdef HAVE_DEBUG
	void display()
	{
		pl.display();
		displayStats();
	}

	void displayStats()
	{
		int min, max, sum;
		double avg;

		stats(min, avg, max, sum);
		printf(">>> hashtable statistics (min/avg/max/total): %d/%g/%d/%d\n", min, (double) sum/(double) n, max, sum);
	}
#endif
};
#endif
