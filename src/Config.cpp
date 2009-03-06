

#include <errno.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>

#include "Config.hpp"
#include "class-ent.h"
#include "Inetconn.hpp"
#include "random.hpp"

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

	registerObject(alias_storage = entMult("alias"));
	for(i=0; i<MAX_ALIASES; ++i)
	{
		registerObject(alias[i] = entString("alias", 1, 255));
		alias[i].setDontPrintIfDefault(true);
		alias_storage.add(&alias[i]);
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

	Psotnic::srand();
	if(ctcptype == -1)
		ctcptype.value = (Psotnic::rand() % 5) + 2;

	//lock some options
	botnetword.setReadOnly(true);
	ownerpass.setReadOnly(true);
	ctcptype.setReadOnly(true);
}

options::event *CONFIG::save(bool decrypted)
{
	inetconn conf;

	if(conf.open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR) < 1)
	{
		// FIXME
		_event.setError(NULL, "cannot create crypted config '%s': %s\n", (const char *) file, strerror(errno));
		return &_event;
	}

	if(!decrypted)
		conf.enableLameCrypt();

	ptrlist<ent>::iterator i = list.begin();
	while(i)
	{
		if(!i->isDefault() && i->isPrintable())
			conf.send(i->print(), NULL);
		i++;
	}

	// FIXME
	_event.setOk(NULL, "config file has been saved");
	return &_event;
}
