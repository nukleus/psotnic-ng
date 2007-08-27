/*******************************************************
 * Psotnic framework generated by framework.sh
 * Copyright (c) 2004-2005 Grzegorz Rusin <grusin@gmail.com>
 * Created: Fri Apr  6 10:34:25 BST 2007
 * 
 * WARNING! All changes made in this file will be lost!
 */

#define HAVE_MODULES 1
#include "includes.h"
#include "iterator.h"
#include "ptrlist.h"
#include "fastptrlist.h"
#include "tiny_ptrlist.h"
#include "hashlist.h"
#include "defines.h"
#include "pstring.h"
#include "class-ent.h"
#include "grass.h"
#include "common.h"

typedef int foobar;
typedef int* FUNCTION;
 
#ifndef PSOTNIC_STRUCTS_H
#define PSOTNIC_STRUCTS_H 1
#include "pstring.h"
class comment;
struct psotnicHeader
{
	char id[8];
	int version;
};
struct unit_table
{
    char unit;
    int  ratio;
};
struct IOBUF
{
	char *buf;
	int pos;
	int len;
};
struct SOCKBUF
{
    int fd;
    char *buf;
    int len;
    int pos;
};
struct EXPANDINFO
{
	pstring<8> system;
	pstring<8> release;
	pstring<8> arch;
	pstring<8> version;
	pstring<8> realname;
};
struct flagTable
{
    char letter;
    unsigned int flag;
    int level;
    unsigned int enforced;
    char *desc;
    char *long_desc;
};
#endif
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
class XSRand
{
    private:
    unsigned int x;
	static unsigned int a, b, c;
    public:
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
	};
	class host2resolv
	{
		public:
		char *host;
		unsigned int hash;
	};
	private:
	hashlist<host2ip> *cache;
	hashlist<host2resolv> *resolving;
	hashlist<host2resolv> *todo;
	bool die;
	int n;
	pthread_t *th;
	public:
#ifdef HAVE_DEBUG
#endif
	friend class client;
};
#endif
class dns
{
	struct FOOTER
	{
		short int qtype;
		short int qclass;
	};
	char dnspacket[PACKETSZ];
	int packetlen;
	sockaddr_in ns[2];
	int fd;
	private:
	public:
};
class protmodelist 
{
	public:
	class entry
	{
		public:
		char *mask;
		char *reason;
		char *by;
		time_t expires;
		time_t when;
		bool sticky;
	};
	ptrlist<entry> data;
	int type;
	char mode;
};
class Pchar
{
	public:
	char *data;
	int step;
	int alloced;
	int len;
};
class comment
{
	public:
	class entry
	{
		public:
		char *key;
		char *value;
	};
	ptrlist<entry> data;
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
	};
	private:
	chan *ch;
	grouplist<modeq_ent> data;
	public:
};
class fifo
{
	public:
	int maxEnt;
	static time_t lastFlush;
	int flushDelay;
	ptrlist<pstring<8> > data;
};
class chanuser
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
#ifdef HAVE_ADNS
#endif
#ifdef HAVE_DEBUG
#endif
	void *customData;
#ifdef HAVE_MODULES
	static FUNCTION (*customDataConstructor)(chanuser *me);
	static FUNCTION (*customDataDestructor)(chanuser *me);
#endif
};
class clone_host
{
	public:
	chanuser *user;
	time_t cr;
};
class clone_ident
{
	public:
	chanuser *user;
	time_t cr;
};
class clone_proxy
{
	public:
	chanuser *user;
	time_t cr;
};
class wasoptest
{
	class entry
	{
		public:
		char *mask;
		time_t when;
	};
	public:
	ptrlist<entry> data;
	int TOL;
	time_t since;
};
class masklist_ent
{
	public:
	char *mask;
	time_t expire;
	time_t when;
	char *who;
	bool sent;
};
class masklist
{
	public:
	ptrlist<masklist_ent> masks;
	
};
class inetconn
{
	private:
	foobar *blowfish;
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
#ifdef HAVE_SSL
#endif
	
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
#ifdef HAVE_DEBUG
#endif
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
	entBool DONT_TRUST_OPS;
	entBool PRE_023_FINAL_COMPAT;
    entBool PRE_0211_FINAL_COMPAT;
    
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
	entChattr PROTECT_CHMODES;
	entBool STRICT_BANS;
	entBool CHECK_SHIT_ON_NICK_CHANGE;
	entBool INVITE_ON_UNBAN_REQUEST;
	entBool KEEPOUT;
	entInt IDIOTS;
	
};
class prvset : public options
{
	public:
	entInt debug_show_irc_write;
	entInt debug_show_irc_read;
};
class CONFIG : public options
{
	public:
	pstring<> file;
	entWord nick;
	entWord ident;
	entWord oidentd_cfg;
	entWord nickappend;
	entString realname;
	entIp myipv4;
	entIp vhost;
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
	entBool altuidnick;
	entBool dontfork;
	entIPPH bnc;
	entIPPH router;
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
#ifdef HAVE_ADNS
	entInt resolve_threads;
	entTime domain_ttl;
#endif
	entBool check_shit_on_nick_change;
	int bottype;
};
class CHANLIST
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
	void *customData;
#ifdef HAVE_MODULES
	static FUNCTION (*customDataConstructor)(CHANLIST *me);
	static FUNCTION (*customDataDestructor)(CHANLIST *me);
#endif
};
class chan
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
	
	/* Probabilistics */
	/* Got something */
	/* other */
#ifdef HAVE_ADNS
#endif
	/* Debug */
#ifdef HAVE_DEBUG
#endif
	/* Constructor */
	/* Destruction derby */
	void *customData;
#ifdef HAVE_MODULES
	static FUNCTION (*customDataConstructor)(chan *me);
	static FUNCTION (*customDataDestructor)(chan *me);
#endif
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
	/* Irc channels */
	/* Nick stuff */
	/* Net */
	/* other */
	/* Debug */
#ifdef HAVE_DEBUG
#endif
	/* Constructor */
	/* Destruction derby */
};
class ul
{
	public:
	HANDLE *first, *last;
	//HANDLE noban;
	CHANLIST chanlist[MAX_CHANNELS];
	int users, bots;
	Pchar *ulbuf;
	time_t nextSave;
	protmodelist *protlist[4];
	chanset *dset;
	/* Handle */
	/* `idiots' code */
	/* Host */
	/* Flags */
	
	/* Channels */
	/* other */
	/* Constructor */
	/* Destruction derby */
};
class ptime
{
    public:
	struct timeval tv;
};
#ifdef HAVE_TCL
class tcl
{
    Tcl_Interp *tcl_int;
    public:
    int curid;
    struct timer_t
    {
		char *nick;
		time_t exp;
		int id;
	};
    ptrlist<timer_t> timer;
};
#endif
class penal
{
	private:
	time_t when;
	int penality;
	public:
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
	};
	ptrlist<entry> data;
	int nextConn;
	int count;
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
#ifdef HAVE_DEBUG
#endif
	};
	enum errorTypes {
		ERR_OK = 0,
		ERR_EOF = -1,
		ERR_URL = -2,
		ERR_DST = -3,
		ERR_CONN = -4,
		ERR_HEADER = -5,
		ERR_DST_WRITE = -6,
		ERR_READ = -7
	};
	inetconn www;
	char *data;
	private:
};
class update
{
	public:
	inetconn child;
	int parent;
	pid_t pid;
	pid_t child_pid;
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
		int fromFlags; // flags before change
		int toFlags; //flags after change
		bool global;
#ifdef HAVE_DEBUG
#endif
	};
	ptrlist<entry> data;
};
#endif
