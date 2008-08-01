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

#include "../prots.h"
#include "../global-var.h"
#include "module.h"
#include <time.h>
#include <string>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace std;

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

/**
 * fileExists
 * this function checks if the passed in filename is a valid file already or not
 *
 * @param file The filename
 * @return true if the file exists, false otherwise (or if the file isnt readable I think.. in which case ERRORCITY.)
 */
bool fileExists(string file) {
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
bool isIn(string channel) {
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
void logLine(string line,string channel) {
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
string getNick(const char *from) {
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
string getRest(const char *from) {
	string ret = from; // our return value
	// strip off the nickname! part of the host
	if (ret.find("!") > 0) { ret = ret.substr(ret.find("!") + 1); }
	// return
	return ret;
}

void hook_privmsg(const char *from, const char *to, const char *msg) {
	chan *ch = findChannel(to);// channel we are acting in
	string line = "<" + getNick(from) + "> " + msg;
	// check if we have a channel
	if(ch) {
		logLine(line,string(ch->name));
	} else {
		logLine(line,LOG_PRIVMSG);
	} // end of check for if we found a valid channel
} // end of function

void hook_ctcp(const char *from, const char *to, const char *msg) {
	chan *ch = findChannel(to);// channel we are acting in
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

void hook_join(chanuser *u, chan *ch, const char *mask, int netjoin) {
	if (u && ch) {
		string line = string(u->nick) + " (" + getRest(mask) + ") joined " + string(ch->name);
		logLine(line,string(ch->name));
	}
}

void hook_nick(const char *from, const char *to) {
	string line = "Nick change: " + string(from) + " -> " + string(to);
	logLine(line,"");
}

void hook_topicChange(chan *ch, const char *topic, chanuser *u, const char *oldtopic) {
	if (ch && u) {
		string line = "Topic changed on " + string(ch->name) + " by " + string(u->nick) + "!" +
			string(u->ident) + "@" + string(u->host) + ": " + string(topic);
		logLine(line,string(ch->name));
	}
}

void hook_post_part(const char *mask, const char *channel, const char*, bool) {
	logLine(getNick(mask) + " (" + getRest(mask) + ") left " + string(channel),string(channel));
}

void hook_rawirc(const char *data) {
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

extern "C" module *init() {
	module *m = new module("Channel logger", "Stuart Scott <stu@wilf.co.uk>", "0.1.0");
	m->hooks->privmsg = hook_privmsg;
	m->hooks->ctcp = hook_ctcp;
	m->hooks->join = hook_join;
	m->hooks->nick = hook_nick;
	m->hooks->topicChange = hook_topicChange;
	m->hooks->post_part = hook_post_part;
	m->hooks->rawirc = hook_rawirc;
	return m;
}

extern "C" void destroy() { }

