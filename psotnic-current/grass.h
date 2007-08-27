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

#ifndef PKS_GRASS_H
#define PKS_GRASS_H 1

#include "ptrlist.h"

template <typename T, typename P=ptrlist<T>, typename Q=ptrlist<P> >
class grouplist
{
	public:
	typedef iterator_type<P> iterator;

	private:
	Q data;

	int ent;
	bool _removePtrs;

	////////////////////////////////
	P *addGroup()
	{
		P *p = new P;
		if(_removePtrs) p->removePtrs();
		data.add(p);
		return p;
	}
	P *addGroupLast()
	{
		P *p = new P;
		if(_removePtrs) p->removePtrs();
		data.addLast(p);
		return p;
	}

	public:
	////////////////////////////////
	iterator begin() const { return iterator(data.begin()); };

	////////////////////////////////
	int expire(time_t t, time_t now)
	{
		typename Q::iterator q, p = data.begin();

		while(p)
		{
			q = p;
			q++;
			if(!p->expire(t, now))
				data.removeLink(p);
			p = q;
		}
		return data.entries();
	}
	///////////////////////////////
	void removePtrs()
	{
		_removePtrs = 1;
		data.removePtrs();
	}
	////////////////////////
	grouplist()
	{
		_removePtrs = 0;
	}
	///////////////////////////////////
	/*
	typename Q::link *start()
	{
		return data.start();
	}
	*/
	///////////////////////////////////
	P *findGroup(T *ptr)
	{
		typename Q::iterator g = data.begin();
		while(g)
		{
			if(g->begin().obj() & *ptr)
				return g;
			g++;
		}
		return NULL;
	}
	///////////////////////////////////
	T *find(T *ptr)
	{
		P *g = findGroup(ptr);
		if(g) return g->find(ptr);
		else return NULL;
	}
	//////////////////////////////////
	T *find(T &ptr)
	{
		P *g = findGroup(&ptr);
		if(g) return g->find(ptr);
		else return NULL;
	}
	////////////////////////////////////
	int addLast(T *ptr)
	{
		P *p = findGroup(ptr);
		if(!p) p = addGroupLast();
		p->addLast(ptr);
		return p->entries();
	}
	////////////////////////////////////
	int add(T *ptr)
	{
		P *p = findGroup(ptr);
		if(!p) p = addGroup();
		p->add(ptr);
		return p->entries();
	}
	////////////////////////////////////
	int sortAdd(T *ptr)
	{
		P *p = findGroup(ptr);
		if(!p) p = addGroup();
		p->sortAdd(ptr);
		return p->entries();
	}
	////////////////////////////////////
	int remove(T *ptr)
	{
		P *g = findGroup(ptr);
		if(g)
		{
			g->remove(ptr);
			if(!g->entries())
			{
				data.remove(g);
				return -1;
			}
			return 1;
		}
		return 0;
	}
	///////////////////////////////
	int remove(T &obj)
	{
		P *g = findGroup(&obj);
		if(g)
		{
			g->remove(obj);
			if(!g->entries())
			{
				data.remove(g);
				return -1;
			}
			return 1;
		}
		return 0;
	}

};
#endif
