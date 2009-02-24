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

#ifndef PKS_HASHLIST_H
#define PKS_HASHLIST_H 1

#include "ptrlist.h"
#include "pstring.h"

template <class T>
class hashlist
{
	ptrlist<T> *_cont;
	int _n;

	ptrlist<T> &cont(const T *p)
	{
		return _cont[p->hash32() % _n];
	}

	ptrlist<T> &cont(const T &p)
	{
		return _cont[p.hash32() % _n];
	}

	public:
	
	ptrlist<T> *getContainer()
	{
		return _cont;
	}

	int getContainerSize()
	{
		return _n;
	}

	void expire(time_t t, time_t now)
	{
		for(int i=0; i<_n; ++i)
			_cont[i].expire(t, now);
	}
	void removePtrs()
	{
		for(int i=0; i<_n; ++i)
			_cont[i].removePtrs();
	}
	/////////////////////////////////////
	hashlist(int n)
	{
		_cont = new ptrlist<T>[n];
		_n = n;
	}
	/////////////////////////////////////
	~hashlist()
	{
		delete [] _cont;
	}

	/////////////////////////////////////////
	typename ptrlist<T>::iterator find(const T &obj)
	{
		return cont(obj).find(obj);
	}

	/////////////////////////////////////////
	typename ptrlist<T>::iterator find(const T *obj)
	{
		return cont(obj).find(obj);
	}

	/////////////////////////////////////////
	int removeLink(typename ptrlist<T>::link *p, bool rm=1)
	{
		return cont(p->ptr()).removeLink(p);
	}

	/////////////////////////////////////////
	int remove(T &obj, bool rm=1)
	{
		return cont(obj).remove(obj, rm);
	}

	/////////////////////////////////////////
	int remove(T *obj, bool rm=1)
	{
		return cont(obj).remove(obj, rm);
	}

	/////////////////////////////////////////
	void add(const T *ptr)
	{
		cont(ptr).add(ptr);
	}

	/////////////////////////////////////////
	T *pop()
	{
		for(int i=0; i<_n; ++i)
		{
			typename ptrlist<T>::iterator a = _cont[i].begin();
			if(a)
			{
				T *ptr = a;
				_cont[i].removeLink(a, 0);
				return ptr;
			}
		}
		return NULL;
	}
	void clear()
	{
		for(int i=0; i<_n; ++i)
			_cont[i].clear();
	}

	void stats(int &min, double &avg, int &max, int &sum)
	{
		min = 0;
		max = 0;
		sum = 0;

		for(int i=0; i<_n; ++i)
		{
			int n = _cont[i].entries();
			if(n < min)
				min = n;
			else if(n > max)
				max = n;
			sum += n;
		}

		avg = (double) sum/(double) _n;
	}
#ifdef HAVE_DEBUG
	void display()
	{
		for(int i=0; i<_n; ++i)
			printf("cont[%d]: %d\n", i, _cont[i].entries());
	}

	void displayStats()
	{
		int min, max, sum;
		double avg;
		stats(min, avg, max, sum);
		printf(">>> hashtable statistics (min/avg/max/total): %d/%g/%d/%d\n", min, (double) sum/(double) _n, max, sum);
	}

#endif
};
#endif
