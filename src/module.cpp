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

#include "prots.h"
#include "global-var.h"

#ifdef HAVE_MODULES

/*! \file
 * Module loading related stuff.
 */

/*! Initialisation constructor.
 * \param handle Handle to use with dl*.
 * \param file Filename of the module.
 * \param md5sum Modules md5 checksum.
 * \param loadDate Time the module was loaded.
 * \param dataDir Directory where the module should put its data in.
 */
Module::Module( void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir )
{
	m_handle = handle;
	m_file = file;
	m_md5sum = md5sum;
	m_loadDate = loadDate;
	m_dataDir = dataDir;
}

/*! Destructor.
 * Place all your cleanup code inside this function.
 */
Module::~Module()
{
}

/*! Function to check if the module was loaded successfully.
 * If the module notices that something is not correctly set, it returns false here and will be unloaded.
 * \param msg Message from the module explaining stuff.
 *\return Wether or not the initialisation was successfull.
 */
bool Module::onLoad( string &msg )
{
	msg = "Not overloaded!";
	return false;
}

/*! Called when a PRIVMSG is received.
 * \param from The sender.
 * \param to The receiver. {special case: ME!}
 * \param msg The message.
 */
void Module::onPrivmsg( const char *from, const char *to, const char *msg )
{
}

/*! Called when a NOTICE is received.
 * \param from The sender.
 * \param to The receiver.
 * \param msg The message.
 */
void Module::onNotice( const char *from, const char *to, const char *msg )
{
}

/*! Called when someone joins a channel in which the bot lays.
 * \todo implement netjoin and netpart.
 * \param u The new user.
 * \param ch The channel which the user joined.
 * \param mask The users mask.
 * \param netjoin true if the user recovered from netsplit.
 */
void Module::onJoin( chanuser *u, chan *ch, const char *mask, int netjoin )
{
}

/*! Called when someone sends a command via the <em>botnet control</em> to this bot.
 * \param from The owners name who send the command.
 * \param cmd The command string.
 */
void Module::onBotnetcmd( const char *from, const char *cmd )
{
}

/*! Called whenever the timer triggers normal actions inside psotnic.
 * Usual frequency is 1Hz, means once per second.
 */
void Module::onTimer()
{
}

/*!
 */
void Module::onConnecting()
{
}

/*!
 */
void Module::onConnected()
{
}

/*!
 */
void Module::onDisconnected ( const char *reason )
{
}

/*! Called when the bot got K-Lined.
 * \param reason The reason for the ban.
 */
void Module::onKlined ( const char *reason )
{
}

/*!
 */
void Module::onMode ( chan *ch, const char mode[2][MODES_PER_LINE], const char *arg[MODES_PER_LINE], const char *mask )
{
}

/*!
 */
void Module::onCrap ( const char *data )
{
}

/*! Called when the bot encounters a kick in a channel.
 * \param ch The channel from which someone was kicked.
 * \param kicked The user who got kicked.
 * \param kicker The person who initiated the kick.
 * \param msg The reason for the kick, may be empty.
 */
void Module::onKick ( chan *ch, chanuser *kicked, chanuser *kicker, const char *msg )
{
}

/*! Called when the bot sees a nickchange.
 * \param from The old nick.
 * \param to The new nick.
 */
void Module::onNickChange ( const char *from, const char *to )
{
}

/*!
 */
void Module::onInvite( const char *who, const char *channame, chan *chan, CHANLIST *chLst )
{
}

/*! Called whenever a line is received from the irc server.
 * \note Use this function only if the other functions don't give you the information you need!
 * \param data The raw data as received from the server.
 */
void Module::onRawirc( const char *data )
{
}

/*!
 */
void Module::onCtcp( const char *from, const char *to, const char *msg )
{
}

/*! Called when the channel \e ch was synced completly.
 * \param ch The channel that was synced completly.
 */
void Module::onChannelSynced( chan *ch) 
{
}

/*! Called when someone sends a line via partyline.
 * \param from The senders name.
 * \param flags The senders user flags.
 * \param cmd The command send.
 * \param arg All additional arguments in a string.
 */
void Module::onPartylineCmd( const char *from, int flags, const char *cmd, const char *args )
{
}

/*! Called when the \e topic of a channel \e ch was changed.
 * \param ch The channel.
 * \param topic The new topic.
 * \param u The user who changed the topic.
 * \param oldtopic The old topic string.
 */
void Module::onTopicChange( chan *ch, const char *topic, chanuser *u, const char *oldtopic )
{
}

/*! Called when someone parts from a \e channel but before psotnic takes actions.
 * \todo need a on[Pre|Post]_quit instead of the \e quit parameter.
 * \param mask The partees mask.
 * \param channel The channel from which a user parted.
 * \param msg The part message, may be empty.
 * \param quit true if the user left the server, false if he just parted from the channel.
 */
void Module::onPrePart( const char *mask, const char *channel, const char *msg, bool quit )
{
}

/*! Called when someone parts from a \e channel but after psotnic took actions itself.
 * \param mask The partees mask.
 * \param channel The channel from which a user parted.
 * \param msg The part message, may be empty.
 * \param quit true if the user left the server, false if he just parted from the channel.
 */
void Module::onPostPart( const char *mask, const char *channel, const char *msg, bool quit )
{
}

/*! Called just after the userlist was loaded successfully.
 */
void Module::onUserlistLoaded()
{
}

/*!
 */
void Module::onChanuserConstructor( const chan *ch, chanuser *cu )
{
}

/*! Called when psotnic received a SIGHUP-signal.
 * This may be used to inform the bot that logfiles got rotated or that some modules should start
 * special actions like closing files.
 */
void Module::onReceivedSigHup()
{
}

/*! Called when a module rehash was initiated via partyline.
 * \note This can safely be ignored, as no rehash algorythm is implemented at the time of writing.
 */
void Module::onRehash()
{
}

/*! Called when a new chan is created.
 * Create your customData here!
 */
void Module::onNewChan( chan *me )
{
}

/*! Called when a new chanuser is created.
 * Create your customData here.
 */
void Module::onNewChanuser( chanuser *me )
{
}

/*! Called when a new CHANLIST is created.
 * Create your customData here!
 */
void Module::onNewCHANLIST( CHANLIST *me )
{
}

/*! Called when a chan is deleted.
 * Remove your customData here!
 */
void Module::onDelChan( chan *me )
{
}

/*! Called when a chanuser is deleted.
 * Remove your customData here!
 */
void Module::onDelChanuser( chanuser *me )
{
}

/*! Called when a CHANLIST is deleted.
 * Remove your customData here!
 */
void Module::onDelCHANLIST( CHANLIST *me )
{
}

/*! Set the module author string.
 * \note Only used when instantiating a module.
 * \param author The authors name.
 */
void Module::setAuthor( pstring<> author )
{
	m_author = author;
}

/*! Set the time and date the module was compiled.
 * \note Only used when instantiating a module.
 * \param date The date of compilation.
 * \param time The time of compilation.
 */
void Module::setCompileTime( pstring<> date, pstring<> time )
{
	m_compileTime = time; m_compileDate = date;
}

/*! Set the module description.
 * \note Only used when instantiating a module.
 * \param desc The description string.
 */
void Module::setDescription( pstring<> desc )
{
	m_desc = desc;
}

/*! Set the module author email.
 * \note Only used when instantiating a module.
 * \param email The authors email address.
 */
void Module::setEmail( pstring<> email )
{
	m_email = email;
}

/*! Set the modules name.
 * \note Only used when instantiating a module.
 * \warning [a-fA-F0-9] only!
 * \param name The modules name.
 */
void Module::setName( pstring<> name )
{
	m_name = name;
}

/*! Set the modules version string.
 * \note Only used when instantiating a module.
 * \param version The version string.
 */
void Module::setVersion( pstring<> version )
{
	m_version = version;
}

/*! Return the authors name.
 * \return The authors name.
 */
pstring<> Module::author()
{
	return m_author;
}

/*! Return the time of compilation.
 * \return The time of compilation.
 */
pstring<> Module::compileTime()
{
	return m_compileTime;
}

/*! Return the date of compilation.
 * \return The date of compilation.
 */
pstring<> Module::compileDate()
{
	return m_compileDate;
}

/*! Return the directory where the module may place its data.
 * \return The directory where the module may place its data.
 */
pstring<> Module::dataDir()
{
	return m_dataDir;
}

/*! Return the modules description.
 * \return The modules description.
 */
pstring<> Module::description()
{
	return m_desc;
}

/*! Return the email address of the modules author.
 * \return The email address of the module author.
 */
pstring<> Module::email()
{
	return m_email;
}

/*! Return the filename which is used to identify the module.
 * \return The filename which is used to identify the module.
 */
pstring<> Module::file()
{
	return m_file;
}

/*! Return the handle used for the dl*-family commands.
 * Use this handle to reach functions inside the modules shared object!
 * \return The handle used for the dl*-family commands..
 */
void *Module::handle()
{
	return m_handle;
}

/*! Return the modules md5 checksum.
 * \return The modules md5 checksum.
 */
pstring<> Module::md5sum()
{
	return m_md5sum;
}

/*! Return the modules name.
 * \return The modules name.
 */
pstring<> Module::name()
{
	return m_name;
}

/*! Return the time the module was loaded.
 * \return The time the module was loaded.
 */
const time_t *Module::loadDate()
{
	return &m_loadDate;
}

/*! Return the modules version.
 * \return The modules version.
 */
pstring<> Module::version()
{
	return m_version;
}


#endif
