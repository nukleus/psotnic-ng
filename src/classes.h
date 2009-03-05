#ifndef PSOTNIC_CLASSES_H
#define PSOTNIC_CLASSES_H 1

class chan;
class fifo;
class inetconn;
class inet;
class adns;
class chanuser;
class ent;
class CONFIG;
class client;
class CBlowFish;

struct HANDLE;

#include <map>
#include <netinet/in.h> // AF_INET
#include <string>

using std::map;
using std::string;

#include "class-ent.h"
#include "config.h"
#include "CustomDataStorage.hpp"
#include "fastptrlist.h"
#include "grass.h"
#include "hashlist.h"
#include "Inetconn.hpp"
#include "Modeq.hpp"
#include "Protmodelist.hpp"
#include "pstring.h"
#include "structs.h"

class XSRand
{
	private:
	unsigned int x;
	static unsigned int a, b, c;

	public:
	XSRand() { };

	void srand(unsigned int seed);
	unsigned int rand();

};

class asyn_socks5
{
	char proxyip[16];
	unsigned short proxyport;
	char remotehost[256];
	unsigned short remoteport;
	int step;

	int i;
	int toRead;
	int fd;
	char buf[515];
	unsigned char len;
	unsigned char atyp;

	public:
	asyn_socks5();
	void setup(const char *pip, unsigned short pport, const char *rhost, unsigned short rport);
	int connect();
	void disconnect();
	int work(char byte);
	int use();
};

class clone_host
{
	public:
	chanuser *user;
	time_t cr;
	int type;

	clone_host(chanuser *u, int t);
	time_t creation();

	int operator==(const clone_host &c);
	int operator&(const clone_host &c);
};

class clone_ident
{
	public:
	chanuser *user;
	time_t cr;

	clone_ident(chanuser *u);
	time_t creation();

	int operator==(const clone_ident &c);
	int operator&(const clone_ident &c);

};

class clone_proxy
{
	public:
	chanuser *user;
	time_t cr;

	clone_proxy(chanuser *u);
	time_t creation();

	int operator==(const clone_proxy &c);
	int operator&(const clone_proxy &c);
};

class wasoptest
{
	class entry
	{
		public:
		char *mask;
		time_t when;

		entry(char *n, char *i, char *h);
		entry(char *m, int alloc=1);
		~entry();
	};

	public:
	ptrlist<entry> data;
	int TOL;
	time_t since;

	int add(chanuser *p);
	int add(char *nick, char *ident, char *host);
	int add(char *mask, int alloc=1);

	int remove(chanuser *user);
	int remove(char *nick, char *ident, char *host);
	int remove(char *mask);

	static int checkSplit(const char *reason);
	void expire();
	bool isEmpty();

	wasoptest(int life=60*45);
};

class masklist_ent
{
	public:
	char *mask;
	time_t expire;
	time_t when;
	char *who;
	bool sent;

	masklist_ent(const char *m, const char *w, time_t t);
	~masklist_ent();
};

class masklist
{
	public:
	ptrlist<masklist_ent> masks;
	bool received;

	int add(const char *mask, const char *who, time_t);
	int remove(char *mask);
	masklist_ent *find(const char *mask);
	masklist_ent *wildMatch(char *mask);
	masklist_ent *matchBan(char *mask, char *ip, char *uid);
	masklist_ent *match(const char *mask);
	
	ptrlist<masklist_ent>::iterator expire(ptrlist<masklist_ent>::iterator m=0);
//	masklist_ent *exactFind(char *mask);
	int remove(masklist_ent *m);
	void clear();
	masklist();
};

class chanset;

/*! Representation of a channel in IRC. */
class chan
#ifdef HAVE_MODULES
	: public CustomDataStorage
#endif
{
	public:
	fastptrlist<chanuser> users, toOp, botsToOp, opedBots, toKick;
	grouplist<clone_host> hostClones;
	grouplist<clone_ident> identClones;
	grouplist<clone_proxy> proxyClones;
	chan *next, *prev;
	pstring<> name, key, topic;
	int status, limit, channum, nextlimit, synlevel, flags, sentKicks;
	chanuser *me;
	chanset *chset;
	wasoptest *wasop;
	time_t initialOp, since;
	masklist list[4];
	masklist sentList[4];
	modeq modeQ[2];
	protmodelist *protlist[4];

	/* Actions */
	int op(chanuser **MultHandle, int num);
	int op(chanuser *p);
	int deOp(const chanuser *p);
	int kick4(chanuser **MultHandle, int num);
	int kick6(chanuser **MultHandle, int num);
	int kick(chanuser *p, const char *reason);
	int invite(const char *nick);
	void enforceLimits();
	void updateLimit();
	void updateKey(const char *newkey);
	int massKick(const int who, int lock=0);
	void requestOp() const;
	int applyShit(const protmodelist::entry *s, int force=0);
	void enforceBan(const char *ban, chanuser *u, const char *reason=NULL, const bool autoKick=true);
	bool flushKickQueue();
	void punishClones(const char *mask, bool isMyTurn);
	void knockout(chanuser *u, const char *reason, int delay=60);

	/* Probabilistics */
	int myTurn(int num, int hash=0);

	/* Got something */
	void gotNickChange(const char *from, const char *to);
	void gotMode(const char *args, const char *modes, const char *mask);
	void gotKick(const char *victim, const char *offender, const char *reason);
	void gotPart(const char *nick, int netsplit=0);
	int gotBan(const char *ban, chanuser *caster);
	bool checkClone(chanuser *u);

	chanuser *gotJoin(const char *mask, int def_flags=0);
	chanuser *getUser(const char *nick);

	/* other */
	void recheckFlags();
	void quoteBots(const char *str);
	int quoteOpedBots(const char *str, int num);
	void reOp();
	void rejoin(int t);
	int numberOfBots(int num) const;
	int chops() const;
	int synced() const;
	void setFlags(const char *str);
	void addFlags(const char *str);
	int hasFlag(char f) const;
	void removeFlags(const char *str);
	void buildAllowedOpsList(const char *offender);
	char *getModes();
	int userLevel(const chanuser *c) const;
	int userLevel(int flags) const;
	void names(const char *owner);
	void cwho(const char *owner, const char *arg="");
	int myPos();
#ifdef HAVE_ADNS
	void updateDnsEntries();
#endif

	protmodelist::entry *checkShit(const chanuser *u, const char *host=NULL);
	void recheckShits();
	void checkList();
	void checkKeepout();
	void checkProtectedChmodes();
	static char valid(const char *str);

	static bool chanModeRequiresArgument(char ,char);
	static char getTypeOfChanMode(char);
	static bool isChannel(const char *);

	/* Debug */
#ifdef HAVE_DEBUG
	void display();
#endif

	/* Constructor */
	chan();

	/* Destruction derby */
	~chan();
	void removeFromAllPtrLists(chanuser *handle);
};

#endif
