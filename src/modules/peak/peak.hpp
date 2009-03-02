/*********************************************************
 * Psotnic peak module                                   *
 * Copyright (c) 2005 matrix <admin@areaunix.org>        *
 * Created: 14-05-2005 by matrix@IRCNet                  *
 * Usage: !peak in channel to show the peak              *
 *                Automatic when new peak in channel     *
 *********************************************************/

#ifndef PEAK_HPP
#define PEAK_HPP 

class Peak : public Module
{
	public:
	Peak(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);
	virtual bool onLoad(string &msg);
	virtual void onJoin(chanuser *u, chan *ch, const char *mask, int netjoin);
	virtual void onPrivmsg(const char *from, const char *to, const char *msg);
};

#endif /* PEAK_HPP */
