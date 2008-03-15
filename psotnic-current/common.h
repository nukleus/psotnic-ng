#ifndef PSOTNIC_COMMON_H
#define PSOTNIC_COMMON_H 1

#include "pstring.h"
#include "prots.h"

class chanuser;
class chan;
struct CHANLIST;
class ptime;
class comment;
class offence;

class Hooks
{
	public:
	void (*privmsg)(const char *from, const char *to, const char *msg);
	void (*notice)(const char *from, const char *to, const char *msg);
	void (*join)(chanuser *u, chan *ch, const char *mask, int netjoin);
	void (*botnetcmd)(const char *from, const char *cmd);
	void (*timer)();
	void (*connecting)();
	void (*connected)();
	void (*disconnected)(const char *reason);
	void (*klined)(const char *reason);
	void (*mode)(chan *ch, const char mode[2][MODES_PER_LINE], const char *arg[MODES_PER_LINE]);
	void (*modeWho)(chan *ch, const char mode[2][MODES_PER_LINE], const char *arg[MODES_PER_LINE], const char *mask);
	void (*crap)(const char *data);
	void (*kick)(chan *ch, chanuser *kicked, chanuser *kicker);
	void (*kickMsg)(chan *ch, const char *kicked, const char *kicker, const char *msg);
	void (*nick)(const char *from, const char *to);
	void (*invite)(const char *who, const char *channame, chan *chan, CHANLIST *chLst);
	void (*rawirc)(const char *data);
	void (*ctcp)(const char *from, const char *to, const char *msg);
	void (*justSynced)(chan *ch);
	void (*partylineCmd)(const char *from, int flags, const char *cmd, const char *args);
	void (*topicChange)(chan *ch, const char *topic, chanuser *u, const char *oldtopic);
	void (*pre_part)(const char *mask, const char *channel);
	void (*post_part)(const char *mask, const char *channel);
	void (*pre_partMsg)(const char *mask, const char *channel, const char *msg, bool quit);
	void (*post_partMsg)(const char *mask, const char *channel, const char *msg, bool quit);
	void (*userlistLoaded)();
	void (*chanuserConstructor)(const chan *ch, chanuser *cu);

	Hooks()
	{
		memset(this, 0, sizeof(Hooks));
	};
};

class module
{
	public:
	pstring<> desc, author, version;
	pstring<> file, md5sum;
	time_t loadDate;
	Hooks *hooks;
	void *handle;
	void *(*destroy)();

	module(const char *Desc="", const char *Author="", const char *Version="") :
			desc(Desc), author(Author), version(Version), hooks(new Hooks) {};

	~module()
	{
		destroy();
		delete hooks;
	}
};

struct HANDLE
{
	char *name;
	char *host[MAX_HOSTS];
	char *hostBy[MAX_HOSTS];
	unsigned char pass[16];
	int flags[MAX_CHANNELS+1];
	unsigned int ip;
	unsigned long long int channels;
	HANDLE *next;
	HANDLE *prev;
	char updated;
	ptime *creation;
	comment *info;
	offence *history;
	char *createdBy;
};
#endif
