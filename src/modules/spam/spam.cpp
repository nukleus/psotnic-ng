
#include <regex.h>

#include "global-var.h"
#include "classes.h"
#include "match.h"
#include "module.h"
#include "spam.h"
#include "repeat.h"

Spam::Spam(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir) : Module(handle, file, md5sum, loadDate, dataDir)
{
    prepareCustomData();

    //construct regular expressions
    regcomp(&spamChannel, "#[[:alpha:]]", REG_ICASE | REG_EXTENDED);
    regcomp(&spamOp, "(!|\\s|^)(op|opme|@+)(\\s|$)", REG_ICASE | REG_EXTENDED);
    regcomp(&spamWWW, "http://|www[.*]|ftp://", REG_ICASE | REG_EXTENDED);
}

Spam::~Spam()
{
    chan *ch;
    ptrlist<chanuser>::iterator u;
    for(ch=ME.first; ch; ch=ch->next)
    {
	for (u=ch->users.begin(); u; u++)
	{
	    onDelChanuser(u);
	}
    }
}

bool Spam::onLoad(string &msg)
{
    return true;
}

int Spam::countCrap(const char *str)
{
    int i;
    for(i=0; *str; ++str)
    {
        if(*str == 1 || *str == 2 || *str == 3 || *str == 6 || *str == '\017' || *str == '\037' || *str == '\026' || *str == '\007')
	    ++i;
    }
    return i;
}

/**
 * Hooks
 */
void Spam::onCtcp(const char *from, const char *to, const char *msg)
{
    chan *ch = ME.findChannel(to);
    
    if(ch)
    {
	chanuser *u = ch->getUser(from);
	
	if(u && !(u->flags & (HAS_V | HAS_O | IS_OP | IS_VOICE)))
	{
	    if(match("ACTION *", msg))
	    {
		if(match("*away*", msg) || match("*gone*", msg))
		    ch->knockout(u, "public away - expires in 30 minutes", 60*30);
		else if(match("*back*", msg))
		    ch->knockout(u, "public back from away - expires in 5 minutes", 60*5);
	    }
	    else if(match("DCC SEND *", msg))
		ch->knockout(u, "Dcc spam - expires in 30 minutes", 60*30);
	}
    }
}

void Spam::onPrivmsg(const char *from, const char *to, const char *msg)
{
    static char buf[MAX_LEN];
    chan *ch = ME.findChannel(to);
    if(ch)
    {
	chanuser *u = ch->getUser(from);
	
	
	if(u && !(u->flags & (HAS_V | HAS_O | IS_OP | IS_VOICE)))
	{
	    int crap = countCrap(msg);    
	    repeat *cdata = (repeat*)u->customData( "spam" );
	
	    //big crap
	    if(crap > 4)
	    {
		int t = (crap - 3) * 5;
		snprintf(buf, MAX_LEN, "more then 3 crap chars in a line - expires in %d minutes", t);
		ch->knockout(u, buf, t*60);
	    }
	    //op
	    else if(!regexec(&spamOp, msg, 1, &spamMatch, 0))
	    {
		ch->knockout(u, "dont ask for op - expires in 5 minutes", 60*5);
	    }
	    //www
	    else if( cdata && cdata->creation + 60 >= NOW &&
		!regexec(&spamWWW, msg, 1, &spamMatch, 0))
	    {
		ch->knockout(u, "http://spam.com - expires in 5 minutes", 60*5);
	    }
	    //#channel spam
	    if(!regexec(&spamChannel, msg, 1, &spamMatch, 0))
	    {
		ch->knockout(u, "#channel spam - expires in 1 minute", 60);
	    }
	    //crap
	    else if(crap == 3)
	    {
		ch->knockout(u, "more then 3 crap chars in a line - expires in 1 minute", 60);
	    }
	    //repeat
	    else if( cdata && cdata->hit(msg))
	    {
		u->setReason("Do not repeat yourself!");
		ch->toKick.sortAdd(u);
		ch->kick(u, "Do not repeat yourself!");
	    }
	}
    }
}

/**
 * Custom constructors and destructors
 */
void Spam::onNewChanuser(chanuser *me)
{
	printf( "[M] spam: new chanuser( %s )\n", me->nick );
	me->setCustomData( "spam", new repeat );
}

void Spam::onDelChanuser(chanuser *me)
{
    repeat *cdata = (repeat *) me->customData( "spam" );

    if( cdata )
    {
        delete cdata;
        me->delCustomData( "spam" );
    }
}

void Spam::prepareCustomData()
{
    chan *ch;
    ptrlist<chanuser>::iterator u;

    for(ch=ME.first; ch; ch=ch->next)
    {
        for(u=ch->users.begin(); u; u++)
            onNewChanuser(u);
    }
}

MOD_LOAD( Spam );
MOD_DESC( "Spam", "example #2: antispam" );
MOD_AUTHOR( "Grzegorz Rusin", "pks@irc.pl, gg:0x17f1ceh" );
MOD_VERSION( "0.1.0" );

