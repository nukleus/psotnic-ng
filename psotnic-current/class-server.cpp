/***************************************************************************
 *   Copyright (C) 2008 by patrick                                         *
 *   patrick@psotnic.com                                                   *
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

#include "prots.h"
#include "global-var.h"

void Server::_isupport::insert(const char *key, const char *value)
{
    if(!key || !*key)
        return;

    isupport_map.insert(std::pair<std::string, std::string>(key, value?value:""));
}

const char *Server::_isupport::find(const char *key)
{
    isupportType::const_iterator it=isupport_map.find(key);

    if(it != isupport_map.end())
    {
        if(it->second.length() > 0)
            return it->second.c_str();

        else
            return "";
    }

    else
        return NULL;
}

void Server::_isupport::clear()
{
    if(chan_status_flags)
    {
        free(chan_status_flags);
        chan_status_flags=NULL;
    }

    if(chanmodes)
    {
        free(chanmodes);
        chanmodes=NULL;
    }

    isupport_map.clear();
}

/** This function will be executed every time when we received a 005 line.
 * \author patrick <patrick@psotnic.com>
 */

void Server::_isupport::init()
{
    const char *p1;
    char *p2;

    // get channel status flags from PREFIX which looks like "(ov)@+"

    if(chan_status_flags)
        free(chan_status_flags);

    p1=find("PREFIX");

    if(!p1)
        chan_status_flags=strdup("ov");

    else
    {
        chan_status_flags=strdup(p1+1);

        if((p2=strchr(chan_status_flags, ')')))
            *p2='\0';
    }

    // CHANMODES

    if(chanmodes)
        free(chanmodes);

    p1=find("CHANMODES");

    if(!p1)
        chanmodes=strdup("beIR,k,l,imnpstaqr");

    else
        chanmodes=strdup(p1);
}

/** Clears server information.
 *  \author patrick <patrick@psotnic.com>
 */
void Server::reset()
{
        if(name)
        {
            free(name);
            name=NULL;
        }

        if(version)
        {
            free(version);
            version=NULL;
        }

        if(usermodes)
        {
            free(usermodes);
            usermodes=NULL;
        }

        if(chanmodes)
        {
            free(chanmodes);
            chanmodes=NULL;
        }

        isupport.clear();
}

/** Tells the maximum number of channels a client can join.
 *
 * \author patrick <patrick@psotnic.com>
 * \return see above
 */

int Server::maxchannels()
{
    const char *p1, *p2;

    if((p1=isupport.find("CHANLIMIT")))
    {
        if((p2=strchr(p1, ':')))
        {
            *p2++;

            if(p2)
                return atoi(p2);
        }
    }

    else if((p1=isupport.find("MAXCHANNELS")))
        return atoi(p1);

    return 21;
}

/** Tells the limit of how many "variable" modes of type A a client may set in total on a channel.
 *
 * \author patrick <patrick@psotnic.com>
 * \return see above
 */

int Server::maxlist()
{
    const char *p1, *p2;

    if((p1=isupport.find("MAXLIST")))
    {
        if((p2=strchr(p1, ':')))
        {
            *p2++;

            if(p2)
                return atoi(p2);
        }
    }

    else if((p1=isupport.find("MAXBANS")))
        return atoi(p1);

    return 42;
}

