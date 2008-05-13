/* repeat module for psotnic

   Detects users who repeat or flood and kicks/bans them.
   It has also a lagcheck.

   TODO: -testing ;-)
*/

#include "../prots.h"
#include "../global-var.h"
#include "module.h"

// configuration


// repeat

// set how many lines in how many seconds are considered as repeat-flood
#define RP_REPEATS 3
#define RP_SECONDS 60

#define RP_KICKREASON "stop repeating"

#define RP_BANREASON "multiple repeat floods"

// ban time (in minutes)
#define RP_BANTIME 60

// lines that should not be counted (e.x. a line that contains only a smiley)
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

#define FL_BANREASON "multiple flood"

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



#define RP_BUFSIZE 512

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
            strncpy(this->content, content, RP_BUFSIZE-1);
            this->content[RP_BUFSIZE-1]='\0';
            count=1;
            timestamp=NOW;
            next=NULL;
        }

        char content[RP_BUFSIZE];
        int count;
        time_t timestamp;
        line *next;
    };

    repeatcheck()
    {
        firstLine=NULL;
    }

    int addLine(const char *content)
    {
        line *node, *ptr;

        if((node=findLine(content)))
        {
            node->count++;
            return node->count;
        }

        node=new line(content);

        if(!firstLine)
            firstLine=node;
        else
        {
            for(ptr=firstLine; ptr->next; ptr=ptr->next) ;
            ptr->next=node;
        }

        return 1;
    }

    line *findLine(const char *content)
    {
        for(line *ptr=firstLine; ptr; ptr=ptr->next)
            if(!strcmp(ptr->content, content))
                return ptr;
        return NULL;
    }

    void delLine(const char *content)
    {
        line *ptr, *ptr2;

        if(!strcmp(firstLine->content, content))
        {
            ptr=firstLine;
            firstLine=firstLine->next;
            delete ptr;
        }

        else
        {
            for(ptr=firstLine; ptr && ptr->next; ptr=ptr->next)
            {
                if(!strcmp(ptr->next->content, content))
                {
                    ptr2=ptr->next;
                    ptr->next=ptr2->next;
                    delete ptr2;
                    return;
                }
            }
        }
    }

    void delExpiredLines()
    {
        line *ptr=firstLine, *help;

        while(ptr)
        {
            help=ptr->next;

            if(NOW>ptr->timestamp+RP_SECONDS)
                delLine(ptr->content);

            ptr=help;
        }
    }

private:
    line *firstLine;
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
class info
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

class cache 
{
public:
    class user
    {
        public:
        user(char *ident, char *host)
        {
            strncpy(this->ident, ident, sizeof(this->ident)-1);
            this->ident[sizeof(this->ident)-1]='\0';
            strncpy(this->host, host, sizeof(this->host)-1);
            this->host[sizeof(this->host)-1]='\0';
            timestamp=NOW;
            next=NULL;
        }

        char ident[16];
        char host[256];
        time_t timestamp;
        user *next;
    };

    cache()
    {
        firstUser=NULL;
    }

    void add(char *ident, char *host)
    {
        user *node=new user(ident, host), *ptr;

        if(!firstUser)
            firstUser=node;
        else
        {
            for(ptr=firstUser; ptr->next; ptr=ptr->next) ;
            ptr->next=node;
        }
    }

    bool find(char *ident, char *host)
    {
        for(user *ptr=firstUser; ptr; ptr=ptr->next)
            if(!strcmp(ptr->ident, ident) && !strcmp(ptr->host, host))
                return true;

        return false;
    }

    void del(char *ident, char *host)
    {
        user *ptr, *ptr2;

        if(!strcmp(firstUser->ident, ident) && !strcmp(firstUser->host, host))
        {
            ptr=firstUser;
            firstUser=firstUser->next;
            delete ptr;
        }

        else
        {
            for(ptr=firstUser; ptr && ptr->next; ptr=ptr->next)
            {
                if(!strcmp(ptr->next->ident, ident) && !strcmp(ptr->next->host, host))
                {
                    ptr2=ptr->next;
                    ptr->next=ptr2->next;
                    delete ptr2;
                    return;
                }
            }
        }
    }

    void delExpiredUsers()
    {
        user *ptr=firstUser, *help;

        while(ptr)
        {
            help=ptr->next;

            if(NOW>ptr->timestamp+RP_FORGET_DELAY*60)
                del(ptr->ident, ptr->host);

            ptr=help;
        }
    }

private:
    user *firstUser;
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
    if(!(ch=findChannel(to)))
        return;

    if(!(cu=findUser(from, ch)))
        return;

    if(((info*)cu->customData)->wait || cu->flags & (HAS_V | HAS_O | IS_OP))
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
    if(!(ch=findChannel(to)))
        return;
    
    if(!(ch->me->flags&IS_OP))
        return;
    
    if(!(cu=findUser(from, ch)))
        return;
    
    if(((info*)cu->customData)->wait || cu->flags & (HAS_V | HAS_O | IS_OP))
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
    chanuser *cu;

    for(int i=0; i<MAX_CHANNELS; i++)
    {
        if((ch=findChannel(userlist.chanlist[i].name)))
        {
            for(u=ch->users.begin(); u; u++)
            {
                if((cu=findUser(u->nick, ch)))
                    ((info*)cu->customData)->repeat->delExpiredLines();
            }
#ifndef BAN_DIRECTLY
            ((cache*)ch->customData)->delExpiredUsers();
#endif
        }
    }
#ifdef USE_LAGCHECK
    if(lagcheck.in_progress)
        lagcheck.current_lag=NOW-lagcheck.sent;

    else if(lagcheck.next && NOW>=lagcheck.next)
    {
        privmsg(ME.nick, "RP_LAGCHECK");
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
    ((info*)cu->customData)->flood->increase();

    if(((info*)cu->customData)->flood->getCount()>=FL_LINES)
    {
#ifndef BAN_DIRECTLY
        if(((cache*)ch->customData)->find(cu->ident, cu->host))
        {
            ((cache*)ch->customData)->del(cu->ident, cu->host);
#endif
            knockout(ch, cu, FL_BANREASON, FL_BANTIME*60);
#ifndef BAN_DIRECTLY
        }

        else
        {
            ((cache*)ch->customData)->add(cu->ident, cu->host);
            addKick(ch, cu, FL_KICKREASON);
        }
#endif
        ((info*)cu->customData)->wait=true;
    }
}
#endif
void detect_repeat(chanuser *cu, chan *ch, const char *msg)
{
    for(unsigned int i=0; i<sizeof(rp_exceptions)/sizeof(rp_exceptions[0]); i++)
        if(match(rp_exceptions[i], msg))
            return;

    if(((info*)cu->customData)->repeat->addLine(msg)>=RP_REPEATS)
    {
#ifndef BAN_DIRECTLY
        if(((cache*)ch->customData)->find(cu->ident, cu->host))
        {
            ((cache*)ch->customData)->del(cu->ident, cu->host);
#endif
            knockout(ch, cu, RP_BANREASON, RP_BANTIME*60);
#ifndef BAN_DIRECTLY
        }

        else
        {
            ((cache*)ch->customData)->add(cu->ident, cu->host);
            addKick(ch, cu, RP_KICKREASON);
        }
#endif
        ((info*)cu->customData)->wait=true;
    }
}

void chanuserConstructor(chanuser *me)
{
    me->customData=(void *)new info;
}

void chanuserDestructor(chanuser *me)
{
    if(me->customData)
    {
        delete (info *)me->customData;
        me->customData=NULL;
    }
}

#ifndef BAN_DIRECTLY
void chanConstructor(chan *me)
{
    me->customData=(void *)new cache;
}

void chanDestructor(chan *me)
{
    if(me->customData)
    {
        delete (cache *)me->customData;
        me->customData=NULL;
    }
}
#endif

// for the case that the module is loaded while the bot is running
void prepareCustomData()
{
    chan *ch;
    ptrlist<chanuser>::iterator u;
    chanuser *cu;
 
    for(int i=0; i<MAX_CHANNELS; i++)
    {
        if((ch=findChannel(userlist.chanlist[i].name)))
        {
            for(u=ch->users.begin(); u; u++)
                if((cu=findUser(u->nick, ch)))
                    chanuserConstructor(cu);
#ifndef BAN_DIRECTLY
            chanConstructor(ch);
#endif
        }
    }
}

extern "C" module *init()
{
    module *m=new module("repeat", "patrick <patrick@psotnic.com>", "0.1");
    initCustomData("chanuser", (FUNCTION) chanuserConstructor, (FUNCTION) chanuserDestructor);
#ifndef BAN_DIRECTLY
    initCustomData("chan", (FUNCTION) chanConstructor, (FUNCTION) chanDestructor);
#endif
    prepareCustomData();
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
}
