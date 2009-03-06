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

#ifndef MODULE_H
#define MODULE_H 

#include <string>

#include "pstring.h"
#include "ptrlist.h"
#include "defines.h"
#include "class-ent.h"

using std::string;

class chan;
class CHANLIST;
class chanuser;

/*! Create the initial functions needed to load a module.
 * Execute this macro inside a module only.
 */
#define MOD_LOAD( CLASS )						\
extern "C"								\
{									\
	Module *load( void *handle, const char *file,			\
		const char *md5sum, time_t loadDate,			\
		const char *dataDir )					\
	{								\
		return new CLASS( handle, file, md5sum, loadDate,	\
		dataDir );						\
	}								\
	void unload( Module *p )					\
	{								\
		if( p )							\
			delete p;					\
	}								\
	pstring<> compileTime()					\
	{								\
		return __TIME__;					\
	}								\
	pstring<> compileDate()					\
	{								\
		return __DATE__;					\
	}								\
}


/*! Return the module name and description.
 * \param NAME The modules name.
 * \param DESCRIPTION The modules description.
 * \return The description string.
 */
#define MOD_DESC( NAME, DESCRIPTION )		\
extern "C"					\
{						\
	pstring<> name()			\
	{					\
		return NAME;			\
	}					\
	pstring<> description()			\
	{					\
		return DESCRIPTION;		\
	}					\
\
}

/*! bla.
 * \param AUTHOR The authors name.
 * \param EMAIL The authors email.
 */
#define MOD_AUTHOR( AUTHOR, EMAIL )		\
extern "C"					\
{						\
	pstring<> author()			\
	{					\
		return AUTHOR;			\
	}					\
	pstring<> email()			\
	{					\
		return EMAIL;			\
	}					\
}

/*! Returns the modules version.
 * \param VERSION The modules version.
 */
#define MOD_VERSION( VERSION )			\
extern "C"					\
{						\
	pstring<> version()			\
	{					\
		return VERSION;			\
	}					\
}

/*! Module base class.
 * All modules have to inherit from this class in order to be loaded.
 * This class defines a bunch of virtual methods that replace the old hook-system which consists of
 * function pointers.
 */
class Module
{
	public:
	Module( void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir="" );
	virtual ~Module();

	virtual bool onLoad( string &msg );
	virtual void onPrivmsg( const char *from, const char *to, const char *msg );
	virtual void onNotice( const char *from, const char *to, const char *msg );
	virtual void onJoin( chanuser *u, chan *ch, const char *mask, int netjoin );
	virtual void onBotnetcmd( const char *from, const char *cmd );
	virtual void onTimer();
	virtual void onConnecting();
	virtual void onConnected();
	virtual void onDisconnected ( const char *reason );
	virtual void onKlined ( const char *reason );
	virtual void onMode ( chan *ch, const char mode[2][MODES_PER_LINE], const char *arg[MODES_PER_LINE], const char *mask );
	virtual void onCrap ( const char *data );
	virtual void onKick ( chan *ch, chanuser *kicked, chanuser *kicker, const char *msg );
	virtual void onNickChange ( const char *from, const char *to );
	virtual void onInvite( const char *who, const char *channame, chan *chan, CHANLIST *chLst );
	virtual void onRawirc( const char *data );
	virtual void onCtcp( const char *from, const char *to, const char *msg );
	virtual void onChannelSynced( chan *ch) ;
	virtual void onPartylineCmd( const char *from, int flags, const char *cmd, const char *args );
	virtual void onTopicChange( chan *ch, const char *topic, chanuser *u, const char *oldtopic );
	virtual void onPrePart( const char *mask, const char *channel, const char *msg, bool quit );
	virtual void onPostPart( const char *mask, const char *channel, const char *msg, bool quit );
	virtual void onUserlistLoaded();
	virtual void onChanuserConstructor( const chan *ch, chanuser *cu );
	virtual void onReceivedSigHup();
	virtual void onRehash();

	/*! \name CustomData stuff
	 * \{
	 */
	virtual void onNewChan( chan *me );
	virtual void onNewChanuser( chanuser *me );
	virtual void onNewCHANLIST( CHANLIST *me );

	virtual void onDelChan( chan *me );
	virtual void onDelChanuser( chanuser *me );
	virtual void onDelCHANLIST( CHANLIST *me );
	//! \}

	void setAuthor( pstring<> author );
	void setCompileTime( pstring<> date, pstring<> time );
	void setDescription( pstring<> desc );
	void setEmail( pstring<> email );
	void setName( pstring<> name );
	void setVersion( pstring<> version );

	pstring<> author();
	pstring<> compileTime();
	pstring<> compileDate();
	pstring<> dataDir();
	pstring<> description();
	pstring<> email();
	pstring<> file();
	void *handle();
	pstring<> md5sum();
	pstring<> name();
	const time_t *loadDate();
	pstring<> version();

	protected:
	pstring<> m_author;		//!< Module author.
	pstring<> m_compileDate;	//!< Date the module was compiled.
	pstring<> m_compileTime;	//!< Time the module was compiled.
	pstring<> m_dataDir;		//!< Directory where the module may save its data.
	pstring<> m_desc;		//!< Module description.
	pstring<> m_email;		//!< Module author email.
	pstring<> m_name;		//!< Modules name
	pstring<> m_version;		//!< Module version string.
	

	private:
	pstring<> m_file;		//! Modules filename.
	pstring<> m_md5sum;		//! Modules md5 sum.
	time_t m_loadDate;		//! Time the module was loaded.
	void *m_handle;			//! Handle for dl*-commands.
};

extern ptrlist<Module> modules;

#endif /* MODULE_H */
