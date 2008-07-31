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

//options::event ent::_event;

/*
 * class options
 */
options::options()
{
	maxVarLen = 0;
}

void options::registerObject(const ent &e)
{
	int i = strlen(e.getName());

	if(i > maxVarLen)
		maxVarLen = i;

	list.addLast(&e);
}


/*
 * options::event
 */

options::event _event;

options::event *options::setVariable(const char *var, const char *value)
{
	ptrlist<ent>::iterator o = list.begin();
	event *e;

	while(o)
	{
		e = o->setValue(var, value);
		if(e)
			return e;

		o++;
	}
	_event.setNotFound("no such variable %s", var);
	return &_event;
}

const char *options::getValue(const char *var)
{
	ptrlist<ent>::iterator o = list.begin();

	while(o)
	{
		if(!strcmp(var, o->getName()))
		    return o->getValue();
		o++;
	}
	return NULL;
}

void options::sendToOwner(const char *owner, const char *var, const char *prefix)
{
	ptrlist<ent>::iterator o = list.begin();
	int i=0;

	while(o)
	{
		if(o->isPrintable() && (!*var || !strncmp(o->getName(), var, strlen(var))))
		{
			net.sendOwner(owner, prefix, ": ", o->print(maxVarLen), NULL);
			++i;
		}

		o++;
	}
}

bool options::parseUser(const char *from, const char *var, const char *value, const char *prefix, const char *prefix2)
{
	if((!value || !*value) && *var != '+' && *var != '-')
	{
		sendToOwner(from, var, prefix);
	}
	else
	{
		options::event *e = setVariable(var, value);

		if(e->ok)
		{
			net.sendCmd(from, prefix2, prefix, " ", var, " ", e->entity->getValue(), NULL);
			net.sendOwner(from, prefix, ": ", (const char *) e->reason, NULL);
			return 1;
		}
		net.sendOwner(from, prefix, ": ", (const char *) e->reason, NULL);

	}
	return 0;
}


void options::reset()
{
	ptrlist<ent>::iterator o = list.begin();

	while(o)
	{
		o->reset();
		o++;
	}
}

void options::sendToFile(inetconn *c, pstring<> prefix)
{
	ptrlist<ent>::iterator i = list.begin();

	while(i)
	{
		if(!i->isDefault() && i->isPrintable())
			c->send((const char *) prefix, " ", i->print(), NULL);
		i++;
	}
}

options::event::event()
{
	notFound = ok = 0;
}

void options::event::setOk(ent *e, const char *format, ...)
{
	char buf[MAX_LEN];
	va_list ap;
	va_start(ap, format);

	vsnprintf(buf, MAX_LEN, format, ap);
	reason = buf;
	ok = 1;
	notFound = 0;

	va_end(ap);

	entity = e;
}

void options::event::setError(ent *e, const char *format, ...)
{
	char buf[MAX_LEN];
	va_list ap;
	va_start(ap, format);

	vsnprintf(buf, MAX_LEN, format, ap);
	ok = 0;
	notFound = 0;
	reason = buf;

	va_end(ap);

	entity = e;
}

void options::event::setError(ent *e)
{
	ok = 0;
	notFound = 0;
	reason = "";
	entity = e;
}

void options::event::setNotFound(const char *format, ...)
{
	char buf[MAX_LEN];
	va_list ap;

	va_start(ap, format);
	vsnprintf(buf, MAX_LEN, format, ap);
	va_end(ap);

	ok = 0;
	notFound = 1;
	reason = buf;
	entity = NULL;
}

#ifdef HAVE_DEBUG
void options::display()
{
	ptrlist<ent>::iterator i = list.begin();

	while(i)
	{
		if(i->isPrintable())
			printf("%s\n", i->print());
		i++;
	}
}
#endif

/*
 * class set
 */

settings::settings()
{
	registerObject(CYCLE_DELAY = entTime("cycle-delay", 0, 60, 10));
	registerObject(REJOIN_DELAY = entTime("rejoin-delay", 0, 60, 0));
	registerObject(REJOIN_FAIL_DELAY = entTime("rejoin-fail-delay", 7, 60, 25));
	registerObject(HUB_CONN_DELAY = entTime("hub-conn-delay", 10, 3600, 20));
	registerObject(IRC_CONN_DELAY = entTime("irc-conn-delay", 10, 3600, 25));
	registerObject(AUTH_TIME = entTime("auth-time", 15, 360, 45));
	registerObject(PRIVATE_CTCP = entBool("private-ctcp", 1));
	registerObject(OPS_PER_MODE = entInt("ops-per-mode", 0, 3, 2));
	registerObject(ASK_FOR_OP_DELAY = entTime("ask-for-op-delay", 1, 60, 4));
	registerObject(GETOP_OP_CHECK = entBool("get-op-check", 1));
	registerObject(CONN_TIMEOUT = entTime("conn-timeout", 16, 600, 180));
	registerObject(KEEP_NICK_CHECK_DELAY = entTime("keep-nick-check-delay", 5, 24*3600, 10));
	registerObject(REMEMBER_OLD_KEYS = entBool("remember-old-keys", 0));
	registerObject(TELNET_OWNERS = entInt("telnet-owners", 0, 2, 1));
	registerObject(MAX_MATCHES = entInt("max-matches", 0, MAX_INT, 20));
	registerObject(PERIP_MAX_SHOWN_CONNS = entInt("perip-max-shown-conns", 0, MAX_INT, 6));
	registerObject(PERIP_BURST_SIZE = entInt("perip-burst-size", 1, MAX_INT, 30));
	registerObject(PERIP_BURST_TIME = entTime("perip-burst-time", 0, MAX_INT, 60));
	registerObject(PERIP_IGNORE_TIME = entTime("perip-ignore-time", 0, MAX_INT, 300));
	registerObject(SYNFLOOD_MAX_CONNS = entInt("synflood-max-conns", 1, MAX_INT, 100));
	registerObject(SYNFLOOD_IGNORE_TIME = entTime("synflood-ignore-time", 0, MAX_INT, 300));
	registerObject(BIE_MODE_BOUNCE_TIME = entTime("bIe-mode-bounce-time", 0, MAX_INT, 3600));
	registerObject(WASOP_CACHE_TIME = entTime("wasop-cache-time", 0, MAX_INT, 3000));
	registerObject(AWAY_TIME = entTime("away-time", 60, MAX_INT, 8*3600));
	registerObject(CHAT_TIME = entTime("chat-time", 60, MAX_INT, 6*3600));
	registerObject(BETWEEN_MSG_DELAY = entTime("between-msg-delay", 10, MAX_INT, 5*60));
	registerObject(RANDOMNESS = entPerc("randomness", -100, 0, -50));
	registerObject(PUBLIC_AWAY = entBool("public-away", 0));
	registerObject(IDENT_CLONES = entInt("ident-clones", 0, MAX_INT, 3));
	registerObject(HOST_CLONES = entInt("host-clones", 0, MAX_INT, 3));
	registerObject(PROXY_CLONES = entInt("proxy-clones", 0, MAX_INT, 4));
	//FIXME: clone-life-time == 0
	registerObject(CLONE_LIFE_TIME = entTime("clone-life-time", 0, MAX_INT, 0));
	registerObject(CRITICAL_BOTS = entInt("critical-bots", 0, MAX_INT, 0));
	registerObject(QUARANTINE_TIME = entTime("quarantine-time", 0, 120, 15));
	registerObject(BACKUP_MODE_DELAY = entTime("backup-mode-delay", 0, 60, 7));
	registerObject(DONT_TRUST_OPS = entInt("dont-trust-ops", 0, 2, 0));
	registerObject(SERVER_LIMIT_TIME = entTime("server-limit-time", 0, MAX_INT, 5));
	registerObject(BOTS_CAN_ADD_SHIT = entBool("bots-can-add-shit", 0));
	registerObject(PRE_023_FINAL_COMPAT = entBool("pre-0.2.3-final-compat", 0));
    registerObject(PRE_0211_FINAL_COMPAT = entBool("pre-0.2.11-final-compat", 0));
	registerObject(PRE_0214_FINAL_COMPAT = entBool("pre-0.2.14-final-compat", 1));
	registerObject(PRE_REV127_COMPAT = entBool("pre-rev127-compat", 1));
}

/*
 * class chanset
 */

chanset::chanset()
{
	registerObject(AOP_BOTS = entInt("aop-bots", 0, MAX_INT, 1));
	registerObject(BOT_AOP_BOTS = entInt("bot-aop-bots", 0, MAX_INT, 1));
	registerObject(BOT_AOP_MODE = entInt("bot-aop-mode", 0, 2, 1));
	registerObject(PUNISH_BOTS = entPerc("punish-bots", -100, 0, -30));
	registerObject(GETOP_BOTS = entInt("getop-bots", 0, MAX_INT, 1));
	registerObject(INVITE_BOTS = entInt("invite-bots", 0, MAX_INT, 2));
	registerObject(GUARDIAN_BOTS = entInt("guardian-bots", 0, MAX_INT, 2));
	registerObject(CHANNEL_CTCP = entBool("channel-ctcp", 1));
	registerObject(ENFORCE_BANS = entInt("enforce-bans", 0, 2, 1));
	registerObject(ENFORCE_LIMITS = entBool("enforce-limits", 0));
	registerObject(STOP_NETHACK = entBool("stop-nethack", 1));
	registerObject(LIMIT = entBool("limit", 1));
    registerObject(LIMIT_TIME = entTime("limit-time", 1, MAX_INT, 120));
	registerObject(LIMIT_TIME_UP = entTime("limit-time-up", 1, MAX_INT, 120));
    registerObject(LIMIT_TIME_DOWN = entTime("limit-time-down", 1, MAX_INT, 30));
    registerObject(LIMIT_OFFSET = entInt("limit-offset", 1, MAX_INT, 5));
	registerObject(LIMIT_BOTS = entInt("limit-bots", 1, 2, 1));
	registerObject(LIMIT_TOLERANCE = entPerc("limit-tolerance", -100, MAX_INT, -50));
	registerObject(OWNER_LIMIT_TIME = entTime("owner-limit-time", 0, MAX_INT, 15));
	registerObject(TAKEOVER = entBool("takeover", 0));
	registerObject(BITCH = entBool("bitch", 1));
	registerObject(WASOPTEST = entBool("wasoptest", 1));
	registerObject(CLONECHECK = entInt("clonecheck", 0, 2, 1));
	registerObject(DYNAMIC_BANS = entBool("dynamic-bans", 1));
	registerObject(DYNAMIC_INVITES = entBool("dynamic-invites", 1));
	registerObject(DYNAMIC_EXEMPTS = entBool("dynamic-exempts", 1));
	registerObject(LOCKDOWN = entBool("lockdown", 0));
	registerObject(LOCKDOWN_TIME = entTime("lockdown-time", 0, MAX_INT, 180));
	registerObject(PROTECT_CHMODES = entInt("protect-chmodes", 0, 2, 2));
	registerObject(MODE_LOCK = entChattr("mode-lock", "+nt"));
	registerObject(STRICT_BANS = entBool("strict-bans", 1));
	registerObject(CHECK_SHIT_ON_NICK_CHANGE = entBool("check-shit-on-nick-change", 0));
	registerObject(INVITE_ON_UNBAN_REQUEST = entBool("invite-on-unban-request", 0));
	registerObject(KEEPOUT = entBool("keepout", 0));
	registerObject(IDIOTS = entInt("idiots", 0, 5, 2));
	registerObject(USER_INVITES = entInt("user-invites", 0, 2, 1));
	registerObject(USER_EXEMPTS = entInt("user-exempts", 0, 2, 1));
	registerObject(USER_REOPS = entInt("user-reops", 0, 2, 1));
}

chanset &chanset::operator=(const chanset &chset)
{
	ptrlist<ent>::iterator a, b;
	a = list.begin();
	b = chset.list.begin();

	while(a && b)
	{
		//printf(">>> setting: %s = %s\n", (const char *) a->getName(), (const char *) b->getValue());
		a->set(b->getValue());
		a++;
		b++;
	}

	return *this;
}


/*
 * class pset
 */

prvset::prvset()
{
	registerObject(debug_show_irc_write = entInt("debug_show_irc_write", 0, 3, 0));
	registerObject(debug_show_irc_read = entInt("debug_show_irc_read", 0, 3, 0));
}

/*
 * class CONFIG
 */
CONFIG::CONFIG()
{
	int i;

	struct passwd *pent = getpwuid(getuid());

	if(!pent)
	{
		printf("[-] Cannot get current user.\n");
		exit(1);
	}

	/* mandatory options */
	registerObject(nick = entWord("nick", 1, 15));

	/* optional options :) */
	registerObject(realname = entString("realname", 1, 255, "Psotnic C++ Edition"));
	registerObject(nickappend = entWord("nickappend", 1, 255, "_-^`|"));
	registerObject(altnick = entWord("altnick", 1, 15));
	registerObject(ident = entWord("ident", 1, 12, pent->pw_name ? pent->pw_name : "nobody"));
	registerObject(oidentd_cfg = entWord("oidentd-config", 1, 255, ""));
	registerObject(handle = entWord("handle", 1, 15, ""));		// = nick

	registerObject(bnc = entHPPH("bnc",
			new entHost("host", entHost::ipv4 | entHost::domain),
			new entInt("port", 1, 65535, 0),
			new entWord("pass", 1, 256)));

	registerObject(router = entHPPH("router",
			new entHost("host", entHost::ipv4 | entHost::domain),
			new entInt("port", 1, 65535, 0),
			new entWord("pass", 1, 256)));

	registerObject(hub = entHub("hub",
			new entHost("host", entHost::ipv4 | entHost::domain | entHost::use_ssl),
			new entInt("port", 1, 65535, 0),
			new entMD5Hash("pass"),
			new entWord("handle", 0, 15)));

	registerObject(alt_storage = entMult("alt"));
	for(i=0; i<MAX_ALTS; ++i)
	{
		registerObject(alt[i] = entHub("alt",
					   new entHost("host", entHost::ipv4 | entHost::domain | entHost::use_ssl),
					   new entInt("port", 1, 65535, 0)));
		alt[i].setDontPrintIfDefault(true);
		alt_storage.add(&alt[i]);
	}

	registerObject(server_storage = entMult("server"));
	for(i=0; i<MAX_SERVERS; ++i)
	{
		registerObject(server[i] = entServer("server",
						new entHost("host", entHost::ipv4 | entHost::ipv6 | entHost::domain
#ifdef HAVE_SSL
																			| entHost::use_ssl
#endif
						),
					  	new entInt("port", 1, 65535, 0),
					  	new entWord("pass", 0, 256)));
		server[i].setDontPrintIfDefault(true);
		server_storage.add(&server[i]);
	}

	registerObject(module_load_storage = entMult("load"));
	for(i=0; i<MAX_MODULES; ++i)
	{
		registerObject(module_load[i] = entLoadModules("load"));
		module_load[i].setDontPrintIfDefault(true);
		module_load_storage.add(&module_load[i]);
	}

	registerObject(module_debugLoad_storage = entMult("debugLoad"));
	for(i=0; i<MAX_MODULES; ++i)
	{
		registerObject(module_debugLoad[i] = entLoadModules("debugLoad", false));
		module_debugLoad[i].setDontPrintIfDefault(true);
		module_debugLoad_storage.add(&module_debugLoad[i]);
	}

	registerObject(myipv4 = entHost("myipv4", entHost::ipv4 | entHost::bindCheck));
	registerObject(vhost = entHost("vhost", entHost::ipv4 | entHost::ipv6 | entHost::bindCheck));
	registerObject(logfile = entWord("logfile", 1, 16));
	registerObject(userlist_file = entWord("userlist", 1, 255));	// = nick
	registerObject(dontfork = entBool("dontfork", 0));
	registerObject(keepnick = entBool("keepnick", 0));
	registerObject(kickreason = entString("kickreason", 1, 255));
	registerObject(limitreason = entString("limitreason", 1, 255));
	registerObject(keepoutreason = entString("keepoutreason", 1, 255));
	registerObject(partreason = entString("partreason", 1, 255));
	registerObject(quitreason = entString("quitreason", 1, 255));
	registerObject(cyclereason = entString("cyclereason", 1, 255));
	registerObject(botnetword = entWord("botnetword", 1, 255, "###\001The\001Psotnic\001Project\001###"));
	registerObject(listenport = entInt("listen", 0, 65535, 0));
#ifdef HAVE_SSL
	registerObject(ssl_listenport = entInt("ssl_listen", 0, 65535, 0));
#endif
	registerObject(ctcptype = entInt("ctcptype", -1, 8, -1));
	registerObject(ownerpass = entMD5Hash("ownerpass"));
#ifdef HAVE_ADNS
	registerObject(resolve_threads = entInt("resolve-threads", 0, 256, 0));
	registerObject(domain_ttl = entTime("domain-ttl", 0, MAX_INT, 2*3600));
#endif
	registerObject(check_shit_on_nick_change = entBool("check-shit-on-nick-change", 0));
}

void CONFIG::polish()
{
	char buf[MAX_LEN];

	if(!userlist_file.getLen())
	{
		snprintf(buf, MAX_LEN, "%s.ul", nick.getValue());
		userlist_file.setValue("userlist", buf);
	}
	if(!handle.getLen())
	    handle.setValue("handle", nick);

	srand();
	if(ctcptype == -1)
		ctcptype.value = (rand() % 5) + 2;

	//lock some options
	botnetword.setReadOnly(true);
	ownerpass.setReadOnly(true);
	ctcptype.setReadOnly(true);
}

options::event *CONFIG::save(bool makeBackup)
{
	inetconn conf;

	if(makeBackup)
	{
		char buf[MAX_LEN];
		snprintf(buf, MAX_LEN, "%s.dec", (const char *) file);
		unlink(buf);

		if(rename(file, buf))
		{
			_event.setError(NULL, "cannot create backup file '%s': %s\n", buf, strerror(errno));
			return &_event;
		}
	}

	if(conf.open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR) < 1)
	{
		_event.setError(NULL, "cannot create crypted config '%s': %s\n", (const char *) file, strerror(errno));
		return &_event;
	}

	conf.enableLameCrypt();

	ptrlist<ent>::iterator i = list.begin();
	while(i)
	{
		if(!i->isDefault() && i->isPrintable())
			conf.send(i->print(), NULL);
		i++;
	}

	_event.setOk(NULL, "config file has been saved");
	return &_event;
}
