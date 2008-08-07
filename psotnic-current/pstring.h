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

#ifndef PKS_PSTRING_H
#define PKS_PSTRING_H	1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

template <unsigned int ALIGN=32, unsigned int TOLERANCE=2>
class pstring
{
	private:
	/**
	 * nubmer of allocated bytes - 1 (one is left for NULL string terminator)
	 */
		unsigned int _len;
		char *_data;

		void resize(unsigned int size, bool copy)
		{
			unsigned int s = size / ALIGN;
			unsigned int l = _len / ALIGN;

			int diff = l - s;

			if(diff <= 0 || diff >= (int) TOLERANCE)
			{
				if(copy)
					_data = (char *) realloc(_data, s*ALIGN + ALIGN + 1);
				else
				{
					free(_data);
					_data = (char *) malloc(s*ALIGN + ALIGN + 1);
				}
			}
		}

	public:
	pstring() : _len(0), _data((char *) malloc(ALIGN+1))
	{
		*_data = '\0';
	}

	~pstring()
	{
		free(_data);
	}

	pstring(const char *str) : _len(strlen(str)),
	_data((char *) malloc((_len/ALIGN)*ALIGN + ALIGN + 1))
	{
		strcpy(_data, str);
	}

	pstring(const pstring &s) : _len(s._len)
	{
		_data = (char *) malloc(_len + 1);
		strcpy(_data, s._data);
	};

	void assign(const char *str, const unsigned int len)
	{
		resize(len, false);
		strncpy(_data, str, len);
		_data[len] = '\0';
		_len = len;
	}

	void append(const char *str, const unsigned int len)
	{
		int n = len + _len;
		resize(n, true);
		strcpy(_data + _len, str);
		_len = n;
	}

	pstring &operator=(const pstring &s)
	{
		        assign(s);
			return *this;
	};

	pstring &operator=(const char *str)
	{
		assign(str);
		return *this;
	}

	pstring &operator+=(const pstring &s)
	{
		append(s._data, s._len);
		return *this;
	}

	pstring &operator+=(const char *str)
	{
		append(str);
		return *this;
	}

	operator const char*() const
	{
		return _data;
	}
	operator bool() const
	{
		return _len != 0;
	}
	void assign(const char *str)
	{
		assign(str, strlen(str));
	};
	void assign(const pstring &s)
	{
		assign(s._data, s._len);
	};
	void append(const char *str)
	{
		append(str, strlen(str));
	};
	unsigned int len() const
	{
		return _len;
	};
	char *getCString()
	{
		char *str = _data;
		_len = 0;
		_data = (char *) malloc(1);
		*_data ='\0';
		return str;
	};

};

template <unsigned int A, unsigned int T>
pstring<A, T> operator+(const pstring<A, T> &a, const pstring<A, T> &b)
{
	pstring<A, T> c(a);
	c += b;
	return c;
}

template <unsigned int A, unsigned int T>
pstring<A, T> operator+(const pstring<A, T> &a, const char *b)
{
	pstring<A, T> c(a);
	c += b;
	return c;
}

#endif
