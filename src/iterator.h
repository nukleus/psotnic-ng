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

#ifndef PKS_ITERATOR_H
#define PKS_ITERATOR_H 1

template <typename T>
class link_type
{
	public:
	T *_ptr;
	link_type *_next;
	link_type *_prev;

	link_type *next()				{ return _next; };
	T *&ptr()						{ return _ptr; };
	link_type(const T *p)			{ _ptr = const_cast<T*> (p); };
	link_type()						{ };
};

template <typename T>
class iterator_type
{
    private:
	typedef link_type<T> link;
	typedef link* link_ptr;
    typedef T* pointer;
	link *_lnk;

    public:
    iterator_type(link *l=NULL) { _lnk = l; };
    iterator_type operator++(int) { return (_lnk = _lnk->next()); };
    iterator_type operator--(int) { return (_lnk = _lnk->prev()); };
    operator bool() { return _lnk != 0; };
    operator pointer() { return _lnk->ptr(); };
	operator link_ptr() { return _lnk; };
	T *&operator->() { return _lnk->ptr(); };
    T *&operator&() { return _lnk->ptr(); };
    T *&ptr() { return _lnk->ptr(); };
	T &obj() { return *_lnk->ptr(); };
	link *lnk() { return _lnk; };
};


#endif


