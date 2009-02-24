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

#ifndef PKS_PTRLIST_H
#define PKS_PTRLIST_H 1

#include "iterator.h"
//#include "hashlist.h"

template <class T> class ptrlist;

template <class T>
class ptrlist
{
	public:
	typedef iterator_type<T> iterator;
	typedef link_type<T> link;

	private:
	link *first;
	int ent;
	int _removePtrs;

	public:
	////////////////////////////////////////
	int expire(const time_t t, const time_t now)
	{
		link *q, *p = first;
		while(p)
		{
			if(p->ptr()->creation() + t <= now)
			{
				q = p->next();
				removeLink(p);
				p = q;
			}
			else p = p->next();
		}
		return ent;
	}

	////////////////////////////////////////
	int entries() const		{ return ent; };

	////////////////////////////////////////
	// return interator
	iterator begin() const { return iterator(first); };

	////////////////////////////////////////
	// enable deletion of ptr() on removal
	// of a link
	void removePtrs()
	{
		_removePtrs = 1;
	}

	////////////////////////////////////////
	// gets to n-th element
	iterator getItem(const int num) const
	{
		link *p;
		int i;

		if(num + 1 > ent || num < 0) return iterator(NULL);
		p = first;
		i = 0;

		while(p)
		{
			if(i == num) return iterator(p);
			p = p->_next;
			i++;
		}
		return iterator(NULL);
	}

	//////////////////////////////////////////////////
	// finds object
	// to perform such operation one has to create
	// object which will be identical to one you
	// want to remove (in aspect of == operator, ofc)
	// and then pass it as argument to this fucntion
	iterator find(const T &obj) const
	{
		link *p = first;

		while(p)
		{
				if(*p->_ptr == obj)
				return iterator(p);
			p = p->_next;
		}
		return iterator(NULL);
	}

	//////////////////////////////////////
	// find by pointer
	iterator find(const T *ptr) const
	{
		link *p = first;

		while(p)
		{
			if(ptr == p->_ptr)
				return iterator(p);
			p = p->_next;
		}
		return iterator(NULL);
	}

	//////////////////////////////////
	// remove link
	void removeLink(link *p, bool rm=1)
	{
		if(first == p)
		{
			first = first->_next;
			if(first) first->_prev = NULL;
		}
		else
		{
			p->_prev->_next = p->_next;
			if(p->_next) p->_next->_prev = p->_prev;
		}

		--ent;
		if(_removePtrs && rm)
			delete p->ptr();
		delete p;
	}

	/////////////////////////////////
	// remove by object
	int remove(const T &obj, bool rm=1)
	{
		iterator i = find(obj);

		if(i)
		{
			removeLink(i, rm);
			return 1;
		}
		return 0;
	}
	//////////////////////////////
	// remove by pointer
	int remove(T *ptr, bool rm=1)
	{
		iterator i = find(ptr);

		if(i)
		{
			removeLink(i, rm);
			return 1;
		}

		return 0;
	}

	//////////////////////////////////
	// sort add pointer to a list
	iterator sortAdd(const T *ptr)
	{
		link *p, *q;

		if(!ptr)
			return(NULL);

		if(!ent)
		{
			first = new link(ptr);
			first->_next = first->_prev = NULL;
			++ent;
			return(first);
		}
		else
		{
			//if(strcmp(ptr->nick, first->ptr->nick) < 0)
			if(*ptr < *first->_ptr)
			{
				q = new link(ptr);
				first->_prev = q;
				q->_prev = NULL;
				q->_next = first;
				first = q;
				++ent;
				return iterator(q);
			}
			else
			{
				p = first;
				while(1)
				{
					//if(!strcmp(ptr->nick, p->ptr->nick)) return;
					//if(strcmp(ptr->nick, p->ptr->nick) < 0)

					if(*ptr == *p->_ptr) return(NULL);
					if(*ptr < *p->_ptr)
					{
						q = new link(ptr);
						q->_next = p;
						q->_prev = p->_prev;
						p->_prev->_next = q;
						p->_prev = q;
						++ent;
						return iterator(q);
					}
					else if(p->_next == NULL)
					{
						q = new link(ptr);
						q->_next = NULL;
						q->_prev = p;
						p->_next = q;
						++ent;
						return iterator(q);;
					}
					p = p->_next;
				}
			}
		}
	}

	//////////////////////////////
	// add to begining of a list
	iterator add(const T *ptr)
	{
		link *p;

		if(!ent)
		{
			first = new link(ptr);
			first->_next = first->_prev = NULL;
		}
		else
		{
			p = first->_prev = new link(ptr);
			p->_next = first;
			p->_prev = NULL;
			first = p;
		}
		++ent;
		return iterator(first);
	}

	//////////////////////////////////
	// add to the end of a list
	iterator addLast(const T *ptr)
	{
		if(!ent)
		{
			first = new link(ptr);
			first->_next = first->_prev = NULL;
			++ent;
			return iterator(first);
		}
		else
		{
			link *p = first;

			while(p->_next)
				p = p->_next;

			p->_next = new link(ptr);
			p->_next->_prev = p;
			p->_next->_next = NULL;
			++ent;
			return iterator(p->_next);
		}

	}

	//////////////////////
	// constructor
	ptrlist()
	{
		first = NULL;
		ent = 0;
		_removePtrs = 0;
	}

    //////////////////////
	// destructor
	virtual ~ptrlist()
	{
		link *p = first;
		link *q;

		while(p)
		{
			q = p;
			p = p->_next;
			if(_removePtrs) delete q->ptr();
			delete(q);
		}
	}

	/////////////////////////////
	// clear all entries
	void clear()
	{
		this->~ptrlist();

		first = NULL;
		ent = 0;
	}

#ifdef HAVE_DEBUG
	void display()
	{
		link *p = first;
		while(p)
		{
			p->ptr()->display();
			p = p->_next;
		}
	}
#endif
};

#endif


