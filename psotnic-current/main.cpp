/***************************************************************************
 *   Copyright (C) 2003-2007 by Grzegorz Rusin                             *
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

ptrlist<module> modules;

time_t NOW;
CONFIG config;
client ME;
settings set;
ul userlist;
prvset pset;
inet net;
char *thisfile;
penal penalty;
EXPANDINFO expandinfo;
ign ignore;
time_t fifo::lastFlush = NOW;
fifo ctcp(10, 3);
fifo invite(5, 3);
QTIsaac<8, int> Isaac;
idle antiidle;
update psotget;
asyn_socks5 socks5;
int hostNotify;
int stopPsotnic = 0;
bool stopParsing = false;

#ifdef HAVE_DEBUG
int debug;

#endif
#ifdef HAVE_IRC_BACKTRACE
char irc_buf[IRC_BUFS][MAX_LEN];
int current_irc_buf = 0;
#endif

int creation;
//int noulimit = 0;

#ifdef HAVE_ADNS
adns *resolver;
#endif

unit_table ut_time[] = {
	{'w', 7*24*3600 },
	{'d', 24*3600 },
    {'h', 3600 },
    {'m', 60 },
    {'s', 1 },
	{ 0 , 0 }
};

unit_table ut_perc[] = {
	{'%', -1 },
	{ 0, 1 },
	{ 0 , 0}
};

#ifdef HAVE_TCL
tcl tclparser;
#endif

extern char **environ;

#ifdef HAVE_DEBUG
void md5_test()
{
	CUSTOM_MD5_CTX ctx;
    unsigned char dig[16];
	int i;
				
    unsigned char dupa[] = "dupa";
			
	MD5Init(&ctx);
	MD5Update(&ctx, dupa, 4);
	MD5Final(dig, &ctx);
								
	for(i=0; i<16; ++i)
		printf("%02x", dig[i]);
	printf("\n");
}
#endif

int main(int argc, char *argv[])
{
//	md5_test();
    
	char buf[MAX_LEN];
	int i, n, ret;
	struct timeval tv;
	time_t last, last_dns, diff;
	fd_set rfd, wfd;
	inetconn *c;

	thisfile = argv[0];

#ifdef HAVE_SSL
	SSL_library_init();
#endif


	signalHandling();
	srand();

	precache();
	parse_cmdline(argc, argv);

#ifdef HAVE_ADNS_PTHREAD
	resolver = new adns_pthread(config.resolve_threads);
#endif

#ifdef HAVE_ADNS_FIREDNS
	resolver = new adns_firedns();
#endif


	userlist.addHandle("idiots", 0, 0, 0, 0, config.handle);
	userlist.first->flags[MAX_CHANNELS] = HAS_D;

	if(config.bottype == BOT_MAIN)
		userlist.addHandle(config.handle, 0, B_FLAGS | HAS_H, 0, 0, config.handle);
	else
		userlist.addHandle(config.handle, 0, B_FLAGS | HAS_P, 0, 0, 0);

	if(config.bottype != BOT_LEAF)
	{
		printf("[*] Loading userlist from `%s'\n", (const char *) config.userlist_file);
		n = userlist.load(config.userlist_file);

		if(!n)
		{
        	if(config.bottype == BOT_MAIN)
			{
				userlist.first->next->flags[MAX_CHANNELS] |= HAS_H;
				printf("[*] Userlist not found, running in owner creation mode\n");
				creation = 1;
			}
			else if(config.bottype == BOT_SLAVE)
				printf("[*] Userlist not found (new slave?)\n");
		}
		else if(n == -1)
		{
			if(config.bottype == BOT_MAIN)
			{
				printf("[-] Userlist is broken, please import it from your backup\n");
				exit(1);
			}
			else if(config.bottype == BOT_SLAVE)
				printf("[*] Userlist is broken, i will fetch it later\n");
		}
		else if(n == 1)
		{
			printf("[+] Userlist loaded (sn: %llu)\n", userlist.SN);
			HOOK(userlistLoaded, userlistLoaded());
			stopParsing=false;
		}
	}

#ifdef HAVE_DEBUG
	if(!debug && !creation) lurk();
#else
	if(!creation) lurk();
#endif

	precache_expand();

	last_dns = last = NOW = time(NULL);
	tv.tv_sec = 1;

	antiidle.init();
	if((rand() % 2) == 0) antiidle.togleStatus();

	/* MAIN LOOP */
	while(!stopPsotnic)
	{
		penalty.calc();
		net.resize();
		if(tv.tv_sec > 1 || tv.tv_sec <= 0) tv.tv_sec = 1;
		tv.tv_usec = 0;

		/* Add sockets to SETs */
		FD_ZERO(&rfd);
		FD_ZERO(&wfd);
		net.maxFd = 0;

	   	if(net.irc.fd && !net.irc.timedOut())
		{
			if(net.irc.status & STATUS_SYNSENT) FD_SET(net.irc.fd, &wfd);
			FD_SET(net.irc.fd, &rfd);
			net.bidMaxFd(net.irc.fd);
		}
	    if(net.hub.fd && !net.hub.timedOut())
	    {
			if(net.hub.status & STATUS_SYNSENT) FD_SET(net.hub.fd, &wfd);
			FD_SET(net.hub.fd, &rfd);
			net.bidMaxFd(net.hub.fd);
		}

		if(config.listenport
#ifdef HAVE_SSL
			 || config.ssl_listenport
#endif
		)
		{
			FD_SET(net.listenfd, &rfd);
			net.bidMaxFd(net.listenfd);
			
#ifdef HAVE_SSL
			FD_SET(net.ssl_listenfd, &rfd);
			net.bidMaxFd(net.ssl_listenfd);
#endif
		
			for(i=0; i<net.max_conns; ++i)
			{
                if(net.conn[i].fd && !(net.conn[i].status & STATUS_REDIR) && !net.conn[i].timedOut())
				{
                    if(net.conn[i].status & STATUS_SYNSENT) FD_SET(net.conn[i].fd, &wfd);
					FD_SET(net.conn[i].fd, &rfd);
					net.bidMaxFd(net.conn[i].fd);
				}
			}
		}

		if(psotget.child.fd)
		{
			FD_SET(psotget.child.fd, &rfd);
			net.bidMaxFd(psotget.child.fd);
		}

		for(i=0; i<net.max_conns; ++i)
		{
			if(net.conn[i].write.buf)
			{
				FD_SET(net.conn[i].fd, &wfd);
				net.bidMaxFd(net.conn[i].fd);
			}
		}

#ifdef HAVE_ADNS_FIREDNS
		net.bidMaxFd(dynamic_cast<adns_firedns*>(resolver)->fillFDSET(&rfd));
#endif

		/* SELECT */
		ret = select(net.maxFd+1, &rfd, &wfd, NULL, &tv);

		NOW = time(NULL);
		diff = NOW - last;

		if(diff > 60 || diff < 0)
		{
			last = NOW;
			//net.send(HAS_N, "[!] Time drift: ", itoa(diff), " seconds, enabling workaround", NULL);
			for(i=0; i<net.max_conns; ++i)
			{
				if(net.conn[i].isRegBot())
				{
					net.conn[i].killTime += diff;
					net.conn[i].lastPing += diff;
				}
			}
			net.irc.killTime += diff;
			net.irc.lastPing += diff;
			net.hub.killTime += diff;
			net.hub.lastPing += diff;
		}


		if(diff > 0)
		{
			last = NOW;
			if(net.irc.status & STATUS_REGISTERED)
				ME.checkQueue();
			userlist.autoSave();
			ignore.expire();
			userlist.protlist[BAN]->expireAll();
			userlist.protlist[INVITE]->expireAll();
			userlist.protlist[EXEMPT]->expireAll();
			userlist.protlist[REOP]->expireAll();

			if(penalty < 5)
			{
				char *inv = invite.flush();
				if(inv)
				{
					ME.inviteRaw(inv);
					free(inv);
				}
				else
				{
					if(ctcp.flush(&net.irc))
						penalty++;
				}
				antiidle.eval();
			}

			ME.newHostNotify();

#ifdef HAVE_TCL
			tclparser.expireTimers();
#endif
			HOOK(timer, timer());
			stopParsing=false;
#ifdef HAVE_ADNS
			if(NOW - last_dns >= 5*60)
			{
				last_dns = NOW;
				resolver->expire(config.domain_ttl, NOW);
			}
#endif
		}

		if(!net.hub.fd && config.bottype != BOT_MAIN && ME.nextConnToHub <= NOW)
		{
			ME.connectToHUB();
			ME.nextConnToHub = NOW + set.HUB_CONN_DELAY;
		}

		if(net.irc.fd)
		{
			 if(ME.nextReconnect <= NOW && ME.nextReconnect)
			 {
				net.irc.close("Reconnecting");
				ME.connectToIRC();
				ME.nextConnToIrc = NOW + set.IRC_CONN_DELAY;
				ME.nextReconnect = 0;
			}
		}
		else if(ME.nextConnToIrc <= NOW)
		{
			ME.connectToIRC();
			ME.nextConnToIrc = NOW + set.IRC_CONN_DELAY;
			ME.nextReconnect = 0;
		}

		if(net.irc.status & STATUS_REGISTERED && ME.nextNickCheck <= NOW && ME.nextNickCheck)
		{
			if(config.keepnick && strcmp(config.nick, ME.nick))
				net.irc.send("NICK ", (const char *) config.nick, NULL);
			ME.nextNickCheck = 0;
		}

		if(ret < 1) continue;

		/* WRITE BUFFER */
		for(i=0; i<net.max_conns; ++i)
		{
			
			c = &net.conn[i];
			
			/*
			if(c->write.buf && !(c->status & STATUS_REDIR) && FD_ISSET(c->fd, &wfd))
			{
				write(c->fd, c->write.buf + c->write.pos++, 1);
				if(c->write.pos == c->write.len)
				{
					free(c->write.buf);
					memset(&c->write, 0, sizeof(c->write));
				}
			}
			*/
			
			if(c->isConnected() && FD_ISSET(c->fd, &wfd))
				c->writeBufferedData();
		}

		if(net.hub.isConnected() && FD_ISSET(net.hub.fd, &wfd))
			net.hub.writeBufferedData();
		
		if(net.irc.isConnected() && FD_ISSET(net.irc.fd, &wfd))
			net.irc.writeBufferedData();
		
		/* READ from IRC */
		if(FD_ISSET(net.irc.fd, &rfd))
		{
			if(net.irc.status & STATUS_SOCKS5_CONNECTED && !(net.irc.status & STATUS_CONNECTED))
			{
				if(read(net.irc.fd, &buf, 1) == 1)
				{
					n = socks5.work(buf[0]);
					DEBUG(printf("work(%x): %d\n", buf[0], n));

					if(n > 0)
					{
						DEBUG(printf("done!\n"));
						net.irc.status = STATUS_SOCKS5 | STATUS_SOCKS5_CONNECTED | STATUS_CONNECTED;
						goto registration;
					}
					else if(n != 0)
						net.irc.close("EOF from client");
				}
				else
					net.irc.close("EOF from client");
			}
			else if(net.irc.isConnected())
			{
#ifdef HAVE_SSL
                do
                {
#endif

#ifdef HAVE_IRC_BACKTRACE			
				n = net.irc.readln(irc_buf[current_irc_buf], MAX_LEN);
				if(n > 0)
				{
					HOOK(rawirc, rawirc(irc_buf[current_irc_buf]));
					if(stopParsing)
						stopParsing=false;
					else
						parse_irc(irc_buf[current_irc_buf]);
					if(++current_irc_buf == IRC_BUFS)
						current_irc_buf = 0;

#else			
				n = net.irc.readln(buf, MAX_LEN);
				if(n > 0)
				{
					HOOK(rawirc, rawirc(buf));
					if(stopParsing)
						stopParsing=false;
					else
						parse_irc(buf);
#endif
				}
				else if(n == -1)
					net.irc.close("EOF from client");
#ifdef HAVE_SSL
                } while(net.irc.ssl && SSL_pending(net.irc.ssl));
#endif

			}
		}

		/* READ from HUB */
 		if(FD_ISSET(net.hub.fd, &rfd))
		{
			if(net.hub.isConnected())
			{
#ifdef HAVE_SSL
                do
                {
#endif

					n = net.hub.readln(buf, MAX_LEN);
					if(n > 0 && net.hub.status & STATUS_CONNECTED)
						parse_hub(buf);
					else if(n == -1)
						net.hub.close("EOF from client");
#ifdef HAVE_SSL
				} while(net.hub.ssl && SSL_pending(net.hub.ssl));
#endif
			}
		}

		/* ACCEPT connections */
		if(FD_ISSET(net.listenfd, &rfd))
			acceptConnection(net.listenfd, false);

#ifdef HAVE_SSL
		if(FD_ISSET(net.ssl_listenfd, &rfd) && acceptConnection(net.ssl_listenfd, true))
			continue;
#endif
				
		/* READ from BOTS and OWNERS */
		if(config.listenport
#ifdef HAVE_SSL
			|| config.ssl_listenport
#endif
		)
		{
			for(i=0; i<net.max_conns; i++)
			{
				c = &net.conn[i];
				if(c->fd > 0 && !(c->status & STATUS_REDIR))
				{
					if(c->status & STATUS_CONNECTED && FD_ISSET(c->fd, &rfd))
					{
#ifdef HAVE_SSL
						do
						{
#endif
							n = c->readln(buf, MAX_LEN);
							if(n > 0)
							{
								if(c->status & STATUS_PARTY)
									parse_owner(c, buf);
								else if(c->status & STATUS_BOT)
									parse_bot(c, buf);
							}
							else if(n == -1)
							{
								DEBUG(printf("[D] read -1 bytes, closing socket\n"));
								c->close("EOF from client");
							}
#ifdef HAVE_SSL
						} while(c->ssl && SSL_pending(c->ssl));
#endif
                	}
//#ifdef HAVE_SSL
//					else if(c->status & STATUS_SSL_HANDSHAKING && (FD_ISSET(c->fd, &rfd) || FD_ISSET(c->fd, &wfd)))
//						c->SSLHandshake();
//#endif
				}
               	//OWNER SYN/ACT
				if(c->fd && ((c->status & (STATUS_SYNSENT | STATUS_PARTY)) == (STATUS_SYNSENT | STATUS_PARTY)) && FD_ISSET(c->fd, &wfd))
				{
                   	c->status = STATUS_CONNECTED | STATUS_PARTY;
					c->tmpint = 1;
					c->send("Enter owner password: ", NULL);
					if(!(c->status & STATUS_SILENT))
						net.send(HAS_N, "[+] Connection to ", c->getPeerIpName(), " port ", c->getPeerPortName(), " established", NULL);
				}
			}
		}

		/* update */
		if(psotget.child.fd && FD_ISSET(psotget.child.fd, &rfd))
		{
			n = psotget.child.readln(buf, MAX_LEN);
			if(n > 0)
				net.send(HAS_N, buf, NULL);
			else if(n == -1)
			{
				psotget.end();
				memset(&psotget, 0, sizeof(update));
			}
		}

		/* Asycnhronius connections */
#ifdef HAVE_SSL
		if(net.hub.fd && net.hub.status & STATUS_SSL_HANDSHAKING)
		{
			if(net.hub.status & STATUS_SYNSENT)
			{
				if(FD_ISSET(net.hub.fd, &wfd))
				{
					DEBUG(printf("[D] SSL: TCP connection has been established\n"));
					net.hub.status &= ~STATUS_SYNSENT;
				}
				else 
					continue;
			}
		
			net.hub.SSLHandshake();
			if(net.hub.status & STATUS_CONNECTED)
			{
				net.hub.status &= ~STATUS_SSL_HANDSHAKING;
				net.hub.tmpint = 1;
                net.hub.killTime = NOW + set.AUTH_TIME;
                net.hub.send(config.botnetword, NULL);
			}
		}
#endif

		/* firedns resolver */
#ifdef HAVE_ADNS_FIREDNS
		dynamic_cast<adns_firedns*>(resolver)->processResultSET(&rfd);
#endif		

		if(net.hub.fd && net.hub.status & STATUS_SYNSENT)
		{
			if(FD_ISSET(net.hub.fd, &wfd))
			{
				net.hub.status = STATUS_CONNECTED;
				net.hub.tmpint = 1;
				net.hub.killTime = NOW + set.AUTH_TIME;
				net.hub.send(config.botnetword, NULL);
				net.hub.enableCrypt((const char *) config.botnetword, strlen(config.botnetword));
			}
        }
#ifdef HAVE_SSL
//		DEBUG(printf("[D] net.irc.fd: %d, STATUS_SSL: %u, STATUS_SSL_HANDSHAKING: %u\n", 
//			net.irc.fd, net.irc.status & STATUS_SSL, net.irc.status & STATUS_SSL_HANDSHAKING));
			
		if(net.irc.fd && net.irc.status & STATUS_SSL_HANDSHAKING)
		{
			if(net.irc.status & STATUS_SYNSENT)
			{
				if(FD_ISSET(net.irc.fd, &wfd))
				{
					DEBUG(printf("[D] SSL: TCP connection has been established\n"));
					net.irc.status &= ~STATUS_SYNSENT;
					//net.irc.status |= STATUS_CONNECTED;
				}
				else 
					continue;
			}
			
			net.irc.SSLHandshake();
			if(net.irc.status & STATUS_CONNECTED)
			{
				net.irc.send("NICK ", (const char *) config.nick, NULL);
	            net.irc.send("USER ", (const char *) config.ident, " 8 * :", (const char *) config.realname, NULL);
				
				net.irc.status &= ~STATUS_SSL_HANDSHAKING;
			}
		}
#endif
		
		if(net.irc.fd && net.irc.status & STATUS_SYNSENT)
		{
			if(FD_ISSET(net.irc.fd, &wfd))
			{
				if(net.irc.status & STATUS_SOCKS5)
				{
					DEBUG(printf("socks = 1\n"));
					net.irc.status = STATUS_SOCKS5_CONNECTED | STATUS_SOCKS5;
					socks5.work(0);
				}
				else if(net.irc.status & STATUS_BNC)
				{
					entServer *srv = client::getRandomServer();

					if(srv)
					{
						net.irc.status = STATUS_CONNECTED | STATUS_BNC;
						net.send(HAS_N, "[+] Connection to BNC established", NULL);

						net.irc.killTime = NOW + set.AUTH_TIME;
						net.irc.send("PASS ", (const char *) config.bnc.getPass(), NULL);
						net.irc.send("NICK ", (const char *) config.nick, NULL);
						net.irc.send("USER ", (const char *) config.ident, " 8 * :", (const char *) config.realname, NULL);
						net.irc.send("VIP ", (const char *) config.vhost, NULL);
						net.irc.send("CONN ", (const char *) srv->getHost().ip, " ", itoa(srv->getPort()), NULL);
					}
					else
					{
						net.send(HAS_N, "[+] Connection to BNC established, but no servers are added", NULL);
						net.irc.close("Config creator is a dumbass");
					}
				}
				else if(net.irc.status & STATUS_ROUTER)
				{
					entServer *srv = client::getRandomServer();

					if(srv)
					{
						net.irc.status = STATUS_CONNECTED | STATUS_ROUTER;
						net.send(HAS_N, "[+] Connection to ROUTER established", NULL);

						net.irc.killTime = NOW + set.AUTH_TIME;
						net.irc.send((const char *) config.router.getPass(), NULL);
						net.irc.send("telnet ", (const char *) srv->getHost().ip, " ", itoa(srv->getPort()), NULL);
						
                        if(net.irc.pass)
                            net.irc.send("PASS ", (const char *) net.irc.pass, NULL);
                        net.irc.send("NICK ", (const char *) config.nick, NULL);
						net.irc.send("USER ", (const char *) config.ident, " 8 * :", (const char *) config.realname, NULL);
                    }
					else
					{
						net.send(HAS_N, "[+] Connection to ROUTER established, but no servers are added", NULL);
						net.irc.close("Config creator is a dumbass");
					}
				}
				else
				{

					net.irc.status = STATUS_CONNECTED;
					registration:
					net.irc.killTime = NOW + set.AUTH_TIME;
					
                    if(net.irc.pass)
                        net.irc.send("PASS ", (const char *) net.irc.pass, NULL);
                    net.irc.send("NICK ", (const char *) config.nick, NULL);
					net.irc.send("USER ", (const char *) config.ident, " 8 * :", (const char *) config.realname, NULL);
	        }
			}
        }
	}

	return 0;
}
