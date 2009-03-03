#ifndef PROTMODELIST_HPP
#define PROTMODELIST_HPP 

#include "ptrlist.h"

class inetconn;

class protmodelist
{
	int expire(const char *channel = NULL);

	public:

	class entry
	{
		public:
		char *mask;
		char *reason;
		char *by;
		time_t expires;
		time_t when;
		time_t last_used;
		bool sticky;

		entry();
		entry(const char *_mask, const char *_by, const char *_reason, time_t _when, time_t _expires, bool _sticky=0);
		~entry();
		char *fullReason();
	};

	ptrlist<entry> data;
	int type;
	char mode;

	entry *add(const char *mask, const char *by, time_t when, time_t expires, const char *reason, bool sicky);
	static entry *add(const char *data, inetconn *c=NULL);
	entry *conflicts(const char *mask);
	entry *match(const chanuser *u);
	entry *wildMatch(const char *mask);
	entry *find(const char *str);
	entry *findByMask(const char *mask);
	int remove(const char *mask);
	int remove(int num);

	int sendShitsToOwner(inetconn *c, const char *nme, int i=0);
	static int sendShitsToOwner(inetconn *c, int type, const char *channel, const char *expr);
	void sendToUserlist(inetconn *c, const char *name);
	static bool isSticky(const char *mask, int type, const chan *ch);
	static protmodelist::entry *findSticky(const char *mask, int type, const chan *ch);
	static entry *updateLastUsedTime(const char *channel, const char *mask, int type);
	static entry *findBestByMask(const char *channel, const char *mask, int type);
	static int addShit(const char *channel, const char *mask, const char *from, int delay, const char *reason, const char *bot=NULL);

	static int expireAll();
	void clear();


	protmodelist(int type, char mode);
	~protmodelist();
};

#endif /* PROTMODELIST_HPP */
