/***************************************************************************
 *   Copyright (C) 2008 by Stefan Valouch                                  *
 *   stefanvalouch@googlemail.com                                          *
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

/*! Reads a string from a terminal and sets a pstrings value to it.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param var The pstring to set.
 * \param defaultValue The default to use as pstring does not have that.
 * \return true if a value was set, even if it is \e defaultValue. false if not set (i.e. if no
 * default and nothing entered.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
bool readUserInput( const char *prompt, pstring<> &var, const char *defaultValue )
{
	string buf;
	do
	{
		if(strlen(defaultValue))
		{
			printPrompt( "%s [%s]: ", prompt, defaultValue );
		}
		else
		{
			printPrompt( "%s: ", prompt );
		}
		getline(cin, buf);
		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() >= 1)
		{
			var = buf.c_str();
			return true;
		}
		else if (strlen(defaultValue))
		{
			var = defaultValue;
			return true;
		}
		else
		{
			continue;
		}
	}
	while( true );
}

/*! Reads a boolean value from a terminal and sets an entities value to that value.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \param force Wether or not to repeat the question till valid data is entered.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entBool &entity, bool force )
{
	string buf;
	options::event *e;
	do
	{
		printPrompt( "%s (ON or OFF) [%s]: ", prompt, (entity.defaultValue ? "ON" : "OFF") );
		getline(cin, buf);
		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() >= 1)
		{
			e = entity.setValue(entity.name, buf.c_str());
			if(e->ok)
			{
				break;
			}
			else if(!force)
			{
				entity.setValue(entity.name, itoa(entity.defaultValue));
				break;
			}
			else
			{
				printError( (const char*)e->reason );
			}
		}
	}
	while( force );
	//return false;
}

/*! Reads an integer value from stdin and sets an entities value to it.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \param force Wether or not to repeat the question till valid data is entered.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entInt &entity, bool force )
{
	string buf;
	options::event *e;

	do
	{
		if( entity.defaultValue )
		{
			printPrompt( "%s (%d to %d) [%d]: ", prompt, entity.min, entity.max, entity.defaultValue );
		}
		else
		{
			printPrompt( "%s (%d to %d): ", prompt, entity.min, entity.max );
		}
		getline(cin, buf);

		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		e = entity.setValue(entity.name, buf.c_str());
		if(e->ok)
		{
			break;
		}
		else if(!force)
		{
			entity.setValue(entity.name, itoa(entity.defaultValue));
			return;
		}
		else
		{
			printError( (const char*)e->reason );
		}
	}
	while ( true );
}

/*! Read a host address from stdin and set an entities value to it.
 * \overload
 * \param prompt The question to ask for the value.
 * \param entity The entits.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entHost &entity )
{
	string buf;
	do
	{
		if(strcmp((const char*)entity.ip, "0.0.0.0"))
		{
			printPrompt( "%s [%s]: ", prompt, (const char*)entity.ip );
		}
		else
		{
			printPrompt( "%s: ", prompt );
		}
		getline(cin, buf);
		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		options::event *e = entity.setValue(entity.name, buf.c_str());
		if(!e->ok)
		{
			printError( (const char*)e->reason );
		}
		else
		{
			break;
		}
	}
	while( true );
}

/*! Reads an IP Port Passwort Handle combination from stdin and sets an entities value to it.
 * \todo Implement checks!!!
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entHub &entity )
{
	string buf;
	options::event *e;
	entHost    host   = entHost( "tmp" );
	entInt     port   = entInt( "tmp", 1, 65535, 0 );
	entMD5Hash pass   = entMD5Hash( "tmp" );
	entWord    handle = entWord( "tmp" );

	printMessage( "%s: ", prompt );
	do
	{
		readUserInput( "\tHost", host );
		readUserInput( "\tPort", port );
		readUserInput( "\tPassword", pass );
		readUserInput( "\tHandle", handle );

		char buf[MAX_LEN];
		snprintf( buf, MAX_LEN, "%s %s %s %s", (const char*)host.ip, port.getValue(), pass.getHash(), (const char*)handle.string );
		e = entity.setValue( entity.name, buf );
		if( !e->ok )
		{
			printError( (const char*)e->reason );
		}
		else
		{
			break;
		}
	}
	while( true );
}

/*! Reads a md5 string from stdin.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entMD5Hash &entity )
{
	string buf;
	do
	{
		printPrompt( "%s: ", prompt );
		getline(cin, buf);

		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() < 8) // password length okay?
		{
			printError( "Passwords should have a length of at least 8 characters!" );
			//cin.clear();
			//cin.get();
		}
		else // okay, throw it in
		{
			entity.setValue(entity.name, buf.c_str());
			break;
		}
	}
	while( true );
}

/*! Asks a user for information to build an entServer object.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entServer &entity )
{
	string buf;
	options::event *e;
	entHost host = entHost( "tmp" );
	entInt  port = entInt( "tmp", 1, 65535, 0 );
	entWord pass = entWord( "tmp" );

	printMessage( "%s: ", prompt );
	do
	{
		readUserInput( "\tHost address", host );
		readUserInput( "\tPort number", port );
		readUserInput( "\tPassword", pass );
		
		e = entity.set( entity.name, (const char*)host.ip, port.getValue(), (const char*)pass.string );
		if( !e->ok )
		{
			printError( (const char*)e->reason );
		}
		else
		{
			break;
		}
			
	}
	while( true );
}

/*! Reads a string value from stdin and sets an entities value to it.
 * If the \e entity has no default value, the user is forced to enter something.
 * \overload
 * \param prompt The question used to ask for the value.
 * \param entity The entity.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void readUserInput( const char *prompt, entString &entity )
{
	string buf;
	do
	{
		if(entity.defaultString)
		{
			printPrompt( "%s (%d to %d chars) [%s]: ", prompt, entity.min, entity.max,
				(const char*)entity.defaultString );
		}
		else
		{
			printPrompt( "%s (%d to %d chars): ", prompt, entity.min, entity.max );
		}
		getline(cin, buf);

		if(!cin.good())
		{
			cout << endl;
			exit(1);
		}
		if(buf.size() < 1)
		{
			if(entity.defaultString)
			{
				entity.setValue(entity.name, entity.defaultString);
				break;
			}
			continue;
		}
		entity.setValue(entity.name, buf.c_str());
		break;
	}
	while( true );
}

/*! Asks a multiple choice question and returns the answer-id.
 * \note Be sure that \e defChoice is whithin the range of \e choices.
 * \param prompt The question to ask.
 * \param choices A descriptive text to make the decision easier.
 * \param defChoice The ID of the default value.
 * \return The selected ID or the default if nothing (valid) was entered.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
int readUserMC( const char *prompt, const char *choices[], unsigned int defChoice )
{
	string buf;
	printMessage( "%s:", prompt );
	for( unsigned int i = 0; i < sizeof(choices)-1; i++ )
	{
		if(i == defChoice)
		{
			printItem( "\t[%d]\t%s", i, choices[i] );
		}
		else
		{
			printItem( "\t %d \t%s", i, choices[i] );
		}
	}
	printPrompt( "Your choice: " );
	getline(cin, buf);

	if(!cin.good())
	{
		cout << endl;
		exit(1);
	}
	if(buf.size() < 1)
	{
		return defChoice;
	}
	else
	{
		unsigned int inp = atoi(buf.c_str());
		for( unsigned int i = 0; i < sizeof(choices); i++ )
		{
			if(i == inp)
				return inp;
		}
		return defChoice;
	}
}

/*! Asks the user a simple Yes/No question.
 * \param prompt The question.
 * \param defaultValue The default value.
 * \return true if the user entered yes, false if no.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
bool readUserYesNo( const char *prompt, bool defaultValue )
{
	string buf;
	printPrompt( "%s (Y or N) [%s]: ", prompt, (defaultValue ? "Y" : "N") );
	getline(cin, buf);
	if(buf.size() < 1)
	{
		return defaultValue;
	}
	if(buf == "Y" || buf == "y" || buf == "Yes" || buf == "yes" || buf == "1")
	{
		return true;
	}
	return false;
}

/*! Interactive generator for config files.
 * This function asks the user for the data and generates the initial config without needing to have
 * the unencrypted config file laying around waiting for encryption.
 * \author Stefan Valouch <stefanvalouch@googlemail.com>
 */
void createInitialConfig()
{
	printMessage( "Bot now runs in config creation mode" );
	printMessage( "The options are explained at http://psotnic.wiki.sourceforge.net/Config+File" );
	printMessage( "Mandatory options for all bot types:" );

	readUserInput( "Which nick should the bot use?", config.nick );
	readUserInput( "What should be used as the bots realname?", config.realname );
	readUserInput( "Which characters should get appended to the nick if it is already taken?", config.nickappend );
	readUserInput( "Nick to use when the normal nick is already taken:", config.altnick );

	readUserInput( "Enter the bots IPv4 address, use 0.0.0.0 for any", config.myipv4 );

	const char *choices[3] = {
		"Main",
		"Slave",
		"Leaf"
	};
	int bottype = readUserMC( "What type of bot do you want?", choices, 0 );

	switch( bottype )
	{
		case 0:
			printMessage( "Configuration for bot type: MAIN" );
			config.bottype = BOT_MAIN;
			readUserInput( "What port should the bot listen on for connections from owners and slaves?", config.listenport );
#ifdef HAVE_SSL
			readUserInput( "What port should the bot listen on for SSL encrypted connections from owners and slaves?", config.ssl_listenport );
#endif
			readUserInput( "Please enter a password for connecting to partyline.", config.ownerpass );
			break;
		case 1:
			printMessage( "Configuration for bot type: SLAVE" );
			config.bottype = BOT_SLAVE;
			readUserInput( "What port should the bot listen on for connections from leafs?", config.listenport );
#ifdef HAVE_SSL
			readUserInput( "What port should the bot listen on for SSL encrypted connections from leafs?", config.ssl_listenport );
#endif
			readUserInput( "Please enter the connection options for connecting to the main bot.", config.hub );
			
			break;
		case 2:
			printMessage( "Configuration for bot type: LEAF" );
			config.bottype = BOT_LEAF;
			readUserInput( "Connection options to connect to slave?", config.hub );
			break;
		default:
			printError( "ERROR: unknown bot type: " + bottype );
			exit(1);
			break;
	}

	string buf;
	options::event *e;
	int n;

	// we don't make a function for that entMult stuff, its easier this way!
	if( config.bottype == BOT_LEAF )
	{
		if( readUserYesNo( "Do you want to add alternative hubs?", false ) )
		{
			printMessage( "Valid lines are {}" );
			for( n = 0; n < MAX_ALTS; )
			{
				printPrompt( "\tAltHub %d: ", n );
				getline(cin, buf);
				if(!cin.good())
				{
					cout << endl;
					exit(1);
				}
				if( buf.size() < 1 )
				{
					break;
				}
				e = config.alt[n].setValue( config.alt[n].name, buf.c_str() );
				if( !e->ok )
				{
					printError( (const char*)e->reason );
					continue;
				}
				n++;
			}
			printMessage( "Okay, added %d alternative hubs.", n );
		}
	}

	if( readUserYesNo( "Do you want to add some servers?", true ) )
	{
		printMessage( "Valid lines are: \"1.2.3.4 6667\" or \"1.2.3.4 6667 password\"" );
		for( n = 0; n < MAX_SERVERS; )
		{
			printPrompt( "\tServer %d: ", n );
			getline(cin, buf);
			if(!cin.good())
			{
				cout << endl;
				exit(1);
			}
			if( buf.size() < 1 )
			{
				break;
			}
			e = config.server[n].setValue( config.server[n].name, buf.c_str() );
			if( !e->ok )
			{
				printError( (const char*)e->reason );
				continue;
			}
			n++;
		}
		printMessage( "Okay, added %d servers.", n );
	}

	if( readUserYesNo( "Do you want to add some modules?", false ) )
	{
		for( n = 0; n < MAX_MODULES; )
		{
			printPrompt( "\tModule %d: ", n );
			getline(cin, buf);
			if(!cin.good())
			{
				cout << endl;
				exit(1);
			}
			if(buf.size() < 1)
			{
				break;
			}
			e = config.module_load[n].setValue(config.module_load[n].name, buf.c_str());
			if(!e->ok)
			{
				printError( (const char*)e->reason );
				continue;
			}
			n++;
		}
		printMessage( "Okay, added %d modules.", n );
	}

	if( readUserYesNo( "Do you want to set up the very basic and optional stuff?", false ) )
	{
		printMessage( "Detailed configuration" );
		readUserInput( "What is the bots username?", config.ident );

		config.handle.defaultString = config.nick.string;
		readUserInput( "What is the bots partyline handle?", config.handle );
		// vhost
		// logfile TODO: implement {partially done in branch}
		//readUserInput( "Path where psotnics logfiles should be stored to", config.logfile );
		//readUserInput( "Loglevel for psotnics actions", config.loglevel );
		config.userlist_file.defaultString = config.nick.string + ".ul";
		readUserInput( "Where should the bot place its userlist?", config.userlist_file );

		readUserInput( "Which ctcp version do you want? 0 = none, 1 = psotnic, 2 = irssi, 3 = epic, 4 = lice, 5 = bitchx, 6 = dzony loker, 7 = luzik, 8 = mirc 6.14", config.ctcptype );

		readUserInput( "Disallow forking?", config.dontfork );
		readUserInput( "Keepnick?", config.keepnick );
		readUserInput( "Kick reason used for most kicks", config.kickreason );
		readUserInput( "Kick reason used when someone overrides the channel limit", config.limitreason );
		readUserInput( "Kick reason for keepout setting", config.keepoutreason );
		readUserInput( "Part reason", config.partreason );
		readUserInput( "Quit reason", config.quitreason );
		readUserInput( "Cycle reason", config.cyclereason );
		// bnc
		// bouncer
		readUserInput( "Botnetword, has to be equal on all bots in the same botnet", config.botnetword );

#ifdef HAVE_ADNS
		readUserInput( "How many resolve threads should be used?", config.resolve_threads );
		readUserInput( "Domain Time-To-Live?", config.domain_ttl );
#endif

		readUserInput( "Check shit on nick change?", config.check_shit_on_nick_change );
	}
	
	string defaultcfgfile = string(choices[bottype]) + "_" +  config.nick.getValue() + ".cfg";
	readUserInput( "Where should the config file be saved?", config.file, defaultcfgfile.c_str()); // TODO: use confdir as default path once make-install is merged
	e = config.save();
	if (!e->ok)
	{
		printError( (const char*)e->reason );
		exit(1);
	}
	printMessage( (const char*)e->reason );
	exit(0);
}

