/**
 * Log for psotnic
 * This is a logger producing Eggdrop format logs for channels
 *
 * Usage: Set the parameters below and away you go :)
 *
 * Original author: Stuart Scott <stu@wilf.co.uk>
 * portability fix: Stefan Valouch <stefanvalouch@googlemail.com>
 *
 * @name log
 * @desc Logger producing Eggdrop format logs
 */

#include <ctime>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

#include "Chan.hpp"
#include "Chanuser.hpp"
#include "Client.hpp"
#include "functions.hpp"
#include "global-var.h"
#include "Log.hpp"

using namespace std;

ChanLog::ChanLog( void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir ) : Module( handle, file, md5sum, loadDate, dataDir )
{
}

bool ChanLog::onLoad( string &msg )
{
	return true;
}

/**
 * fileExists
 * this function checks if the passed in filename is a valid file already or not
 *
 * @param file The filename
 * @return true if the file exists, false otherwise (or if the file isnt readable I think.. in which case ERRORCITY.)
 */
bool ChanLog::fileExists( const string &file ) {
	bool ret = false; // our return value
	fstream fin; // file
	// try to open the file
	fin.open(file.c_str(),ios::in);
	// check if we managed to open it
	if (fin.is_open()) { ret = true; }
	// close it
	fin.close();
	// return
	return ret;
}

/**
 * isIn
 * This function checks to see if the channel name is in our list of logging channels
 * @param channel  The channel name to search for
 * @return true if the channel should be logged, otherwise false
 */
bool ChanLog::isIn( const string &channel ) {
	bool ret = false; // our return value
	// loop through our channel list and see if we can find the channel
	for (int i = 0;i < MAX_CHANNELS;i++) {
		if (LOG_CHANNELS[i] == channel) {
			ret = true; break;
		}
	}
	// return
	return ret;
}

/**
 * logLine
 * This function does the actual logging
 *
 * @param line The line to log
 * @param channel The channel to log it to
 */
void ChanLog::logLine( const string &line, const string &channel ) {
	bool all = false; // whether we should log to all channels
	string file,timestamp; // some strings
	struct tm *ptr; // pointer
	time_t tm; // time holder
	char hold[10]; // holder
	fstream log; // filestream

	// check if we should be logging to all channels, private messages or one channel
	if (channel == "") {
		// we got passed a null, so log to all channels
		all = true;
	} else if ((channel != LOG_PRIVMSG) && (!isIn(channel))) {
		// we got passed a channel we shouldnt log, so dont ;)
		return;
	}

	// loop through the channels
	for (int i = 0;i < MAX_CHANNELS;i++) {

		// check what we should be logging to
		if (all) { file = LOG_CHANNELS[i]; }
		else if (channel == LOG_PRIVMSG) { file = string(ME.nick); }
		else { file = channel; }

		// check if we have a valid looking file to log to
		if (file == "") { break; }

		// if we have a filename with a # at the front, strip it off
		if (file.find("#",0) == 0) { file.erase(0,1); }

		// lowercase the filename
		transform (file.begin(),file.end(), file.begin(), static_cast<int (*)(int)>(tolower));

		// get the current time
		tm = time(NULL);
		// get the local time
		ptr = localtime(&tm);
		// get a nicely formatted time
		strftime(hold ,10 , "[%R] ",ptr);
		// save it as timestamp
		timestamp = hold;
		// get a nicely formated date
		strftime(hold ,10 , "%Y%m%d",ptr);
		// build the filename
		file = LOG_DIR + file + LOG_MID + hold;

		// check if the filename exists
		if (fileExists(file)) {
			// file exists, append
			log.open(file.c_str(),ios::out|ios::app);
		} else {
			// new file, open at the beginning
			log.open(file.c_str(),ios::out);
		}
		// log the entry to the open file
		log << timestamp << line << endl;
		// close the file
		log.close();
		// if we are only doing 1 channel, break now
		if (!all) { break; }
	} // end of for looping through channels
}

/**
 * getNick
 * This function returns just the nickname from a host string
 *
 * @param from The host string
 * @return The nickname
 */
string ChanLog::getNick(const char *from) {
	string ret = from; // our return value
	// get the nickname from the host string
	if (ret.find("!") > 0) {
		ret = ret.substr(0,ret.find("!"));
	}
	// return
	return ret;
}

/**
 * getRest
 * This function returns everything but the nickname! from the host string
 *
 * @param from The host string
 * @return The host part
 */
string ChanLog::getRest(const char *from) {
	string ret = from; // our return value
	// strip off the nickname! part of the host
	if (ret.find("!") > 0) { ret = ret.substr(ret.find("!") + 1); }
	// return
	return ret;
}

void ChanLog::onPrivmsg(const char *from, const char *to, const char *msg) {
	chan *ch = ME.findChannel(to);// channel we are acting in
	string line = "<" + getNick(from) + "> " + msg;
	// check if we have a channel
	if(ch) {
		logLine(line,string(ch->name));
	} else {
		logLine(line,LOG_PRIVMSG);
	} // end of check for if we found a valid channel
} // end of function

void ChanLog::onCtcp(const char *from, const char *to, const char *msg) {
	chan *ch = ME.findChannel(to);// channel we are acting in
	string line = "Action: " + getNick(from);
	string action = msg;
	line = line + action.substr(6);
	// check if we have a channel
	if(ch) {
		logLine(line,string(ch->name));
	} else {
		logLine(line,LOG_PRIVMSG);
	}// end of check for if we found a valid channel
}

void ChanLog::onJoin(chanuser *u, chan *ch, const char *mask, int netjoin) {
	if (u && ch) {
		string line = string(u->nick) + " (" + getRest(mask) + ") joined " + string(ch->name);
		logLine(line,string(ch->name));
	}
}

void ChanLog::onNickChange(const char *from, const char *to) {
	string line = "Nick change: " + string(from) + " -> " + string(to);
	logLine(line,"");
}

void ChanLog::onTopicChange(chan *ch, const char *topic, chanuser *u, const char *oldtopic) {
	if (ch && u) {
		string line = "Topic changed on " + string(ch->name) + " by " + string(u->nick) + "!" +
			string(u->ident) + "@" + string(u->host) + ": " + string(topic);
		logLine(line,string(ch->name));
	}
}

void ChanLog::onPostPart(const char *mask, const char *channel, const char*, bool) {
	logLine(getNick(mask) + " (" + getRest(mask) + ") left " + string(channel),string(channel));
}

void ChanLog::onRawirc(const char *data) {
	string dat = data; // our data string
	string cmd,usr,msg,nick; // some strings
	char arg[4][MAX_LEN]; // our arguments
	// get our broken up words from the data
	str2words(arg[0], data, 4, MAX_LEN, 0);
	// get the command
	cmd = arg[1];
	// check if we had a quit, if so log it
	if ((cmd == "QUIT") || (cmd == "KICK") || (cmd == "MODE")) {
		// get the user
		usr = dat.substr(1,dat.find(" ") - 1);
		// get the nick from the user
		nick = getNick(usr.c_str());
		// get the message
		msg = dat.substr(dat.find(":",1) + 1);
		// check what it is that happened
		if (cmd == "QUIT") {
			// log the quit
			logLine(nick + " (" + getRest(usr.c_str()) + ") left irc: " + msg,"");
		} else if (cmd == "KICK") {
			// log the kick
			logLine(string(arg[3]) + " kicked from " + string(arg[2]) + " by " + nick + ": " + msg,string(arg[2]));
		} else {
			// log the mode change
			msg = dat.substr(dat.find(" ",dat.find(arg[2])) + 1);
			logLine(string(arg[2]) + ": mode change '" + msg + "' by " + usr,string(arg[2]));
		}
	}
}

MOD_LOAD( ChanLog );
MOD_DESC( "ChanLog", "Eggdrop format channel logger" );
MOD_AUTHOR( "Stuart Scott", "stu@wilf.co.uk" );
MOD_VERSION( "0.1.0" );

