/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "prots.h"
#include "global-var.h"

char __port[16];

#ifdef HAVE_SSL
SSL_CTX *inet::server_ctx = NULL;
#endif

/* figure out whether or not this system has va_copy. FIXME: this could
   be done by configure */
#ifdef va_copy
#define MY_VA_COPY va_copy
#elif defined(__va_copy)
#define MY_VA_COPY __va_copy
#else
/* must be freebsd <= 4.11 ;-P if they don't have va_copy, then most of the
   time simple assignment will do */
#define MY_VA_COPY(a, b) ((a) = (b))
#endif

/* Network Connection Handling */

inetconn *inet::findMainBot()
{
	for(int i=0; i<max_conns; ++i)
		if(net.conn[i].isMain() && net.conn[i].fd)
			return &net.conn[i];

	return NULL;
}

inet::inet()
{
	conn = (inetconn *) malloc(sizeof(inetconn));
	memset(&conn[0], 0, sizeof(inetconn));
	max_conns = 1;
	conns = maxFd = 0;
}

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

	if(pos >= 0 && pos < max_conns || c == &net.hub)
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

		if(!userlist.first->next->next)
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


/*
 *
 * Specific Internet Connection Handling
 *
 */

inetconn::inetconn()
{
	memset(this, 0, sizeof(inetconn));
}

inetconn::~inetconn()
{
	close();
}

void inetconn::close(const char *reason)
{
	if(fd > 0)
	{
		if(!net.closeConn(this, reason)) _close(reason);
	}
	if(blowfish)
	{
		delete blowfish;
		blowfish = NULL;
	}
}

void inetconn::_close(const char *reason)
{
	/* no sense of killing nothing */
	//if(!fd) return;
	int killhim = !(status & STATUS_REDIR);

	if(status & STATUS_CONNECTED)
	{
		if(this == &net.irc)
		{
			if(status & STATUS_REGISTERED)
			{
				net.propagate(NULL, S_CHNICK, NULL);

				HOOK(disconnected, disconnected(reason));
				if(stopParsing)
					stopParsing=false;
				else
					net.send(HAS_N, "[-] Disconnected from server ", net.irc.name, " (", reason, ")", NULL);

			}
			else if(status & STATUS_KLINED)
			{
				HOOK(klined, klined(reason));
				if(stopParsing)
					stopParsing=false;
				else
					net.send(HAS_N, "[-] I am K-lined on ", net.irc.name, " (", reason, ")", NULL);

			}

			ME.reset();
		}
		else if(this == &net.hub)
		{
			if(net.hub.status &  STATUS_REGISTERED)
			{
				net.propagate(&net.hub, S_BQUIT, " ", reason, NULL);
			}
			else
			{
				++config.currentHub->failures;
				if(config.hub.failures > 0 && config.currentHub != &config.hub)
					--config.hub.failures;
			}

			if(userlist.ulbuf)
			{
				delete userlist.ulbuf;
				userlist.ulbuf = NULL;
			}
		}
		else
		{
			if(status & STATUS_REGISTERED)
			{
				if(status & STATUS_PARTY)
				{
					net.send(HAS_N, "[*] ", handle->name, " has left the partyline (", reason, ")", NULL);
					if(!checkFlag(HAS_N))
						send("[*] ", handle->name, " has left the partyline (", reason, ")", NULL);
				}
				else if(status & STATUS_BOT)
				{
					if(!(status & STATUS_REDIR)) net.send(HAS_N, "[-] Lost connection to ", handle->name, " (", reason, ")", NULL);
				}
			}
			else if(!(status & STATUS_SILENT)) net.send(HAS_N, "[-] Lost ", getPeerIpName(), " / ", getPeerPortName(), " (", reason, ")", NULL);
		}
	}
	else
	{
		if(this == &net.hub)
		{
			++config.currentHub->failures;
			if(config.hub.failures > 0 && config.currentHub != &config.hub)
				--config.hub.failures;
		}
	}


	if(read.buf) free(read.buf);
	if(write.buf) free(write.buf);
	if(name) free(name);
	if(tmpstr) free(tmpstr);
	if(origin) free(origin);
	if(fd && killhim) killSocket(fd);
	if(blowfish) delete blowfish;
#ifdef HAVE_SSL
	if(ssl_ctx) SSL_CTX_free(ssl_ctx);
	if(ssl) SSL_free(ssl);
#endif
	memset(this, 0, sizeof(inetconn));
}

int inetconn::va_send(va_list ap, const char *lst)
{
	int size; 
	char *p, *q;

	va_list ap_copy;
	MY_VA_COPY(ap_copy, ap);
	size = va_getlen(ap_copy, lst) + 3;
	va_end(ap_copy);

	inetconn *conn = status & STATUS_REDIR ? net.findRedirConn(this) : this;
#ifdef HAVE_DEBUG
	if(debug)
	{
		if(!conn)
		{
			printf("### class inet is broken\n");
			printf("### info this->name: %s\n", name);
			printf("### exit(1)\n");
			exit(1);
		}

		if(conn->fd < 1)
		{
			printf("### %s|%s: bad fd\n", (const char *) config.handle, conn->name);
        	conn->close("Bad file descryptor");
			return 0;
		}
	}
#endif

	if(conn->fd < 1) return 0;

	p = va_push(NULL, ap, lst, size);
	size = strlen(p) + 3;

	if(status & STATUS_REDIR)
	{
		q = push(NULL, S_FORWARD, " ", (const char *) config.handle, " ", handle->name, " ", p, "###", NULL);
		free(p);
		p = q;
		size = strlen(p);
	}

	if(conn == &net.irc)
	{
		p[size-3] = '\r';
		p[size-2] = '\n';
		p[size-1] = '\0';
		--size;

		if(pset.debug_show_irc_write & 1
#ifdef HAVE_DEBUG
		|| debug
#endif
		)
			printf("[D] send[%s]: %s", conn->name, p);
		if(pset.debug_show_irc_write & 2)
			net.send(HAS_N, "[D] send[", conn->name ? conn->name : "???", "]: ", p, NULL);
	}
	else
	{
		p[size-3] = '\n';
		p[size-2] = '\0';
		size -= 2;
	}

	if(conn->blowfish)
	{
		char *blowbuf = (char *) malloc(size + 8);
		char *tmp = (char *) malloc(size + 8);
		memset(tmp, 0, size + 8);
		strcpy(tmp, p);
		free(p);
		p = tmp;
		size = conn->blowfish->Encode((unsigned char *) p, (unsigned char *) blowbuf, size);
		DEBUG(printf("[C] send[%s]: %s", conn->name, p));
		free(p);
		p = blowbuf;
	}
	else
	{
		if(status & STATUS_SSL)
		{
			DEBUG(printf("[S] send[%s]: %s", conn->name, p));
		}
		else
		{
			DEBUG(printf("[*] send[%s]: %s", conn->name, p));
		}
	}

	if(conn->write.buf)
	{
		conn->write.buf = (char *) realloc(conn->write.buf, conn->write.len + size);
		memcpy(conn->write.buf + conn->write.len, p, size);
		conn->write.len += size;

	}
	else
	{
		int n;
#ifdef HAVE_SSL
		if(status & STATUS_SSL)
		{
			n = SSL_write(ssl, p, size);
			if(n < 0)
			{
				int ret = SSL_get_error(ssl, n) ;
				if(ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE)
				{
					DEBUG(printf("[D] SSL: write error: want read/write\n"));
				}
			}
			DEBUG(printf("[D] SSL: wrote %d bytes\n", n));
		}
		else
			n = ::write(conn->fd, p, size);
#else
		n = ::write(conn->fd, p, size);
#endif
		if(n == -1)
			n = 0;
		if(n != size)
		{
			conn->write.len = size-n;
			conn->write.pos = 0;
			conn->write.buf = (char *) malloc(size-n);
			memcpy(conn->write.buf, p+n, size-n);

		}
	}

	free(p);
	return size;
}

int inetconn::send(const char *lst, ...)
{
	va_list ap;
	int n;

	va_start(ap, lst);
	n = va_send(ap, lst);
	va_end(ap);
	return n;
}

int inetconn::readln(char *buf, int len, int *ok)
{
	int n, i, ret;

	//repeat:
#ifdef HAVE_SSL
	if((ret = status & STATUS_SSL ? SSL_read(ssl, buf, 1) : ::read(fd, buf, 1)) > 0)
#else
	if((ret = ::read(fd, buf, 1)) > 0)
#endif
	{
		//printf("--- read: 0x%x %c\n", buf[0], buf[0]);
		if(blowfish && !blowfish->smartDecode(buf[0], buf))
			return 0;

		for(i=0; i < (blowfish ? 8 : 1); ++i)
		{
			if(buf[i] == '\n' || buf[i] == '\0')
			{
				if(read.buf)
				{
					da_end:
					n = read.len;
					strncpy(buf, read.buf, n);
					if(n && (buf[n-1] == '\r' || buf[n-1] == '\n'))
						--n;
					
					buf[n] = '\0';
					free(read.buf);
					memset(&read, 0, sizeof(read));
				}
				else
				{
					n = 0;
					buf[0] = '\0';
				}
				
				if(n)
				{
					DEBUG(printf("[%c] read[%s]: %s\n", blowfish ? 'C' : '*', name, buf));

					if(this == &net.irc)
					{
						if(pset.debug_show_irc_read & 1)
							printf("[D] read[%s]: %s\n", name, buf);
						if(pset.debug_show_irc_read & 2)
							net.send(HAS_N, "[D] read[", name ? name : "???", "]: ", buf, NULL);
					}
				}
				
				if(ok)
					*ok = 1;
				
				return n;
			}
			else
			{
				if(!read.buf)
				{
					read.buf = (char *) malloc(len);
					read.buf[0] = buf[i];
					read.len = 1;
				}
				else
				{
					if(read.len == MAX_LEN - 2)
					{
						//buffer overflow attempt
						if(ok)
							*ok = 0;
						return -1;
					}
					read.buf[read.len++] = buf[i];
					
				}
			}
		}
		
		if(ok)
			*ok = 0;
		return 0;
	}
#ifdef HAVE_SSL
	else if(status & STATUS_SSL && ((n = SSL_get_error(ssl, ret)) == SSL_ERROR_WANT_READ || n == SSL_ERROR_WANT_WRITE))
	{
		DEBUG(printf("[D] SSL: read error: want read/write\n"));
		//goto repeat;
		if(ok)
			*ok = 0;
		return 0;
	}
#endif
	else if(read.buf)
		goto da_end;

	if(ok)
		*ok = ret ? 0 : 1;
	
	return -1;
}

int inetconn::checkFlag(int flag, int where)
{
	return handle ? (handle->flags[where] & flag) : 0;
}

int inetconn::isRedir()
{
	return fd > 0 && (status & STATUS_REDIR);
}

int inetconn::isRegBot()
{
	return fd > 0 && (status & STATUS_REGISTERED) && checkFlag(HAS_B);
}

int inetconn::isRegOwner()
{
	return fd > 0 && (status & STATUS_REGISTERED) && checkFlag(HAS_N);
}

int inetconn::isRegUser()
{
	return fd > 0 && (status & STATUS_REGISTERED) && checkFlag(HAS_P);
}

int inetconn::sendPing()
{
	if(!(status & STATUS_REDIR) && (status & STATUS_REGISTERED) && lastPing && NOW - lastPing > set.CONN_TIMEOUT / 2)
	{
		if(this == &net.irc)
		{
			send("ison ", (const char *) ME.nick, NULL);
			penalty++;
		}
		else send(S_FOO, NULL);
		lastPing = NOW;
		return 1;
	}
	return 0;
}

int inetconn::timedOut()
{
	if(status & STATUS_REDIR) return 0;
	if(killTime && killTime <= NOW)
	{
		close("Ping timeout");
		return 1;
	}
	sendPing();
	return 0;
}
int inetconn::enableCrypt(const char *key, int len)
{
	return enableCrypt((const unsigned char *) key, len);
}

int inetconn::enableCrypt(const unsigned char *key, int len)
{
	if(fd < 1) return 0;

#ifdef HAVE_SSL
	if(status & STATUS_SSL)
		return 0;
#endif

	if(blowfish)
	{
		delete blowfish;
		blowfish = NULL;
	}

	blowfish = new CBlowFish;

	if(len == -1) len = strlen((char *) key);
	if(len > 16) blowfish->Initialize(const_cast<unsigned char*>(key), len);
	else
	{
		unsigned char digest[16];
		MD5Hash(digest, (char *) key, len);
		blowfish->Initialize(digest, len);
	}
	return 1;
}

int inetconn::disableCrypt()
{
#ifdef HAVE_SSL
	if(status & STATUS_SSL)
		return 0;
#endif
	
	if(blowfish)
	{
		delete blowfish;
		blowfish = NULL;
		return 1;
	}
	else return 0;
}

#define GETXPEERFUN int (*)(int s, struct sockaddr *name, socklen_t *namelen)

const char *inetconn::getPeerIpName()
{
	return getipstr(fd, getIpVersion(fd), (GETXPEERFUN) getpeername);
}

const char *inetconn::getMyIpName()
{
	return getipstr(fd, getIpVersion(fd), (GETXPEERFUN) getsockname);
}

char *inetconn::getPeerPortName()
{
	return itoa(getport(fd, (GETXPEERFUN) getpeername));
}

char *inetconn::getMyPortName()
{
	return itoa(getport(fd, (GETXPEERFUN) getsockname));
}


unsigned int inetconn::getPeerIp4()
{
	return getip4(fd, (GETXPEERFUN) getpeername);
}

unsigned int inetconn::getMyIp4()
{
	return getip4(fd, (GETXPEERFUN) getsockname);
}

int inetconn::open(const char *pathname, int flags, mode_t mode)
{
	if(mode) fd = ::open(pathname, flags, mode);
	else fd = ::open(pathname, flags);

	if(fd > 0)
	{
		status = STATUS_FILE;
		mem_strcpy(name, getFileName((char *) pathname));
	}
	return fd;
}

/*
	version == 0					- 0.2.2rciles
	version == 1					- 0.2.2rciles
	version == 2					- 0.2.2rc13 (config file)
	version == 2 + STATUS_ULCRYPT	- 0.2.2rc13 (userfile)
*/
int inetconn::enableLameCrypt()
{
	struct stat info;

	if(fd < 1) return 0;

	if(blowfish)
	{
		delete blowfish;
		blowfish = NULL;
	}

	if(!fstat(fd, &info))
	{
		blowfish = new CBlowFish;
		unsigned int version = 3 + (status & STATUS_ULCRYPT);

		psotnicHeader h;

		if(!info.st_size)
		{
			strcpy(h.id, "psotnic");
			h.version = version;
			::write(fd, &h, sizeof(psotnicHeader));
		}
		else
		{
			::read(fd, &h, sizeof(psotnicHeader));
			if(!strcmp(h.id, "psotnic"))
			{

				if(!(status & STATUS_ULCRYPT))
					h.version &= ~STATUS_ULCRYPT;
				version = h.version;

			}
			else
			{
				lseek(fd, 0, SEEK_SET);
				version = 0;
			}
		}

		unsigned char seed[16];
		if(version == (3 + STATUS_ULCRYPT) || version == htonl(3 + STATUS_ULCRYPT))
		{
			gen_ul_seed(seed);
			blowfish->Initialize(seed, 16);
			memset(seed, 0, 16);
		}
		else if(version == 3 || version == htonl(3))
		{
			gen_cfg_seed(seed);
			blowfish->Initialize(seed, 16);
			memset(seed, 0, 16);
		}
		else
		{
			printf("[!] Unsupported file format\n");
			exit(1);
		}

		return 1;
	}
	return 0;
}

int inetconn::isSlave()
{
	return (handle ? ul::isSlave(handle) : 0);
}

int inetconn::isLeaf()
{
	return (handle ? ul::isLeaf(handle) : 0);
}

int inetconn::isMain()
{
	return (handle ? ul::isMain(handle) : 0);
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

void inetconn::echo(int n, char *app)
{
	if(n)
	{
		DEBUG(printf("send[%s]: \\xFF\\xFC\\x01\n", name));
		::write(fd, "\xFF\xFC\x01", 3);
	}
	else
	{
		DEBUG(printf("send[%s]: \\xFF\\xFB\\x01\n", name));
		::write(fd, "\xFF\xFB\x01", 3);
	}

	if(app)
	{
		DEBUG(printf("send[%s]: ^append^\n", name));
		::write(fd, app, strlen(app));
	}
}

int inetconn::isReg()
{
	return status & STATUS_REGISTERED;
}

int inetconn::isConnected()
{
	return status & STATUS_CONNECTED;
}

int inetconn::isBot()
{
	return status & STATUS_BOT;
}

void inetconn::writeBufferedData()
{
	if(write.buf && !(status & STATUS_REDIR) && fd > 0)
	{
		//FIXME: we should write more then 1 byte at a time
#ifdef HAVE_SSL
		int n;
		if(status & STATUS_SSL)
		{
			if((n = SSL_write(ssl, write.buf + write.pos, 1)) == 1)
				++write.pos;
			else if(n < 0)
			{
				int ret = SSL_get_error(ssl, n);
				if(ret == SSL_ERROR_WANT_READ || ret == SSL_ERROR_WANT_WRITE)
				{
					DEBUG(printf("[D] SSL: write error (from buffer): want read/write\n"));
				}
			}
		}
		else
#endif
		if(::write(fd, write.buf + write.pos, 1) == 1)
			++write.pos;
				
		if(write.pos == write.len)
		{
			free(write.buf);
			memset(&write, 0, sizeof(write));
		}
	}
}

#ifdef HAVE_SSL
bool inetconn::enableSSL()
{
	if(status & STATUS_SSL)
		return false;

	status |= STATUS_SSL;
	ssl_ctx = SSL_CTX_new(SSLv23_method());
	if(!ssl_ctx)
		return false;
		
	DEBUG(printf("[D] ssl_ctx: %p\n", (void *) ssl_ctx));
	SSL_CTX_set_mode(ssl_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE);
	ssl = SSL_new(ssl_ctx);
	if(!ssl_ctx)
		return false;

	DEBUG(printf("[D] net.irc.ssl: %p\n", (void *) ssl));
	SSL_set_fd(ssl, fd);

	return true;
}

int inetconn::isSSL()
{
	return status & STATUS_SSL;
}

void inetconn::SSLHandshake()
{
	if(status & STATUS_SSL_HANDSHAKING)
	{
		DEBUG(printf("[D] SSL: %s\n", SSL_state_string_long(ssl)));
		int ret;

		if(status & STATUS_SSL_WANT_CONNECT)
			ret = SSL_connect(ssl);
		else if(status & STATUS_SSL_WANT_ACCEPT)
			ret = SSL_accept(ssl);

		DEBUG(printf("[D] SSL connect: %d\n", ret));
		DEBUG(printf("[D] SSL cipher: %s\n", SSL_get_cipher_name(ssl)));
		DEBUG(printf("[D] SSL state string: %s\n", SSL_state_string_long(ssl)));

		switch(ret)
		{
			case 1:
				killTime = NOW + set.AUTH_TIME;
				status |= STATUS_CONNECTED;
				status &= ~(STATUS_SSL_HANDSHAKING | STATUS_SSL_WANT_ACCEPT | STATUS_SSL_WANT_CONNECT);
				DEBUG(printf("[D] SSL: CONNECTED ;-))))\n"));
				break;
			case 0:
				//ssl conn was closed
				DEBUG(printf("[D] SSL: Other end closed connection\n"));
				close("[D] SSL: handshake terminated");
				break;
			default:
				//check for non blocking specinfig errors;
				switch(SSL_get_error(ssl, ret))
				{
					case SSL_ERROR_WANT_READ:
					case SSL_ERROR_WANT_WRITE:
						//we will read/write later on...
						DEBUG(printf("[D] SSL: want someting\n"));
						break;
					default:
						//error
						DEBUG(printf("[D] SSL: something is wrong... ;/\n"));
						close("SSL handshake error");
				}
		}
	}
}



#endif
