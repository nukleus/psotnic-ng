#ifndef NOGARBAGE_HPP
#define NOGARBAGE_HPP 

#include "module.h"

class chan;
class chanuser;
class ng_options;

class nogarbage : public Module
{
	public:
	nogarbage(void *, const char *, const char *, time_t, const char *);
	~nogarbage();

	virtual bool onLoad(string &msg);
	virtual void onBotnetcmd(const char *, const char *);
	virtual void onPrivmsg(const char *, const char *, const char *);
	virtual void onNotice(const char *, const char *, const char *);
	virtual void onCtcp(const char *, const char *, const char *);
	virtual void onNickChange(const char *from, const char *to);
	virtual void onJoin(chanuser *, chan *, const char *, int);
	virtual void onTimer();
	virtual void onKick(chan *, chanuser *, chanuser *, const char *);
	virtual void onPrePart(const char *, const char *, const char *, bool);
	virtual void onChanuserConstructor(const chan *, chanuser *u);
	virtual void onNewCHANLIST(CHANLIST *);
	virtual void onDelCHANLIST(CHANLIST *);
	virtual void onNewChan(chan *);
	virtual void onDelChan(chan *);

	void action(chanuser *, chan *, ng_options *);
	void ban(chan *, chanuser *, ng_options *);
	void kick(chan *, chanuser *, ng_options *);
	int calc_caps(const char *);
	void handle_query_spam(const char *, const char *);
	void load_conf();
	void save_conf();

	private:
	time_t next_save;
	int current_cycle_chan;
};

#endif /* NOGARBAGE_HPP */
