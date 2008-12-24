/* repeat module for psotnic

   Detects users who repeat or flood and kicks/bans them.
   It has also a lagcheck.
*/

#include "../prots.h"
#include "../global-var.h"

// configuration


// repeat

// set how many lines in how many seconds are considered as repeat-flood
#define RP_REPEATS 3
#define RP_SECONDS 60

#define RP_KICKREASON "stop repeating"

#define RP_BANREASON "multiple repeat floods"

// ban time (in minutes)
#define RP_BANTIME 60

// lines that should not be counted (e.g. a line that contains only a smiley)
const char *rp_exceptions[]={
    ":?",       // matches ':)', ':P' etc
    ":-?",      // matches ':-)', ':-P' etc
    ";?",
    ";-?"
};

// flood

// enable this feature
#define USE_FLOOD_PROT

#define FL_LINES   8 
#define FL_SECONDS 30

#define FL_KICKREASON "stop flooding"

#define FL_BANREASON "multiple floods"

#define FL_BANTIME 60

/* lag check:
   if the connection to ircserver is lagged, we should disable repeat/flood protection
   there can still be lag between ircserver links
*/

// enable this feature
#define USE_LAGCHECK

// disable protections if lag is bigger than this value (in seconds)
#define MAX_LAG 10

// delay between lag checks (in seconds)
#define LAGCHECK_DELAY 60


// repeaters/flooders will be banned directly (not kicked first)

//#define BAN_DIRECTLY

// repeaters/flooders will be cached for this time (in minutes) and banned if they repeat/flood again
#define RP_FORGET_DELAY 3

// end of configuration

module *m;

#ifdef USE_LAGCHECK
struct _lagcheck
{
    int next;         // time when to perform next check
    int sent;         // time when last check was performed
    bool in_progress; // sent check but didnt get reply yet
    int current_lag;  // current lag
} lagcheck;
#endif

/* In this class every line that an user writes will be saved for a few seconds (RP_SECONDS).
   Every time he repeats, the counter will be increased until the limit is reached.
*/

class repeatcheck
{
public:
    class line
    {
    public:
        line(const char *content)
        {
            this->content=strdup(content);
            count=1;
            timestamp=NOW;
        }

        char *content;
        int count;
        time_t timestamp;
    };

    repeatcheck()
    {
        lines.removePtrs();
    }

    int addLine(const char *content)
    {
        line *l;

        if((l=findLine(content)))
        {
            l->count++;
            return l->count;
        }

        l=new line(content);

        lines.add(l);
        return 1;
    }

    line *findLine(const char *content)
    {
        ptrlist<line>::iterator i;

        for(i=lines.begin(); i; i++)
            if(!strcmp(i->content, content))
                return i;

        return NULL;
    }

    void delExpiredLines()
    {
        ptrlist<line>::iterator i, j;
        
        i=lines.begin();

        while(i)
        {
            j=i;
            j++;

            if(NOW>i->timestamp+RP_SECONDS)
            {
                free(i->content);
                lines.remove(i);
            }

            i=j;
        }
    }

private:
    ptrlist<line> lines;
};

#ifdef USE_FLOOD_PROT
class floodcheck
{
public:
    floodcheck()
    {
        timer=0;
        count=0;
    }

    void increase()
    {
        if(timer==0)
        {
            timer=NOW;
            count=1;
            return;
        }

        if(NOW>=timer+FL_SECONDS) // expired
        {
            timer=NOW;
            count=1;
            return;
        }

        count++;
    }

    int getCount()
    {
        return count;
    }

private:
    int count;
    time_t timer;
};
#endif
class info : public CustomDataObject
{
public:
    info()
    {
        repeat=new repeatcheck();
#ifdef USE_FLOOD_PROT
        flood=new floodcheck();
#endif
        wait=false;
    }

    repeatcheck *repeat;
#ifdef USE_FLOOD_PROT
    floodcheck *flood;
#endif
    bool wait; // wait until the user got kicked before we proceed
};

#ifndef BAN_DIRECTLY
/* In this class repeaters/flooders will be cached for a few minutes (RP_FORGET_DELAY).
   If he should reach the repeat/flood-limit again, he will be banned this time.
*/

class cache : public CustomDataObject
{
public:
    class entry
    {
    public:
        entry(char *ident, char *host)
        {
            this->ident=strdup(ident);
            this->host=strdup(host);
            timestamp=NOW;
        }

        char *ident;
        char *host;
        time_t timestamp;
    };

    
    cache()
    {
        data.removePtrs();
    }

    void add(char *ident, char *host)
    {
        entry *e=new entry(ident, host);
        data.add(e);
    }

    entry *find(char *ident, char *host)
    {
        ptrlist<entry>::iterator i;

        for(i=data.begin(); i; i++)
            if(!strcmp(i->ident, ident) && !strcmp(i->host, host))
                return i;

        return NULL;
    }

    void del(entry *e)
    {
        free(e->ident);
        free(e->host);
        data.remove(e);
    }

    void delExpiredUsers()
    {
        ptrlist<entry>::iterator i, j;

        i=data.begin();

        while(i)
        {
            j=i;
            j++;

            if(NOW>i->timestamp+RP_FORGET_DELAY*60)
                del(i);

            i=j;
        }
    }

private:
    ptrlist<entry> data;
};
#endif

#ifdef USE_FLOOD_PROT
void detect_flood(chanuser *, chan *);
#endif
void detect_repeat(chanuser *, chan *, const char *);

void hook_privmsg_notice(const char *from, const char *to, const char *msg)
{
    chan *ch;
    chanuser *cu;

#ifdef USE_LAGCHECK
    if(!strcasecmp(from, ME.mask) && !strcasecmp(to, ME.nick) && !strcmp(msg, "RP_LAGCHECK"))
    {
        lagcheck.current_lag=NOW-lagcheck.sent;
        lagcheck.next=NOW+LAGCHECK_DELAY;
        lagcheck.in_progress=false;
        return;
    }

    if(lagcheck.current_lag>MAX_LAG)
        return;
#endif
    if(!(ch=ME.findChannel(to)))
        return;

    if(!(cu=ch->getUser(from)))
        return;

    if(((info*)cu->customData(m->desc))->wait || cu->flags & (HAS_V | HAS_O | IS_OP))
        return;

    detect_repeat(cu, ch, msg);
#ifdef USE_FLOOD_PROT
    detect_flood(cu, ch);
#endif
}

void hook_ctcp(const char *from, const char *to, const char *msg)
{
    char buffer[MAX_LEN];
    chan *ch;
    chanuser *cu;
#ifdef USE_LAGCHECK
    if(lagcheck.current_lag>MAX_LAG)
        return;
#endif 
    if(!(ch=ME.findChannel(to)))
        return;
    
    if(!(ch->me->flags&IS_OP))
        return;
    
    if(!(cu=ch->getUser(from)))
        return;
    
    if(((info*)cu->customData(m->desc))->wait || cu->flags & (HAS_V | HAS_O | IS_OP))
        return;

    if(match("ACTION *", msg))
    {
        strncpy(buffer, msg+7, MAX_LEN);
        detect_repeat(cu, ch, buffer);
#ifdef USE_FLOOD_PROT
        detect_flood(cu, ch);
#endif
    }
}

void hook_timer()
{
    chan *ch;
    ptrlist<chanuser>::iterator u;

    for(ch=ME.first; ch; ch=ch->next)
    {
        for(u=ch->users.begin(); u; u++)
            ((info*)u->customData(m->desc))->repeat->delExpiredLines();
#ifndef BAN_DIRECTLY
        ((cache*)ch->customData(m->desc))->delExpiredUsers();
#endif
    }
#ifdef USE_LAGCHECK
    if(lagcheck.in_progress)
        lagcheck.current_lag=NOW-lagcheck.sent;

    else if(lagcheck.next && NOW>=lagcheck.next)
    {
        ME.privmsg(ME.nick, "RP_LAGCHECK", NULL);
        lagcheck.sent=NOW;
        lagcheck.in_progress=true;
    }
#endif
}

#ifdef USE_LAGCHECK
void hook_connected()
{
    lagcheck.next=NOW+120;
    lagcheck.in_progress=false;
}

void hook_disconnected(const char *reason)
{
    lagcheck.current_lag=0;
    lagcheck.next=0;
    lagcheck.in_progress=false;
}
#endif

#ifdef USE_FLOOD_PROT
void detect_flood(chanuser *cu, chan *ch)
{
    char buffer[MAX_LEN];
    cache::entry *e;
    ((info*)cu->customData(m->desc))->flood->increase();

    if(((info*)cu->customData(m->desc))->flood->getCount()>=FL_LINES)
    {
#ifndef BAN_DIRECTLY
        if((e=((cache*)ch->customData(m->desc))->find(cu->ident, cu->host)))
        {
            ((cache*)ch->customData(m->desc))->del(e);
#endif
            snprintf(buffer, MAX_LEN, "*!%s@%s", cu->ident, cu->host);

            if(!set.BOTS_CAN_ADD_SHIT
                 || !protmodelist::addShit(ch->name, buffer, "repeat", RP_BANTIME*60, RP_BANREASON))
                ch->knockout(cu, FL_BANREASON, FL_BANTIME*60);
#ifndef BAN_DIRECTLY
        }

        else
        {
            ((cache*)ch->customData(m->desc))->add(cu->ident, cu->host);
            cu->setReason(FL_KICKREASON);
            ch->toKick.sortAdd(cu);
        }
#endif
        ((info*)cu->customData(m->desc))->wait=true;
    }
}
#endif
void detect_repeat(chanuser *cu, chan *ch, const char *msg)
{
    char buffer[MAX_LEN];
    cache::entry *e;

    for(unsigned int i=0, size=sizeof(rp_exceptions)/sizeof(rp_exceptions[0]); i<size; i++)
        if(match(rp_exceptions[i], msg))
            return;

    if(((info*)cu->customData(m->desc))->repeat->addLine(msg)>=RP_REPEATS)
    {
#ifndef BAN_DIRECTLY
        if((e=((cache*)ch->customData(m->desc))->find(cu->ident, cu->host)))
        {
            ((cache*)ch->customData(m->desc))->del(e);
#endif
            snprintf(buffer, MAX_LEN, "*!%s@%s", cu->ident, cu->host);

            if(!set.BOTS_CAN_ADD_SHIT
                 || !protmodelist::addShit(ch->name, buffer, "repeat", RP_BANTIME*60, RP_BANREASON))
                ch->knockout(cu, RP_BANREASON, RP_BANTIME*60);
#ifndef BAN_DIRECTLY
        }

        else
        {
            ((cache*)ch->customData(m->desc))->add(cu->ident, cu->host);
            cu->setReason(RP_KICKREASON);
            ch->toKick.sortAdd(cu);
        }
#endif
        ((info*)cu->customData(m->desc))->wait=true;
    }
}

void hook_new_chanuser(chanuser *me)
{
    me->setCustomData(m->desc, new info);
}

void hook_del_chanuser(chanuser *me)
{
    info *cdata=(info *)me->customData(m->desc);

    if(cdata)
    {
        delete cdata;
        me->delCustomData(m->desc);
    }
}

#ifndef BAN_DIRECTLY
void hook_new_chan(chan *me)
{
    me->setCustomData(m->desc, new cache);
}

void hook_del_chan(chan *me)
{
    cache *cdata=(cache *)me->customData(m->desc);

    if(cdata)
    {    
        delete cdata;
        me->delCustomData(m->desc);
    }
}
#endif

// for the case that the module is loaded while the bot is running
void prepareCustomData()
{
    chan *ch;
    ptrlist<chanuser>::iterator u;
 
    for(ch=ME.first; ch; ch=ch->next)
    {
        for(u=ch->users.begin(); u; u++)
            hook_new_chanuser(u); 
#ifndef BAN_DIRECTLY
        hook_new_chan(ch);
#endif
    }
}

extern "C" module *init()
{
    m=new module("repeat", "patrick <patrick@psotnic.com>", "0.2");
    prepareCustomData();
    m->hooks->new_chanuser=hook_new_chanuser;
    m->hooks->del_chanuser=hook_del_chanuser;
    m->hooks->new_chan=hook_new_chan;
    m->hooks->del_chan=hook_del_chan;
    m->hooks->privmsg=hook_privmsg_notice;
    m->hooks->notice=hook_privmsg_notice;
    m->hooks->ctcp=hook_ctcp;
    m->hooks->timer=hook_timer;
#ifdef USE_LAGCHECK
    m->hooks->connected=hook_connected;
    m->hooks->disconnected=hook_disconnected;
    lagcheck.next=0;
    lagcheck.current_lag=0;
    lagcheck.in_progress=false;
#endif
    return m;
}

extern "C" void destroy()
{
    chan *ch;
    ptrlist<chanuser>::iterator u;

    for(ch=ME.first; ch; ch=ch->next)
    {
        for(u=ch->users.begin(); u; u++)
            hook_del_chanuser(u); 
#ifndef BAN_DIRECTLY
        hook_del_chan(ch);
#endif
    }
}
