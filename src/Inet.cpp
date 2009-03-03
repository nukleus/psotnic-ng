
#include <arpa/inet.h> // inet_ntop
#include <cstdarg>
#include <netdb.h>

#include "functions.hpp"
#include "global-var.h"
#include "Inet.hpp"
#include "match.h"
#include "Userlist.hpp"

/* Network Connection Handling */

/*! Standard constructor, generates a new connection. */
inet::inet()
{
	conn = (inetconn *) malloc(sizeof(inetconn));
	memset(&conn[0], 0, sizeof(inetconn));
	max_conns = 1;
	conns = maxFd = 0;
}

/*! Destructor, closes the connection. */
inet::~inet()
{
	net.hub.close("Going down");
	net.irc.close("Going down");
	if(listenfd)
		killSocket(listenfd);
	listenfd = 0;

#ifdef HAVE_SSL
	if(ssl_listenfd)
		killSocket(ssl_listenfd);
	ssl_listenfd = 0;
#endif

	for(int i=0; i<max_conns; ++i)
		conn[i].close("Going down");

	resize();
}


/*! Find the hub.
 * \return The connection to the hub or NULL if not found.
 */
inetconn *inet::findMainBot()
{
	for(int i=0; i<max_conns; ++i)
		if(net.conn[i].isMain() && net.conn[i].fd)
			return &net.conn[i];

	return NULL;
}

void inet::resize()
{
	if(max_conns <= conns + 1)
	{
		int size = max_conns * 2;
		int i;

		conn = (inetconn *) realloc(conn, size*sizeof(inetconn));
		for(i=max_conns; i<size; ++i) memset(&conn[i], 0, sizeof(inetconn));
		max_conns = size;
		//DEBUG(printf("max_conns(up): %d\n", max_conns));

	}
	else if(conns <= max_conns / 4 && conns > 4)
	{
		int size = max_conns / 2;

		inetconn *old = conn;
		conn = (inetconn *) malloc(size*sizeof(inetconn));
		int i, j;

		for(i=j=0; i<max_conns; ++i)
		{
			if(old[i].fd)
			{
				memcpy(&conn[j], &old[i], sizeof(inetconn));
				++j;
			}
		}
		for(; j<size; ++j)
			memset(&conn[j], 0, sizeof(inetconn));

		free(old);
		max_conns = size;

		//DEBUG(printf("max_conns(down): %d\n", max_conns));
	}
}

inetconn *inet::addConn(int fd)
{
	int i;

	if(fd < 1) return NULL;

	for(i=0; i<max_conns; ++i)
	{
		if(conn[i].fd < 1)
		{
			memset(&conn[i], 0, sizeof(inetconn));
			conn[i].fd = fd;
			conn[i].status = STATUS_CONNECTED;
			++conns;
			if(fd > maxFd) maxFd = fd;
			return &conn[i];
		}
	}
	return NULL;
}

/* connection can only have name when is registered, so this function returns only
   registered connections */
inetconn *inet::findConn(const char *name)
{
	int i;

	if(!conn) return NULL;

	if(hub.fd > 0 && hub.name) if(!strcmp(hub.name, name)) return &hub;
	for(i=0; i<max_conns; ++i)
		if(conn[i].fd > 0 && conn[i].name && !strcmp(conn[i].name, name)) return &conn[i];
	return NULL;
}

inetconn *inet::findConn(const HANDLE *h)
{
	int i;
	if(!conn || !h) return NULL;

	if(hub.handle == h && h) return &hub;
	for(i=0; i<max_conns; ++i)
		if(conn[i].handle == h && h) return &conn[i];
	return NULL;
}

int inet::closeConn(inetconn *c, const char *reason)
{
	int i, pos, fd, red, ret;

	if(!c || !conn) return 0;

	fd = c->fd;
	red = c->status & STATUS_REDIR;

	/* is this right connection? */
	pos = abs(c - &conn[0]);

	//DEBUG(printf("pos: %d\n", pos));

	if ((pos >= 0 && pos < max_conns) || c == &net.hub)
	{
		/* close given connection */
		if(c->isRegBot() && !c->isRedir()) net.propagate(c, S_BQUIT, " ", reason, NULL);
		DEBUG(printf("kill: %s\n", c->name));
		c->_close(reason);
		if(c != &net.hub) --conns;
		ret = 1;
	}
	else ret = 0;

	/* close redir conns */
	if(!red)
	{
		for(i=max_conns-1; i != -1; --i)
		{
			if(conn[i].fd == fd)
			{
				if(ret) net.propagate(&conn[i], S_BQUIT, " ", reason, NULL);
				DEBUG(printf("red kill: %s\n", conn[i].name));
				conn[i]._close(reason);
				--conns;
			}
		}
	}
	return ret;
}

void inet::sendCmd(const char *from, const char *lst, ...)
{
	if(!from) return;
	va_list ap;
	int len;
	int i;
	char buf[MAX_LEN], *a;

	strftime(buf, MAX_LEN, "[\002%H\002:\002%M\002] ", localtime(&NOW));
	a = push(NULL, buf, "\002#\002", from, "\002#\002", " ", NULL);
	va_start(ap, lst);
	len = va_getlen(ap, lst);
	va_end(ap);
	va_start(ap, lst);
	a = va_push(a, ap, lst, len + 1);
	va_end(ap);

	for(i=0; i<max_conns; ++i)
		if(conn[i].fd && conn[i].isRegOwner())
			conn[i].send(a, NULL);
	free(a);
}


void inet::sendCmd(inetconn *c, const char *lst, ...)
{
	if(!c) return;
	va_list ap;
	int i;
	char buf[MAX_LEN], *a;

	strftime(buf, MAX_LEN, "[\002%H\002:\002%M\002] ", localtime(&NOW));
	a = push(NULL, buf, "\002#\002", c->name, "\002#\002", " ", NULL);

	int len;
	va_start(ap, lst);
	len = va_getlen(ap, lst);
	va_end(ap);
	va_start(ap, lst);
	a = va_push(a, ap, lst, len + 1);
	va_end(ap);

	for(i=0; i<max_conns; ++i)
		if(conn[i].fd && conn[i].isRegOwner())
			conn[i].send(a, NULL);
	free(a);
}

void inet::sendexcept(int excp, int who, const char *lst, ...)
{
	va_list ap;
	int i;

	for(i=0; i<max_conns; ++i)
	{
		if(conn[i].fd > 0 && conn[i].status & STATUS_REGISTERED && conn[i].checkFlag(who)
		 && !(conn[i].status & STATUS_REDIR) && conn[i].fd != excp)
		{
			va_start(ap, lst);
			conn[i].va_send(ap, lst);
			va_end(ap);
		}
	}
	if(hub.fd > 0 && hub.fd != excp && hub.isRegBot()  && hub.checkFlag(who))
	{
		va_start(ap, lst);
		net.hub.va_send(ap, lst);
		va_end(ap);
	}

}

void inet::sendOwner(const char *who, const char *lst, ...)
{
	va_list ap;

	if(!who) return;

	if(config.bottype != BOT_MAIN)
	{
		HANDLE *h;

		if(!userlist.first)
			return;

		if(!userlist.first->next)
			return;

		h = userlist.first->next->next;

		while(h)
		{
			if(userlist.isMain(h))
			{
				inetconn *c = findConn(h);
				if(c)
				{
					int len;
					va_start(ap, lst);
					len = va_getlen(ap, lst);
					va_end(ap);

					char *str;

					va_start(ap, lst);
					str = va_push(NULL, ap, lst, len + 1);
					va_end(ap);

					c->send(S_OREDIR, " ", userlist.first->next->name, " ", who, " ", str, NULL);
					free(str);
				}
			}
			h = h->next;
		}
	}
	else
	{
		for(int i=0; i<max_conns; ++i)
			if(conn[i].isRegOwner() && match(who, net.conn[i].handle->name))
			{
				va_start(ap, lst);
				conn[i].va_send(ap, lst);
				va_end(ap);
			}
	}
}

void inet::send(int who, const char *lst, ...)
{
	va_list ap;
	int i;

	for(i=0; i<max_conns; ++i)
	{
		if(conn[i].fd > 0 && conn[i].status & STATUS_REGISTERED && conn[i].checkFlag(who)
		 && !(conn[i].status & STATUS_REDIR))
		{
			va_start(ap, lst);
			conn[i].va_send(ap, lst);
			va_end(ap);
		}
	}

	//owners
	if(who == HAS_N && config.bottype != BOT_MAIN)
	{
		int len;
		va_start(ap, lst);
		len = va_getlen(ap, lst);
		va_end(ap);
		va_start(ap, lst);
		char *str = va_push(NULL, ap, lst, len + 1);
		va_end(ap);
		sendOwner("*", str, NULL);
		free(str);
	}
}

void inet::sendBotListTo(inetconn *c)
{
	int i;

	if(c)
	{
		for(i=0; i<max_conns; ++i)
		{
			if(&conn[i] != c && conn[i].isRegBot())
			{
				c->send(S_FORWARD, " ", conn[i].handle->name, " * ",
					S_BJOIN, " ", conn[i].name, " ", conn[i].origin, NULL);
			}
		}
		if(net.hub.fd && c != &net.hub && net.hub.isRegBot())
			c->send(S_FORWARD, " ", hub.handle->name, " * ",
					S_BJOIN, " ", hub.name, " ", hub.origin, NULL);

	}
}

void inet::propagate(inetconn *from, const char *str, ...)
{
	va_list ap;
	int len;

	char *p = push(NULL, S_FORWARD, " ", from ? from->handle->name : (const char *) config.handle, " * ", NULL);

	va_start(ap, str);
	len = va_getlen(ap, str);
	va_end(ap);

	va_start(ap, str);
	p = va_push(p, ap, str, len + 1);
	va_end(ap);

	net.sendexcept(from ? from->fd : 0, HAS_B, p, NULL);
	free(p);

}

#ifdef HAVE_DEBUG
void inet::display()
{
	int i;

	if(hub.fd) printf("hub: %s\n", hub.name);
	for(i=0; i<max_conns; ++i)
		if(conn[i].fd > 0) printf("conn[%d]: %s\n", i, conn[i].name);
}
#endif

inetconn *inet::findRedirConn(inetconn *c)
{
	int i;

	for(i=0; i<max_conns; ++i)
	{
		if(conn[i].fd == c->fd && conn[i].isRegBot() && !(conn[i].status & STATUS_REDIR))
			return &conn[i];
	}
	if(hub.fd == c->fd && hub.isRegBot()) return &hub;

	return NULL;
}

int inet::bidMaxFd(int fd)
{
	if(fd > maxFd) maxFd = fd;
	return maxFd;
}



int inet::bots()
{
	int num = 0;

	for(int i=0; i<max_conns; ++i)
		if(conn[i].isRegBot()) ++num;

	return num;
}

int inet::owners()
{
	int num = 0;

	for(int i=0; i<max_conns; ++i)
		if(conn[i].isRegOwner()) ++num;

	return num;
}

//synchronious lookup
int inet::gethostbyname(const char *host, char *buf, int protocol)
{
	if(isValidIp(host))
	{
		strncpy(buf, host, MAX_LEN);
		return 1;
	}
#ifdef NO_6DNS
	struct hostent *h = ::gethostbyname(host);
#else
	struct hostent *h = gethostbyname2(host, protocol);
#endif
	if(h)
	{
		if(inet_ntop(protocol, h->h_addr, buf, MAX_LEN))
			return 1;
	}
	return 0;
}


