/* voicecontrol for psotnic
 *
 * This module gives a voiced (+v) user the possibility to use the following 
 * commands:
 * 		!kick <nick> [reason]
 * 		!ban <nick> [reason]
 * 		!unban <mask>
 * 		!topic <text>
 * 		!voice <nick>
 *		!devoice <nick>
 *
 * If you are going to use this module on more than one bot, you
 * should disable VCTRL_NOTICE and set VCTRL_MAX_DELAY (e.x. to 10)
 *
 * TODO:
 * 		- select channels where the module should work
 *		- get rid of delay
*/

#include "../prots.h"
#include "../global-var.h"
#include "module.h"

// if defined the bot will send notices (introduction, errors)

#define VCTRL_NOTICE

// if defined only users that have +v flag can use the commands

#undef VCTRL_ADDED_ONLY

// delay to prevent mode flood

#define VCTRL_MAX_DELAY 0

#define VCTRL_INTRO "Welcome to the control, you can use:"
#define VCTRL_USER_NOT_FOUND "Sorry, this user is not on the channel."

struct vctrl_func
{
    const char *command;
    void (*func)(chan *, chanuser *, char *);
};

void vctrl_voice(chan *, chanuser *, char *);
void vctrl_devoice(chan *, chanuser *, char *);
void vctrl_kick(chan *, chanuser *, char *);
void vctrl_ban(chan *, chanuser *, char *);
void vctrl_unban(chan *, chanuser *, char *);
void vctrl_topic(chan *, chanuser *, char *);

int vctrl_get_delay(void);

// to disable one of the commands comment out the line

struct vctrl_func vctrl_flist[] = {
    { "!voice", vctrl_voice },
    { "!devoice", vctrl_devoice },
    { "!kick",  vctrl_kick  },
    { "!ban",   vctrl_ban   },
    { "!unban", vctrl_unban },
    { "!topic", vctrl_topic },
    { NULL,     NULL        }
};

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    char cmd[MAX_LEN], *rest;
    chan *ch;
    chanuser *u;
    struct vctrl_func *fptr;

    if(!(ch=findChannel(to)))       // bot received the msg but is not on channel
        return;

    if(!(ch->me->flags & IS_OP))    // bot is not opped
        return;

    if(!(u=findUser(from, ch)))     // user is not on channel
        return;

    if(!(u->flags & IS_VOICE))      // user is not voiced
        return;

#ifdef VCTRL_ADDED_ONLY
   if(!(u->flags & (HAS_V | HAS_O)))
        return;
#endif

    str2words(cmd, msg, 1, MAX_LEN, 0);

    for(fptr=vctrl_flist; fptr->command; fptr++)
    {
        if(match(fptr->command, cmd))
        {
            rest=srewind(msg, 1);

            if(rest && *rest)
            {
                // remove all spaces at the end of the line
                for(int i=strlen(rest)-1; i>=0 && rest[i]==' '; i--)
                    rest[i]='\0';
            }

            fptr->func(ch, u, rest);
            return;
        }
    }
}

#ifdef VCTRL_NOTICE
void hook_modeWho(chan *ch, const char (*mode)[MODES_PER_LINE], const char **user, const char *mask)
{
    char buf[MAX_LEN];
    struct vctrl_func *fptr;

    if(!(ch->me->flags & IS_OP))
        return;

    if(!findUser(mask, ch)) // could be a servermode
        return;

    for(int i=0; i<MODES_PER_LINE; i++, *user++)
    {
        if(mode[0][i]=='+' && mode[1][i]=='v')
        {
#ifdef VCTRL_ADDED_ONLY
            chanuser *u=findUser(*user, ch);

            if(!u || !(u->flags & (HAS_V | HAS_O)))
                continue;
#endif
            strncpy(buf, VCTRL_INTRO, MAX_LEN-1);
            buf[MAX_LEN-1]='\0';

            for(fptr=vctrl_flist; fptr->command; fptr++)
            {
                strncat(buf, " ", MAX_LEN-strlen(buf)-1);
                strncat(buf, fptr->command, MAX_LEN-strlen(buf)-1);
            }

            notice(*user, buf);
        }
    }
}
#endif

void vctrl_voice(chan *ch, chanuser *from, char *text)
{
    chanuser *u;

    if(!(text) || (*(text)=='\0'))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "syntax: !voice <nick>");
#endif
        return;
    }

    if(!(u=findUser(text, ch)))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, VCTRL_USER_NOT_FOUND);
#endif
        return;
    }
	
    if(u->flags & IS_OP)	// user has +o
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "This user has +o, he/she does not need +v.");
#endif
        return;
    }
	
    if(u->flags & IS_VOICE) // user has +v already
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "This user has +v already.");
#endif
        return;
    }

    addMode(ch, "+v", u->nick, PRIO_LOW, vctrl_get_delay());
}

void vctrl_devoice(chan *ch, chanuser *from, char *text)
{
    chanuser *u;

    if(!(text) || (*(text)=='\0'))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "syntax: !devoice <nick>");
#endif
        return;
    }

    if(!(u=findUser(text, ch)))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, VCTRL_USER_NOT_FOUND);
#endif
        return;
    }

    if(!(u->flags & IS_VOICE))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "This user does not have +v.");
#endif
        return;
    }

    if(u->flags & (HAS_O | HAS_V | HAS_F)) // added users should not get devoiced
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "This user is added, i will not take his/her voice.");
#endif
        return;
    }

    addMode(ch, "-v", u->nick, PRIO_LOW, vctrl_get_delay());
}

void vctrl_kick(chan *ch, chanuser *from, char *text)
{
    char arg[2][MAX_LEN], kickreason[150];
    chanuser *u;

    if(!(text) || (*(text)=='\0'))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "syntax: !kick <nick> [reason]");
#endif
        return;
    }

    str2words(arg[0], text, 2, MAX_LEN, 0);

    if(!(u=findUser(arg[0], ch)))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, VCTRL_USER_NOT_FOUND);
#endif
        return;
    }

    if(u->flags & (IS_OP | HAS_O | HAS_V | HAS_F))	// do not kick ops or added users
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "I will not kick ops or users who have flags.");
#endif
        return;
    }

    snprintf(kickreason, sizeof(kickreason), "kicked by %s: %s", from->nick, *arg[1]?srewind(text, 1):"requested");
    addKick(ch, u, kickreason);
}

void vctrl_ban(chan *ch, chanuser *from, char *text)
{
    char arg[2][MAX_LEN], buf[MAX_LEN];
    chanuser *u;
	
    if(!(text) || (*(text)=='\0'))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "syntax: !ban <nick> [reason]");
#endif
        return;
    }

    str2words(arg[0], text, 2, MAX_LEN, 0);

    if(!(u=findUser(arg[0], ch)))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, VCTRL_USER_NOT_FOUND);
#endif
        return;
    }

    if(u->flags & (IS_OP | HAS_O | HAS_V | HAS_F))  // do not kickban ops or added users
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "I will not kickban ops or users who have flags.");
#endif
        return;
    }

    snprintf(buf, MAX_LEN, "*!%s@%s", u->ident, u->host);
    addMode(ch, "+b", buf, PRIO_HIGH, 0);
    flushModeQueue(ch, PRIO_HIGH);

    snprintf(buf, MAX_LEN, "banned by %s: %s", from->nick, *arg[1]?srewind(text, 1):"requested");
    addKick(ch, u, buf);
}

void vctrl_unban(chan *ch, chanuser *from, char *text)
{
    if(!(text) || (*(text)=='\0'))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "syntax: !unban <mask>");
#endif
        return;
    }

    addMode(ch, "-b", text, PRIO_LOW, vctrl_get_delay());
}

void vctrl_topic(chan *ch, chanuser *from, char *text)
{
    if(!(text) || (*(text)=='\0'))
    {
#ifdef VCTRL_NOTICE
        notice(from->nick, "syntax: !topic <text>");
#endif
        return;
    }

    // try to prevent flood from several bots / do not set topic twice
    if(strcmp(ch->topic, text)==0)
        return;
	
    setTopic(ch, text);
}

int vctrl_get_delay(void)
{
    int num;

    if(!VCTRL_MAX_DELAY)
        return 0;

    num=1+(int)((double)VCTRL_MAX_DELAY*rand()/(RAND_MAX+1.0));

    return num;
}

extern "C" module *init()
{
    module *m=new module("voicecontrol", "patrick <patrick@psotnic.com>", "0.3");
    struct timeval tv;

    m->hooks->privmsg=hook_privmsg;
#ifdef VCTRL_NOTICE
    m->hooks->modeWho=hook_modeWho;
#endif

    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);

    return m;
}

extern "C" void destroy()
{
}
