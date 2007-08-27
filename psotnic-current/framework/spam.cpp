#include "includes/psotnic.h"
#include <regex.h>

class repeat
{
    public:
    pstring<> str;
    time_t when;
    time_t creation;
    
    bool hit(const char *s)
    {
        if(when + 10 >= *NOW && !strcmp(str, s))
	{
	    when = *NOW;
	    str = s;
	    return true;
	}
	str =  s;
	when = *NOW;
	
	return false;
    }
		
    repeat() : when(0), creation(*NOW) { };
    ~repeat() { };
};    

regex_t spamChannel;
regex_t spamWWW;
regex_t spamOp;
    
regmatch_t spamMatch;
    
int countCrap(const char *str)
{
    int i;
    
    for(i=0; *str; ++str)
    {
        if(*str == 1 || *str == 2 || *str == 3 || *str == 6 || *str == '\017' || *str == '\037' || *str == '\026' || *str == '\007')
	    ++i;
        
	return i;
    }
    
}

/**
 * Hooks
 */
void hook_ctcp(const char *from, const char *to, const char *msg)
{
    chan *ch = findChannel(to);
    
    if(ch)
    {
	chanuser *u = findUser(from, ch);
	
	if(u && !(u->flags & (HAS_V | HAS_O | IS_OP | IS_VOICE)))
	{
	    if(match("ACTION *", msg))
	    {
		if(match("*away*", msg) || match("*gone*", msg))
		    knockout(ch, u, "public away - expires in 30 minutes", 60*30);
		else if(match("*back*", msg))
		    knockout(ch, u, "public back from away - expires in 5 minutes", 60*5);
	    }
	    else if(match("DCC SEND *", msg))
		knockout(ch, u, "Dcc spam - expires in 30 minutes", 60*30);
	}
    }
}

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    static char buf[MAX_LEN];
    chan *ch = findChannel(to);
    if(ch)
    {
	chanuser *u = findUser(from, ch);
	
	
	if(u && !(u->flags & (HAS_V | HAS_O | IS_OP | IS_VOICE)))
	{
	    int crap = countCrap(msg);    
	
	    //big crap
	    if(crap > 4)
	    {
		int t = (crap - 3) * 5;
		snprintf(buf, MAX_LEN, "more then 3 crap chars in a line - expires in %d minutes", t);
		knockout(ch, u, buf, t*60);
	    }
	    //op
	    else if(!regexec(&spamOp, msg, 1, &spamMatch, 0))
	    {
		knockout(ch, u, "dont ask for op - expires in 5 minutes", 60*5);
	    }
	    //www
	    else if(((repeat *) u->customData)->creation + 60 >= *NOW &&
		!regexec(&spamWWW, msg, 1, &spamMatch, 0))
	    {
		knockout(ch, u, "http://spam.com - expires in 5 minutes", 60*5);
	    }
	    //#channel spam
	    if(!regexec(&spamChannel, msg, 1, &spamMatch, 0))
	    {
		knockout(ch, u, "#channel spam - expires in 1 minute", 60);
	    }
	    //crap
	    else if(crap == 3)
	    {
		knockout(ch, u, "more then 3 crap chars in a line - expires in 1 minute", 60);
	    }
	    //repeat
	    else if(((repeat *) u->customData)->hit(msg))
	    {
		addKick(ch, u, "Do not repeat yourself!");
		kick(ch,u, "Do not repeat yourself!");
	    }
	    
	    
	}
    }
}

/**
 * Custom constructors and destructors
 */
void chanuserConstructor(chanuser *me)
{
    me->customData = (void *) new repeat;
}

void chanuserDestructor(chanuser *me)
{
    if(me->customData)
    {
	delete (repeat *) me->customData;
	me->customData = NULL;
    }
}

/**
 * Init stuff
 */
extern "C" module *init()
{
    module *m = new module("example #2: antispam", "Grzegorz Rusin <pks@irc.pl, gg:0x17f1ceh>", "0.1.0");
    
    //register hooks
    m->hooks->privmsg = hook_privmsg;
    m->hooks->ctcp = hook_ctcp;
    
    //add custom constructor and destructor for all objects of type `chanuser' and `CHANLIST'
    initCustomData("chanuser", (FUNCTION) chanuserConstructor, (FUNCTION) chanuserDestructor);
    
    //construct regular expressions
    regcomp(&spamChannel, "#[[:alpha:]]", REG_ICASE | REG_EXTENDED);
    regcomp(&spamOp, "(!|\\s|^)(op|opme|@+)(\\s|$)", REG_ICASE | REG_EXTENDED);
    regcomp(&spamWWW, "http://|www[.*]|ftp://", REG_ICASE | REG_EXTENDED);

    return m;
}
	    
extern "C" void destroy()
{
}
