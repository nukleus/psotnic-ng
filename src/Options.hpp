/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin <grusin@gmail.com>          *
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

#ifndef OPTIONS_HPP
#define OPTIONS_HPP 

#include "pstring.h"
#include "ptrlist.h"

class inetconn;
class ent;

class options
{
	public:
	class event
	{
		public:
		pstring<> reason;
		bool ok;
		bool notFound;
		ent *entity;

		void setOk(ent *e, const char *format, ...);
		void setError(ent *e, const char *format, ...);
		void setError(ent *e);
		void setNotFound(const char *format, ...);
		event();
	};

	ptrlist<ent> list;

	options();

	event *setVariable(const char *var, const char *value);
	const char *getValue(const char *var);
	void sendToOwner(const char *owner, const char *var, const char *prefix);
	bool parseUser(const char *from, const char *var, const char *value, const char *prefix, const char *prefix2="");
	void reset();
	void sendToFile(inetconn *c, pstring<> prefix);

#ifdef HAVE_DEBUG
	void display();
#endif

	protected:
	void registerObject(const ent &e);
	int maxVarLen;
};

extern options::event _event;

#endif /* OPTIONS_HPP */
