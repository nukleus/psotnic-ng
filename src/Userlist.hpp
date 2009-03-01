#ifndef USERLIST_HPP
#define USERLIST_HPP 

struct HANDLE;
class protmodelist;
class chanset;

#include "classes.h"

/*! The bots userlist. */
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

#endif /* USERLIST_HPP */
