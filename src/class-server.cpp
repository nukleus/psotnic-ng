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

Server::Server()
{
    name=NULL;
    version=NULL;
    usermodes=NULL;
    chanmodes=NULL;
    isupport.server=this;
}

Server::~Server()
{
   reset();
}

Server::Isupport::Isupport()
{
   chan_status_flags=NULL;
   chanmodes=NULL;
   maxchannels=0;
   maxlist=0;
   max_kick_targets=0;
   max_who_targets=0;
   max_mode_targets=0;
}

Server::Isupport::~Isupport()
{
   reset();
}

void Server::Isupport::insert(const char *key, const char *value)
{
    if(!key || !*key)
        return;

    isupport_map.insert(std::pair<std::string, std::string>(key, value?value:""));
}

const char *Server::Isupport::find(const char *key)
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

void Server::Isupport::reset()
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

    maxchannels=0;
    maxlist=0;
    max_kick_targets=0;
    max_who_targets=0;
    max_mode_targets=0;

    isupport_map.clear();
}

/** This function will be executed every time when we received a 005 line.
 * \author patrick <patrick@psotnic.com>
 */

void Server::Isupport::init()
{
    const char *p1;
    char *p2;

    if(!chan_status_flags && (p1=find("PREFIX")))
    {
        chan_status_flags=strdup(p1+1);

        if((p2=strchr(chan_status_flags, ')')))
            *p2='\0';
    }

    if(!chanmodes && (p1=find("CHANMODES")))
        chanmodes=strdup(p1);

    if(!maxchannels)
    {
        if((p1=find("CHANLIMIT")))
        {
            if((p2=strchr(p1, ':')))
            {
                *p2++;

                if(p2)
                    maxchannels=atoi(p2);
            }
        }

        else if((p1=find("MAXCHANNELS")))
            maxchannels=atoi(p1);
    }

    if(!maxlist)
    {
        if((p1=find("MAXLIST")))
        {
            if((p2=strchr(p1, ':')))
            {
                *p2++;

                if(p2)
                    maxlist=atoi(p2);
            }
        }

        else if((p1=find("MAXBANS")))
            maxlist=atoi(p1);
    }

    max_kick_targets=1;
    max_who_targets=1;
    max_mode_targets=1;

    if((p1=find("TARGMAX")))
    {
        /* The TARGMAX parameter specifies the maximum number of targets
         * allowable for commands which accept multiple targets.
         * The server MUST specify all commands available to the user which
         * support multiple targets.
         *
         * If the number is not specified for a particular command, then the
         * command does not have a limit on the number of targets.
         *
         * If the TARGMAX parameter is not advertised, or a value is not sent
         * then a client SHOULD assume that no commands except the "JOIN" and
         * "PART" commands accept multiple parameters.
         */

        while(*p1)
        {
           if(!strncasecmp(p1, "KICK:", 5))
           {
              max_kick_targets=atoi(p1+5);

              if(max_kick_targets <= 0)
                  max_kick_targets=30;
           }

           else if(!strncasecmp(p1, "WHO:", 4))
           {
              max_who_targets=atoi(p1+4);

              if(max_who_targets <= 0)
                  max_who_targets=30;
           }

           else if(!strncasecmp(p1, "MODE:", 5))
           {
               max_mode_targets=atoi(p1+5);

               if(max_mode_targets > 4 || max_mode_targets <= 0)
                   max_mode_targets=4;
           }

           p1=strchr(p1, ',');

           if(p1 != NULL)
               p1++;

           else
               break;
         }
    }

    else
    {
        // lame IRCnet check

        p1=find("NETWORK");

        if((p1 && !strcasecmp(p1, "ircnet"))
           || match("2.*", server->version))
        {
            // hardcoded IRCnet defaults
            max_kick_targets=4;
            max_who_targets=11; // MAXPENALTY + 1
            max_mode_targets=4; // there is no limit actually
        }
    }
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

        isupport.reset();
}
