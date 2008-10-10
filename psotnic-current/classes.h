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

struct HANDLE;

#include "pstring.h"
#include "class-ent.h"
#include <string>
#include <map>

using std::map;

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

class comment
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
	comment();
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

class modeq
{

	public:
	class modeq_ent
	{
		public:
		char mode[3];
		char *arg;
		int expire;
		bool backupmode;
		bool reject;

		modeq_ent(time_t exp, const char *m, const char *a=NULL);
		~modeq_ent();

		int operator==(modeq_ent &c);
		int operator&(modeq_ent &c);
	};

	private:
	chan *ch;
	int validate(modeq_ent *e, int &a, int &n);
	grouplist<modeq_ent> data;

	public:

	modeq_ent *add(time_t exp, const char *m, const char *a=NULL);
	int flush(int prio);

	modeq(chan *channel=NULL);
	void setChannel(chan *channel);
	void removeBackupModesFor(const char *mode, const char *arg);
	modeq_ent *find(const char *mode, const char *arg);
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

/*! Base class for all custom data storages.
 * Inherit from this function if you want your class to be saveable as custom data.
 */
class CustomDataObject
{
	public:
		CustomDataObject() {};
		virtual ~CustomDataObject() {};
};

/*! Storage place for custom data entries.
 * In order to give the ability to let modules store data inside your class, simply inherit from
 * this one.
 */
class CustomDataStorage
{
	public:
		CustomDataStorage();
		virtual ~CustomDataStorage();

		CustomDataObject *customData( const char *moduleName );
		void setCustomData( const char *moduleName, CustomDataObject *data );
		void delCustomData( const char *moduleName );

	private:
		map< const char *, CustomDataObject * > m_data;		//! Storage.
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

class inetconn
{
	private:
	void _close(const char *reason=NULL);
	CBlowFish *blowfish;
	void lock();
	void unlock();

	public:
	int fd;
	int status;
	char *name;
	char *pass;
	char *tmpstr;
	char *origin;
	int tmpint;
	int killTime;
	int lastPing;
#ifdef HAVE_SSL
	SSL_CTX *ssl_ctx;
	SSL *ssl;
#endif


	IOBUF read;
	IOBUF write;

	HANDLE *handle;

	int enableLameCrypt();
	int enableCrypt(const unsigned char *key, int len=-1);
	int enableCrypt(const char *key, int len=-1);
	int disableCrypt();
#ifdef HAVE_SSL
	bool enableSSL();
	void SSLHandshake();
#endif
	int send(const char *lst, ...);
	int va_send(va_list ap, const char *lst);
	int readln(char *buf, int len, int *ok=NULL);
	int open(const char *pathname, int flags, mode_t mode=0);
	void close(const char *reason=NULL);

	int checkFlag(int flag, int where=GLOBAL);

	int sendPing();
	int timedOut();
	unsigned int getPeerIp4();
	unsigned int getMyIp4();
	const char *getPeerIpName();
	const char *getMyIpName();
	char *getPeerPortName();
	char *getMyPortName();

	//int getPort();
	void echo(int n, char *app=NULL);
	inetconn();
	~inetconn();

	int isSlave();
	int isLeaf();
	int isMain();
	int isRegBot();
	int isRegOwner();
	int isRegUser();
	int isRedir();
	int isReg();
	int isConnected();
	int isBot();
	int isSSL();
	
	void writeBufferedData();

	friend class inet;// closeConn(int fd);
};

class inet
{
	public:
	int conns, max_conns, listenfd, maxFd;
	inetconn *conn, hub, irc;
#ifdef HAVE_SSL
	int ssl_listenfd;
	static SSL_CTX *server_ctx;
#endif

	inetconn *addConn(int fd);
	inetconn *findConn(const char *name);
	inetconn *findConn(const HANDLE *h);
	int closeConn(inetconn *c, const char *reason=NULL);
	void send(int who, const char *lst, ...);
	void sendexcept(int excp, int who, const char *lst, ...);
	void sendBotListTo(inetconn *c);
	void sendOwner(const char *who, const char *lst, ...);
	void sendCmd(inetconn *c, const char *lst, ...);
	void sendCmd(const char *from, const char *lst, ...);
	void propagate(inetconn *from, const char *str, ...);
	inetconn *findRedirConn(inetconn *c);
	int bidMaxFd(int fd);
	inetconn *findMainBot();

#ifdef HAVE_DEBUG
	void display();
#endif

	void resize();
	int bots();
	int owners();

	static int gethostbyname(const char *host, char *buf, int protocol=AF_INET);

	inet();
	~inet();
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

	CHANLIST();
	void reset();

};

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
	int flushKickQueue();
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

/** Stores server information.
 *  \author patrick <patrick@psotnic.com>
 *  \warning the values are only available when the bot is connected to an ircserver
 */

class Server
{
        public:
	// from 004
	char *name; ///< name of the server
        char *version; ///< ircd version
	char *usermodes; ///< available user modes
	char *chanmodes; ///< available channel modes (please prefer isupport.chanmodes)

	// from 005
        class Isupport
        {
            public:
            typedef map<std::string, std::string> isupportType;
            isupportType isupport_map; ///< contains all 005 tokens
            Server *server; // pointer to upper class

            /* the following variables are in the map too,
             * but either they are used very often, so the bot should not search
             * for them in map everytime or the value is not accessable without further parsing.
             */

            char *chan_status_flags; ///< specifies a list of channel status flags (usually: "ov")
            char *chanmodes; ///< indicates the channel modes available and the arguments they take (format: "A,B,C,D")
            int maxchannels; ///< maximum number of chans a client can join
            int maxlist; ///< limits how many "variable" modes of type A a client may set in total on a channel
            int max_kick_targets; ///< maximum number of users that can be kicked with one KICK command
            int max_who_targets; ///< maximum number of targets that are allowed in a WHO command
            int max_mode_targets; /**< maximum number of targets (channels and users) that are allowed in a MODE command
                                      e.g. MODE #chan1,#chan2,#chan3 */

            Isupport();
            ~Isupport();
            void insert(const char *key, const char *value);
            const char *find(const char *key);
            void reset();
            void init();
        } isupport;

	Server();
	~Server();
	void reset();
};

class client
{
	public:
	chan *first, *last, *current;
	int channels, joinDelay;
	pstring<> nick, ident, host, mask, ircip, uid, overrider;
	time_t nextNickCheck, startedAt, nextConnToIrc, nextConnToHub;
	time_t nextRecheck;
	time_t nextReconnect, ircConnFailDelay;
	Server server;

	/* Irc channels */
	chan *createNewChannel(const char *name);
	chan *findChannel(const char *name);
	chan *findNotSyncedChannel(const char *name);
	void removeChannel(const char *name);
	void gotUserQuit(const char *mask, const char *reason=NULL);
	void recheckFlags();
	void recheckFlags(const char *channel);
	void autoRecheck();
	void rejoin(const char *name, int t);
	void rejoinCheck();
	void joinAllChannels();
	void gotNickChange(const char *from, const char *to);
	void checkQueue();
	void inviteRaw(const char *str);
	void restart();
	void cycle(chan *ch);

	//char *getMyMask(char *buf, int len);
	void newHostNotify();

	/* Nick stuff */
	void registerWithNewNick(char *nick);

	/* Net */
	int connectToIRC(entServer *s=NULL);
	int connectToHUB();
	int jump(const char *host, const char *port, const char *owner, int protocol=AF_INET);
	int jumps5(const char *proxyhost, int proxyport, const char *ircserver, int ircport, const char *owner);
	static entServer *getRandomServer();

	/* other */
	void sendStatus(const char *name);
	void checkMyHost(const char *to, bool justConnected=0);
	void privmsg(const char *target, const char *lst, ...);
	void notice(const char *target, const char *lst, ...);

	/* Debug */
#ifdef HAVE_DEBUG
	void display();
#endif

	/* Constructor */
	client();

	/* Destruction derby */
	~client();
	void reset();
};

class ul
{
	public:
	HANDLE *first, *last;
	//HANDLE noban;
	CHANLIST chanlist[MAX_CHANNELS];
	int users, bots;
	unsigned long long int SN; // ;-)
	Pchar *ulbuf;
	time_t nextSave;
	protmodelist *protlist[4];
	chanset *dset;

	/* Handle */
	HANDLE *addHandle(const char *name, unsigned int ip, int flags, const char *sec, const char *nano, const char *by);
	HANDLE *findHandle(const char *name) const;
	int removeHandle(const char *name);
	void cleanHandle(HANDLE *h);
	void cleanChannel(int i);

	int userLevel(const HANDLE *h, int n) const;
	int userLevel(int flags) const;
	bool hasEmptyFlags(const HANDLE *h) const;
	HANDLE *me();

	/* `idiots' code */
	int levelFlags(int level) const;
	void decrementFlags(HANDLE *h, int channum, unsigned int number);
	int punishIdiot(HANDLE *h, int channum, unsigned int number);

	HANDLE *findHandleByHost(const char *host, const int channum=GLOBAL) const;
	void reportNewOffences(inetconn *c, const bool showCmd=false);
	unsigned int offences() const;

	int addIdiot(const char *mask, const char *channel, const char *reason, const char *number);
	int addIdiot(const char *mask, const char *channel, const char *reason, unsigned int number);
	int addIdiot(chanuser *user, chan *ch, const char *reason, unsigned int number);

	/* Host */
	int addHost(HANDLE *p, const char *host, const char *by, time_t when=0, int num=-1);
	int findHost(const HANDLE *p, const char *host) const;
	int removeHost(HANDLE *p, const char *host);
	int wildFindHost(const HANDLE *p, const char *host) const;
	int wildFindHostExt(const HANDLE *p, const char *host) const;
	int wildFindHostExtBan(const HANDLE *p, const char *host) const;
	HANDLE *matchMaskToHandle(const char *mask) const;

	/* Flags */
	int changeFlags(const char *name, const char *flags, const char *channel, inetconn *c=NULL);
	int changeFlags(HANDLE *p, const char *flags, int channum, inetconn *c=NULL);
	flagTable *findFlagByLetter(char letter, flagTable *ft);
	bool mergeFlags(unsigned int &flags, const char *str);    
	
	int getFlags(const char *mask, const chan *ch);
	static void flags2str(int flags, char *str);
	static int str2userFlags(const char *str);
	static int str2botFlags(const char *str);
	static void compactFlags(char *str);
	bool hasPartylineAccess(const char *mask) const;
	static bool isBot(const HANDLE *h);
	bool isBot(const char *name);
	bool isBot(unsigned int ip);
	static bool isMain(const HANDLE *h);
	static bool isSlave(const HANDLE *h);
	static bool isLeaf(const HANDLE *h);
	bool isIdiot(const char *mask, int channum) const;

	/* Channels */
	int addChannel(const char *name, const char *pass, const char *attr);
	bool removeChannel(const char *name, char *removedName=NULL);
	int findChannel(const char *name) const;
	bool globalChset(inetconn *c, const char *var, const char *value, int *index=NULL);
	int isRjoined(int i, const HANDLE *h=NULL);
	int rjoin(const char *bot, const char *channel);
	int rpart(const char *bot, const char *channel, const char *flags="");
	CHANLIST *findChanlist(const char *name);

	/* other */
	void send(inetconn *c, CHANLIST *chan);
	void send(inetconn *c, HANDLE *h, int strip=0);
	void sendHandleInfo(inetconn *c, const HANDLE *h, const char *mask=NULL);
	void sendDSetsToBots(const char *channel);

	bool save(const char *file, const int cypher=1, const char *key=NULL);
	int load(const char *file, const int cypher=1, const char *key=NULL);
	void update();
	void send(inetconn *c);
	void sendToAll();
	HANDLE *checkBotMD5Digest(unsigned int ip, const char *digest, const char *authstr);
	HANDLE *checkPartylinePass(const char *username, const char *pass, int flags=0);

	HANDLE *changePass(char *user, char *pass);
	HANDLE *changeIp(char *user, char *ip);
	void setPassword(char *user, char *rawpass);
	bool hasWriteAccess(inetconn *c, char *handle);
	bool hasWriteAccess(inetconn *c, HANDLE *h);
	bool hasReadAccess(inetconn *c, char *handle);
	bool hasReadAccess(inetconn *c, HANDLE *h);

	HANDLE *matchPassToHandle(const char *pass, const char *host, const int flags=0) const;
	void autoSave(int notice=1);
	void sendUsers(inetconn *c);
	void sendBotTree(inetconn *c);
	int parse(char *data);
	/* Constructor */
	ul();

	/* Destruction derby */
	~ul();
	void reset();
	void destroy(CHANLIST *p);
	void destroy(HANDLE *p);
};

class ptime
{
	public:
	struct timeval tv;
	ptime();
	ptime(const char *s, const char *n);
	ptime(time_t s, time_t n);
	char *print();
	char *ctime();
	int operator==(ptime &p);
};

#ifdef HAVE_TCL
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

class http
{
	public:
	class url
	{
		public:
		char *link;
		char *host;
		char *file;
		char *filepath;
		char *ip;
		int port;

		url(const char *u);
		~url();
		int ok();

#ifdef HAVE_DEBUG
		void print();
#endif
	};

	enum errorTypes {
		ERR_OK        =  0,
		ERR_EOF       = -1,
		ERR_URL       = -2,
		ERR_DST       = -3,
		ERR_CONN      = -4,
		ERR_HEADER    = -5,
		ERR_DST_WRITE = -6,
		ERR_READ      = -7
	};

	inetconn www;
	char *data;

	int get(const char *link, int estSize=4096);
	int get(const char *link, const char *file);

	int get(url &u, int estSize=4096);
	int get(url &u, const char *file);

	static char *error(int n);

	http();
	~http();

	private:
	int sendRequest(url &u);
	int processHeader();

};


class update
{
	public:
	inetconn child;
	int parent;
	pid_t pid;
	pid_t child_pid;

	update();
	bool forkAndGo(char *site);
	void end();
	bool doUpdate(const char *site);
};


class offence
{
	public:
	class entry
	{
		public:
		char *chan; //channel
		char *mode; // irc line
		time_t time; //offence time
		unsigned int count; // offence in line counter (1-5)
		int fromFlags; // flags before change
		int toFlags; //flags after change
		bool global;

		entry(const char *_chan, const char *_mode, time_t _time, unsigned int _count, int _fromFlags, int _toFlags, const bool _global=false);
		~entry();

		int operator==(const entry &ent) const;
		int operator<(const entry &e) const;
#ifdef HAVE_DEBUG
		void display();
#endif
	};

	ptrlist<entry> data;

	int add(const char *_chan, const char *_mode, time_t _time, unsigned int _count, int _fromFlags, int _toFlags, const bool _global=false);
	int del(const char *_chan, const char *_mode, time_t _time, unsigned int _count);

	offence::entry *get(const char *_chan, const char *_mode="", time_t _time=0, unsigned int _count=0);
	offence();
};

#endif
