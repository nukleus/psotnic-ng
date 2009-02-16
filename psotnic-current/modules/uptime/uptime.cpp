/* shows system uptime
   usage: !uptime (in channel)
   by default the command is restricted to bot masters (+m), see below.
   supports linux (with proc fs) and *bsd (sysctl).
*/

#include "prots.h"
#include "global-var.h"
#include "module.h"
#include <sys/param.h>

#ifdef linux 
    #define PROC_FS
#else
    #define SYSCTL
    #include <sys/sysctl.h>
#endif

class Uptime : public Module
{
	private:
	bool has_global_flag(chanuser *, int);
	int get_uptime();

	public:
	Uptime( void *, const char *, const char *, time_t, const char * );

	virtual bool onLoad( string &msg );
	virtual void onPrivmsg( const char *from, const char *to, const char *msg );
};

Uptime::Uptime( void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir ) : Module( handle, file, md5sum, loadDate, dataDir )
{
}

bool Uptime::onLoad( string &msg )
{
	return true;
}

void Uptime::onPrivmsg(const char *from, const char *to, const char *msg)
{
    char buffer[MAX_LEN], hostname[MAXHOSTNAMELEN];
    int uptime, days, hours, mins, secs;
    chan *ch;
    chanuser *u;
    struct utsname un;

    if(!(ch=ME.findChannel(to)))
	return;

    if(!(u=ch->getUser(from)))
	return;

    if(!has_global_flag(u, HAS_M))
	return;

    if(match("!uptime", msg))
    {
        if(gethostname(hostname, MAXHOSTNAMELEN)==-1)
        {
            ME.privmsg(to, "error: cannot get hostname", NULL);
            return;
        }

        if(uname(&un)==-1)
        {
            ME.privmsg(to, "error: cannot get uname", NULL);
            return;
        }

        if((uptime=get_uptime())==-1)
        {
            ME.privmsg(to, "error: cannot get uptime", NULL);
            return;
        }

        days=uptime/86400;
        hours=(uptime%86400)/3600;
        mins=(uptime%3600)/60;
        secs=uptime%60;

        snprintf(buffer, MAX_LEN, "uptime of `%s' (running on %s): %d day%s %d hour%s %d min%s %d sec%s",
                 hostname, un.sysname, days, days==1?"":"s", hours, hours==1?"":"s", mins,
                 mins==1?"":"s", secs, secs==1?"":"s");
        ME.privmsg(to, buffer, NULL);
    }
}

bool Uptime::has_global_flag(chanuser *u, int flag)
{
    char mask[MAX_LEN];
    HANDLE *h;

    snprintf(mask, MAX_LEN, "%s!%s@%s", u->nick, u->ident, u->host);

    for(h=userlist.first; h; h=h->next)
    {
	if(!(h->flags[GLOBAL] & flag))
            continue;
		 
        for(int i=0; i<MAX_HOSTS; i++)
            if (h->host[i] && match(h->host[i], mask))
                return true;
    }

    return false;
}

#ifdef SYSCTL
int Uptime::get_uptime()
{
    int mib[2];
    size_t len;
    time_t now;

    struct timeval boottime;

    mib[0]=CTL_KERN;
    mib[1]=KERN_BOOTTIME;
    len=sizeof(boottime);

    if(sysctl(mib, 2, &boottime, &len, NULL, 0)==-1)
        return -1;

    time(&now);
    return (int)now-(int)boottime.tv_sec;
}
#endif

#ifdef PROC_FS
int Uptime::get_uptime()
{
    double proc_uptime;
    FILE *fh;

    if(!(fh=fopen("/proc/uptime", "r")))
        return -1;

    if(fscanf(fh, "%lf %*f", &proc_uptime)!=1)
    {
        fclose(fh);
        return -1;
    }

    fclose(fh);
    return (int)proc_uptime;
}
#endif

MOD_LOAD( Uptime );
MOD_DESC( "Uptime", "Shows system uptime" );
MOD_AUTHOR( "patrick", "patrick@psotnic.com" );
MOD_VERSION( "0.1" )

