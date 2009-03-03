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

#ifdef HAVE_ADNS
class adns
{
	public:
	class host2ip
	{
		public:
		char *host;
		char *ip4;
		char *ip6;
		time_t creat_t;

		unsigned int hash;

		host2ip(const char *h, const char *i4="", const char *i6="");
		~host2ip();
		unsigned int hash32() const;
		int operator==(const host2ip &h) const;
		time_t creation() { return creat_t; };
	};

	class host2resolv
	{
		public:
		char *host;
		unsigned int hash;
		int fd;
		int type;

		host2resolv(const char *h);
		~host2resolv();
		unsigned int hash32() const;
		int operator==(const host2resolv &h) const;
	};

	protected:
	hashlist<host2ip> *cache;
	hashlist<host2resolv> *resolving;
	hashlist<host2resolv> *todo;

	host2ip *__getIp(const char *host);
	
	public:
	virtual void resolv(const char *host) = 0;
	virtual host2ip *getIp(const char *host) = 0;
	virtual void expire(time_t t, time_t now) = 0;

	static unsigned int xorHash(const char *str);
	

	virtual ~adns();	
#ifdef HAVE_DEBUG
	void display();
#endif
};

#endif

#ifdef HAVE_ADNS_PTHREAD
class adns_pthread : public adns
{
	private:
	bool die;

	pthread_mutex_t data_mutex;
	pthread_mutex_t condition_mutex;
	pthread_cond_t condition;

	pthread_t *th;
	int poolSize;

	void work();
	void removePool();
	void killThreads();

	public:
	virtual void resolv(const char *host);
	virtual host2ip *getIp(const char *host);
	void setupPool(int n);
	adns_pthread(int n);
	virtual ~adns_pthread();
	
	void expire(time_t t, time_t now);
	void lock_data();
	void unlock_data();

#ifdef HAVE_DEBUG
	void display();
#endif

	friend void *__adns_work(void *);
	friend class client;
};
#endif

#ifdef HAVE_ADNS_FIREDNS
class adns_firedns : public adns
{
	private:
	time_t last_check;
	bool shouldWeCheck();

	public:
	virtual void resolv(const char *host);
	virtual host2ip *getIp(const char *host);
	adns_firedns();
	virtual ~adns_firedns();
	
	void expire(time_t t, time_t now);

#ifdef HAVE_DEBUG
	void display();
#endif

	int fillFDSET(fd_set *set);
	void processResultSET(fd_set *set);
	void closeAllConnections();
};

#endif

class Pchar
{
	public:
	char *data;
	int step;
	int alloced;
	int len;

	Pchar(int s=4096);
	char *push(const char *str, int l=-1);
	char *push(const char c);
	~Pchar();
	void clean();
};

class Comment
{
	public:
	class entry
	{
		public:
		char *key;
		char *value;

		entry(const char *k, const char *v);
		~entry();

		int operator==(const entry &ent) const;
		int operator<(const entry &e) const;
	};

	ptrlist<entry> data;

	int add(char *key, char *value);
	int del(char *key);
	char *get(char *key);
	Comment();
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

class fifo
{
	public:
	int maxEnt;
	static time_t lastFlush;
	int flushDelay;
	ptrlist<pstring<8> > data;

	fifo(int size=0, int delay=1);
	~fifo();
	int push(const char *lst, ...);
	int wisePush(const char *lst, ...);
	int wildWisePush(char *lst, ...);
	char *pop();
	int flush(inetconn *c);
	char *flush();
};

class chanuser
#ifdef HAVE_MODULES
	: public CustomDataStorage
#endif
{
	public:
	char *nick;
	char *ident;
	char *host;
	char *reason;
	unsigned int flags;
	HANDLE *handle;
	unsigned int hash;
	char *ip4;
	char *ip6;
	unsigned int dnsinfo;
	unsigned int clones_to_check;

	chanuser(const char *n, const char *m, const char *h, const int f=0);
	chanuser(const char *m, const chan *ch, const int f=0, const bool scan=0);
	chanuser(const char *str);
	void getFlags(const chan *ch);
	~chanuser();
	int operator==(const chanuser &c) const;
	int operator<(const chanuser &c) const;
	unsigned int hash32() const;
	void setReason(const char *);
	int matches(const char *mask) const;
	int matchesBan(const char *mask) const;
#ifdef HAVE_ADNS
	int updateDNSEntry();
#endif
	bool ok() const;

#ifdef HAVE_DEBUG
	void display();
#endif
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

class settings : public options
{
	public:
	entBool PRIVATE_CTCP;
	entTime CYCLE_DELAY;
	entTime REJOIN_DELAY;
	entTime REJOIN_FAIL_DELAY;
	entTime HUB_CONN_DELAY;
	entTime IRC_CONN_DELAY;
	entTime AUTH_TIME;
	entInt OPS_PER_MODE;
	entTime ASK_FOR_OP_DELAY;
	entTime CONN_TIMEOUT;
	entTime KEEP_NICK_CHECK_DELAY;
	entBool SAVE_NICK;
	entBool REMEMBER_OLD_KEYS;
	entInt TELNET_OWNERS;
	entBool GETOP_OP_CHECK;
	entInt MAX_MATCHES;
	entInt PERIP_MAX_SHOWN_CONNS;
	entInt PERIP_BURST_TIME;
	entInt PERIP_BURST_SIZE;
	entTime PERIP_IGNORE_TIME;
	entInt SYNFLOOD_MAX_CONNS;
	entTime SYNFLOOD_IGNORE_TIME;
	entTime BIE_MODE_BOUNCE_TIME;
	entTime WASOP_CACHE_TIME;
	entTime CHAT_TIME;
	entTime AWAY_TIME;
	entPerc RANDOMNESS;
	entTime BETWEEN_MSG_DELAY;
	entBool PUBLIC_AWAY;
	entTime CLONE_LIFE_TIME;
	entInt HOST_CLONES;
	entInt IDENT_CLONES;
	entInt PROXY_CLONES;
	entInt CRITICAL_BOTS;
	entTime QUARANTINE_TIME;
	entTime BACKUP_MODE_DELAY;
	entInt DONT_TRUST_OPS;
	entTime SERVER_LIMIT_TIME;
	entBool PRE_023_FINAL_COMPAT;
	entBool PRE_0211_FINAL_COMPAT;
	entBool PRE_0214_FINAL_COMPAT;
	entBool PRE_REV127_COMPAT;
	entBool BOTS_CAN_ADD_SHIT; 
	settings();
};

class chanset : public options
{
	public:
	entInt AOP_BOTS;
	entInt BOT_AOP_BOTS;
	entInt BOT_AOP_MODE;
	entPerc PUNISH_BOTS;
	entInt INVITE_BOTS;
	entInt GUARDIAN_BOTS;
	entBool LIMIT;
	entTime LIMIT_TIME;
	entTime LIMIT_TIME_UP;
	entTime LIMIT_TIME_DOWN;
	entInt LIMIT_OFFSET;
	entInt LIMIT_BOTS;
	entPerc LIMIT_TOLERANCE;
	entBool CHANNEL_CTCP;
	entInt ENFORCE_BANS;
	entBool ENFORCE_LIMITS;
	entBool STOP_NETHACK;
	entInt GETOP_BOTS;
	entTime OWNER_LIMIT_TIME;
	entBool TAKEOVER;
	entBool BITCH;
	entBool WASOPTEST;
	entBool CLONECHECK;
	entBool DYNAMIC_BANS;
	entBool DYNAMIC_EXEMPTS;
	entBool DYNAMIC_INVITES;
	entBool LOCKDOWN;
	entTime LOCKDOWN_TIME;
	entInt PROTECT_CHMODES;
	entChattr MODE_LOCK;
	entBool STRICT_BANS;
	entBool CHECK_SHIT_ON_NICK_CHANGE;
	entBool INVITE_ON_UNBAN_REQUEST;
	entBool KEEPOUT;
	entInt IDIOTS;
	entInt USER_BANS;
	entInt USER_INVITES;
	entInt USER_EXEMPTS;
	entInt USER_REOPS;
	entBool CYCLE;
	
	chanset();
	chanset &operator=(const chanset &chset);
};

class prvset : public options
{
	public:
	entInt debug_show_irc_write;
	entInt debug_show_irc_read;

	prvset();
};

/*! Global bot configuration.
 */
class CONFIG : public options
{
	public:
	pstring<> file;
	entWord nick;
	entWord altnick;
	entWord ident;
	entWord oidentd_cfg;
	entWord nickappend;
	entString realname;
	entHost myipv4;
	entHost vhost;
	entWord userlist_file;
	entString kickreason;
	entString quitreason;
	entString limitreason;
	entString keepoutreason;
	entString partreason;
	entString cyclereason;
	entWord botnetword;

	entWord logfile;
	entWord handle;

	entInt listenport;
#ifdef HAVE_SSL
	entInt ssl_listenport;
#endif
	entBool keepnick;
	entInt ctcptype;
	entBool dontfork;

	entHPPH bnc;
	entHPPH router;

	entHub hub;
	entMult alt_storage;
	entHub alt[MAX_ALTS];
	entHub *currentHub;

	entMult server_storage;
	entMult ssl_server_storage;
#ifdef HAVE_SSL
	entServer server[MAX_SERVERS*2];
#else
	entServer server[MAX_SERVERS];
#endif
	entMD5Hash ownerpass;

	entMult module_load_storage;
	entLoadModules module_load[MAX_MODULES];

	entMult module_debugLoad_storage;
	entLoadModules module_debugLoad[MAX_MODULES];

	entString alias[MAX_ALIASES];
	entMult alias_storage;

#ifdef HAVE_ADNS
	entInt resolve_threads;
	entTime domain_ttl;
#endif

	entBool check_shit_on_nick_change;

	int bottype;


	CONFIG();
	void polish();
	void load(const char *file, bool decrypted=false);
	options::event *save(bool decrypted=false);
};

class CHANLIST
#ifdef HAVE_MODULES
	: public CustomDataStorage
#endif
{
	public:
	pstring<> name;
	pstring<> pass;
	int status;
	int nextjoin;
	char updated;
	chanset *chset;
	wasoptest *wasop;
	wasoptest *allowedOps;
	protmodelist *protlist[4];

	CHANLIST() :
#ifdef HAVE_MODULES
		CustomDataStorage(),
#endif
		status(0), nextjoin(0), updated(0), chset(0), wasop(0), allowedOps(0) { };
	void reset();

};

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

#include "Client.hpp"

#ifdef HAVE_TCL
#include <tcl.h>
class tcl
{
	Tcl_Interp *tcl_int;
	char *setGlobalVar(char *name, char *value);
	void addCommands();

	public:

	int curid;
	struct timer_t
	{
		char *nick;
		time_t exp;
		int id;
	};

	ptrlist<timer_t> timer;

	int load(char *script);
	Tcl_Interp *getInt();
	int eval(char *cmd);

	void expireTimers();

	tcl();
	~tcl();

};
#endif

class penal
{
	private:
	time_t when;
	int penality;

	public:
	operator int();
	void calc();
	penal operator+(int n);
	penal operator+=(int n);
	penal operator-(int n);
	penal operator-=(int n);
	penal operator++(int);
	penal operator--(int);
	int val() { return penality; };

	penal(int n=0);
};

/*! Ignore handling. */
class ign
{
	public:
	class entry
	{
		public:
		unsigned int ip;
		int count;
		time_t nextConn;
		time_t when;

		int operator==(const entry &e) const;
		int operator==(const unsigned int &IP) const;
		entry(unsigned int IP);
		time_t creation();
	};

	ptrlist<entry> data;

	int nextConn;
	int count;

	entry *hit(unsigned int ip);
	void removeHit(unsigned int ip);
	int isIgnored(unsigned int ip);
	void expire();
	void calcCount();

	void parseUser(char *who, char *cmd, char *ip);

	ign();
};

class idle
{
	public:
	char *away;
	time_t nextStatus;
	time_t lastStatus;
	time_t nextMsg;
	time_t lastMsg;

	const char **awayReasons;
	const char **backReasons;
	const char **awayAdd;
	const char **backAdd;

	idle();
	int spread(int x);
	void calcNextStatus();
	void calcNextMsg();
	void sendMsg();
	void eval();
	void togleStatus();
	void init();
	int getIdle();
	int getET();
	int getRT();
	void load();
	const char *getRandAwayMsg();
	const char *getRandBackMsg();
	const char *getRandAwayAdd();
	const char *getRandBackAdd();

};

#endif
