#ifndef INET_HPP
#define INET_HPP 

#include "Inetconn.hpp"

class inet
{
	public:
	int conns, max_conns, listenfd, maxFd;
	inetconn *conn;
	inetconn hub, irc;
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

extern inet net;

#endif /* INET_HPP */
