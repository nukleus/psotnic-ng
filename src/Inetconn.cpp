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

#include <arpa/inet.h> // inet_ntop
#include <cstdarg> // va_*
#include <fcntl.h> // open, for inetconn::open(), enableLameCrypt()
#include <netdb.h> // gethostbyname

#include "blowfish.h"
#include "Client.hpp"
#include "Config.hpp"
#include "global-var.h"
#include "functions.hpp"
#include "Inet.hpp"
#include "Inetconn.hpp"
#include "match.h"
#include "md5.h"
#include "Module.hpp"
#include "Pchar.hpp"
#include "Penal.hpp"
#include "Prvset.hpp"
#include "Settings.hpp"
#include "Userlist.hpp"

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

				HOOK(onDisconnected(reason));
				if(stopParsing)
					stopParsing=false;
				else
					net.send(HAS_N, "[-] Disconnected from server ", net.irc.name, " (", reason, ")", NULL);

			}
			else if(status & STATUS_KLINED)
			{
				HOOK(onKlined(reason));
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
		int ret = 0;

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
