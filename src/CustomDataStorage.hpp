/****************************************************************************
 * Copyright (C) 2008-2009 by Stefan Valouch <stefanvalouch@googlemail.com> *
 *                                                                          *
 *   This program is free software; you can redistribute it and/or modify   *
 *   it under the terms of the GNU General Public License as published by   *
 *   the Free Software Foundation; either version 2 of the License, or      *
 *   (at your option) any later version.                                    *
 *                                                                          *
 *   This program is distributed in the hope that it will be useful,        *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *   GNU General Public License for more details.                           *
 *                                                                          *
 *   You should have received a copy of the GNU General Public License      *
 *   along with this program; if not, write to the                          *
 *   Free Software Foundation, Inc.,                                        *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.              *
 ****************************************************************************/

#ifndef CUSTOMDATASTORAGE_HPP
#define CUSTOMDATASTORAGE_HPP 

#include <map>

class CustomDataObject;

using std::map;

/*! Storage place for custom data entries.
 * In order to give the ability to let modules store data inside your class, simply inherit from
 * this one.
 */
class CustomDataStorage
{
	public:
		CustomDataStorage();
		virtual ~CustomDataStorage();

		CustomDataObject *customData( const char *moduleName );
		void setCustomData( const char *moduleName, CustomDataObject *data );
		void delCustomData( const char *moduleName );

	private:
		map< const char *, CustomDataObject * > m_data;		//! Storage.
};

#endif /* CUSTOMDATASTORAGE_HPP */
