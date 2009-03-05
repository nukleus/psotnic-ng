#ifndef CHAN_HPP
#define CHAN_HPP 

#include "Chanuser.hpp"
#include "Clone.hpp"
#include "Masklist.hpp"

class chanset;
class wasoptest;

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

#endif /* CHAN_HPP */
