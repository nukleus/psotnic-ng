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

#ifndef PKS_TINY_PTRLIST_H
#define PKS_TINY_PTRLIST_H 1

#include "iterator.h"
#include "ptrlist.h"

template <class T>
class tiny_ptrlist
{
	public:
	typedef iterator_type<T> iterator;
	typedef link_type<T> link;

	private:
	link *first;

	public:
	////////////////////////////////////////
	// return interator
	iterator begin() const { return iterator(first); };

	//////////////////////////////////
	// remove link
	void removeLink(link *p)
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
		delete p;
	}

	//////////////////////////////
	// add to begining of a list
	iterator add(const T *ptr)
	{
		link *p;

		if(!first)
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
		return iterator(first);
	}


	//////////////////////
	// constructor
	tiny_ptrlist() : first(NULL) { }

	//////////////////////
	// destructor
	~tiny_ptrlist()
	{
		link *p = first;
		link *q;

		while(p)
		{
			q = p;
			p = p->_next;
			delete(q);
		}
	}

	/////////////////////////////
	// clear all entries
	void clear()
	{
		link *p = first;
		link *q;

		while(p)
		{
			q = p;
			p = p->_next;
			delete(q);
		}
		first = NULL;
	}

#ifdef HAVE_DEBUG
	void display()
	{
		link *p = first;
		while(p)
		{
			p->_ptr->display();
			p = p->_next;
		}
	}
#endif
};

#endif


