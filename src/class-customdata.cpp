/***************************************************************************
 *   Copyright (C) 2008 by Stefan Valouch <stefanvalouch@googlemail.com>   *
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


/* instead of CustomDataConstructor, use HOOK( chanuserConstructor ) */
#include "prots.h"
#include "global-var.h"
#include "classes.h"

/*! Standard constructor */
CustomDataStorage::CustomDataStorage()
{
	// just to be sure.
	m_data.clear();
}

/*! Destructor, cleans up the map. */
CustomDataStorage::~CustomDataStorage()
{
	m_data.clear();
}

/*! Searches an object of type CustomDataObject and returns it, if found.
 * Use this to get access to the data you stored.
 * \param moduleName The name of your module to identify the object.
 * \return Either the found object, or NULL.
 */
CustomDataObject *CustomDataStorage::customData( const char *moduleName )
{
	map< const char *, CustomDataObject * >::iterator it = m_data.find( moduleName );
	if ( it != m_data.end() )
	{
		return (*it).second;
	}
	else
	{
		return NULL;
	}
}

/*! Set custom data for your module.
 * \param moduleName The name of your module, this will always identify the data you stored.
 * \param data The object to store.
 * \return true if inserted, false if already there.
 */
void CustomDataStorage::setCustomData( const char *moduleName, CustomDataObject *data )
{
	if( !data )
		return;

	m_data.insert ( std::pair< const char * , CustomDataObject * >( moduleName, data ) );;
}

/*! Deletes your entry from the map.
 * \param moduleName The name identifying the entry of your object inside the map.
 */
void CustomDataStorage::delCustomData( const char *moduleName )
{
	m_data.erase( moduleName );
}
