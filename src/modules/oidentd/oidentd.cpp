// oidentd spoofing module for psotnic

// - configuration -

// method 1: define this, if you have only one bot on your shell or multiple bots that should use the same ident
#define OS_METHOD1

// method 2: define this, if you have multiple bots on your shell and each bot should use a different ident
//#define OS_METHOD2

// define this, if your oidentd config is not located in $HOME/.oidentd.conf
//#define OS_CONFIG "/home/patrick/.oidentd.conf"

// reconnect if ident set in config and the one the bot actually got are not the same
//#define OS_RECONNECT

// oidentd.conf will be deleted during this period (for method2)
#define OS_TIDY_UP_MIN_DELAY 180
#define OS_TIDY_UP_MAX_DELAY 600

// - end -

#if (defined (OS_METHOD1) && defined (OS_METHOD2)) || (!defined (OS_METHOD1) && !defined (OS_METHOD2))
#error You must define OS_METHOD1 or OS_METHOD2
#endif

#include <pwd.h>
#include <string>

#include "Config.hpp"
#include "global-var.h"
#include "Inet.hpp"
#include "oidentd.hpp"

using std::string;

oidentd::oidentd(void *handle, const char *fileName, const char *md5sum, time_t loadDate, const char *dataDir)
	: Module(handle, fileName, md5sum, loadDate, dataDir)
{
#ifdef OS_CONFIG
	strncpy(oidentd_cfg, OS_CONFIG, MAX_LEN);
#else
	struct passwd *pw;

	if(!(pw=getpwuid(getuid())))
	{
		error = "[\002oidentd\002] getpwuid() failed. oidentd spoofing will not work";
	}
	else
	{
		snprintf(oidentd_cfg, MAX_LEN, "%s/.oidentd.conf", pw->pw_dir);
	}
#endif
	oidentd_tidy_up_time = 0;
}

bool oidentd::onLoad(string &msg)
{
	msg = error;
	return error.empty();
}

void oidentd::onConnecting()
{
    oidentdSpoofing();
}

#ifdef OS_METHOD1
void oidentd::oidentdSpoofing()
{
    FILE *fh;

    if((fh=fopen(oidentd_cfg, "w")))
    {
        fprintf(fh, "global { reply \"%s\" }\n", (const char *) config.ident);
        fclose(fh);
    }

    else
        net.send(HAS_N, "[\002oidentd\002] cannot open '", oidentd_cfg, "'. oidentd spoofing will not work.", NULL);
}
#endif

#ifdef OS_METHOD2
void oidentd::oidentdSpoofing()
{
    FILE *fh;
    char *lport=net.irc.getMyPortName();

    if(!lport || !strcmp(lport, "0"))
    {
        net.send(HAS_N, "[\002oidentd\002] getMyPortName() failed. oidentd spoofing will not work.", NULL);
        return;
    }

    if((fh=fopen(oidentd_cfg, "a")))
    {
        fprintf(fh, "lport %s { reply \"%s\" }\n", lport, (const char *) config.ident);
        fclose(fh);
        oidentd_tidy_up_time=NOW+OS_TIDY_UP_MIN_DELAY+rand()/(RAND_MAX/(OS_TIDY_UP_MAX_DELAY-OS_TIDY_UP_MIN_DELAY)+1);
    }

    else
        net.send(HAS_N, "[\002oidentd\002] cannot open '", oidentd_cfg, "'. oidentd spoofing will not work.", NULL);
}

void oidentd::onTimer()
{
    struct stat sb;

    if(!oidentd_tidy_up_time)
        return;

    if(NOW>=oidentd_tidy_up_time)
    {
        if(stat(oidentd_cfg, &sb)==-1)
            return;

        // do not delete, if the file has been modified lately
        if(NOW-sb.st_mtime < OS_TIDY_UP_MIN_DELAY)
        {
            oidentd_tidy_up_time+=OS_TIDY_UP_MIN_DELAY-(NOW-sb.st_mtime);
            return;
        }

        unlink(oidentd_cfg);
        oidentd_tidy_up_time=0;
    }
}
#endif

#ifdef OS_RECONNECT
void oidentd::onConnected()
{
    if(!match(ME.ident, config.ident))
    {
        net.send(HAS_N, "[\002oidentd\002] I did not get the ident I wanted. reconnecting..", NULL);
        net.irc.send("QUIT :reconnecting", NULL);
        net.irc.close("reconnecting");
        ME.nextConnToIrc=NOW+set.IRC_CONN_DELAY;
    }
}
#endif

MOD_LOAD( oidentd );
MOD_DESC( "oidentd", "oidentd spoofing" );
MOD_AUTHOR( "Patrick", "patrick@psotnic.com" );
MOD_VERSION( "0.1" );

