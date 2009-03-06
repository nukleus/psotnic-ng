#ifndef CONFIG_HPP
#define CONFIG_HPP 

#include "class-ent.h"

/*! Global bot configuration.
 */
class CONFIG : public options
{
	public:
	pstring<> file;
	entWord nick;
	entWord altnick;
	entWord ident;
	entWord oidentd_cfg;
	entWord nickappend;
	entString realname;
	entHost myipv4;
	entHost vhost;
	entWord userlist_file;
	entString kickreason;
	entString quitreason;
	entString limitreason;
	entString keepoutreason;
	entString partreason;
	entString cyclereason;
	entWord botnetword;

	entWord logfile;
	entWord handle;

	entInt listenport;
#ifdef HAVE_SSL
	entInt ssl_listenport;
#endif
	entBool keepnick;
	entInt ctcptype;
	entBool dontfork;

	entHPPH bnc;
	entHPPH router;

	entHub hub;
	entMult alt_storage;
	entHub alt[MAX_ALTS];
	entHub *currentHub;

	entMult server_storage;
	entMult ssl_server_storage;
#ifdef HAVE_SSL
	entServer server[MAX_SERVERS*2];
#else
	entServer server[MAX_SERVERS];
#endif
	entMD5Hash ownerpass;

	entMult module_load_storage;
	entLoadModules module_load[MAX_MODULES];

	entMult module_debugLoad_storage;
	entLoadModules module_debugLoad[MAX_MODULES];

	entString alias[MAX_ALIASES];
	entMult alias_storage;

#ifdef HAVE_ADNS
	entInt resolve_threads;
	entTime domain_ttl;
#endif

	entBool check_shit_on_nick_change;

	int bottype;


	CONFIG();
	void polish();
	void load(const char *file, bool decrypted=false);
	options::event *save(bool decrypted=false);
};

extern CONFIG config;

#endif /* CONFIG_HPP */
