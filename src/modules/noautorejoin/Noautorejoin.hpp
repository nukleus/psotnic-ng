#ifndef NOAUTOREJOIN_HPP
#define NOAUTOREJOIN_HPP 

#include "Module.hpp"

class arj_chk : public Module
{
	public:
	class entry
	{
		public:
		chan *channel;
		char nick[32];
		time_t timestamp;

		entry(chan *, char *, time_t);
		bool expired();
	};

	arj_chk(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);
	~arj_chk();

	virtual bool onLoad(string &msg);
	virtual void onJoin(chanuser *u, chan *ch, const char *mask, int netjoin);
	virtual void onKick(chan *cg, chanuser *kicked, chanuser *kicker, const char *msg);
	virtual void onTimer();

	void add(chan *, char *);
	entry *find(chan *, char *);

	private:
	ptrlist<entry> data;
};

#endif /* NOAUTOREJOIN_HPP */
