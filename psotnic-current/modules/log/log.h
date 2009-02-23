#ifndef LOG_H
#define LOG_H 

#include "module.h"

// the location of the log files
const static string LOG_DIR = "/home/psotnics/logs/";
// the middle bit in the logfile names, format is channel-mid-date (channel has its # stripped first) and botname-mid-date
const static string LOG_MID = ".log.";
// the list of channels to log for, this is needed as when someone QUITS or changes nick I have no idea what channel(s) they
//  are in, so its logged to all these channels
const static string LOG_CHANNELS[MAX_CHANNELS] = {"#uk","#psottst","#beginner"};
// the private message log tag (unless your bot is in a channel called PRIVATE_MESSAGE, there shouldnt be a need to 
//  change this :P
const static string LOG_PRIVMSG = "PRIVATE_MESSAGE";

class ChanLog : public Module
{
	public:
	ChanLog( void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir );

	virtual bool onLoad( string &msg );
	virtual void onPrivmsg( const char *from, const char *to, const char *msg );
	virtual void onCtcp( const char *from, const char *to, const char *msg );
	virtual void onJoin( chanuser *u, chan *ch, const char *mask, int netjoin );
	virtual void onNickChange( const char *from, const char *to );
	virtual void onTopicChange( chan *ch, const char *topic, chanuser *u, const char *oldtopic );
	virtual void onPostPart( const char *mask, const char *channel, const char *msg, bool quit );
	virtual void onRawirc( const char *data );

	protected:
	bool fileExists( const string &file );
	bool isIn( const string &channel );
	void logLine( const string &file, const string &channel );
	string getNick( const char *from );
	string getRest( const char *from );
};

#endif /* LOG_H */
