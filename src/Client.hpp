#ifndef CLIENT_HPP
#define CLIENT_HPP 

#include "classes.h" // Server
#include "pstring.h"

class entServer;

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

#endif /* CLIENT_HPP */
