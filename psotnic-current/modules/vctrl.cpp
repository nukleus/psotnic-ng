/* voicecontrol for psotnic
 *
 * This module gives a voiced (+v) user the possibility to use the following 
 * commands:
 * 		!kick <nick> [<reason>]
 * 		!ban <nick> [<reason>]
 *              !banmask <mask>
 * 		!unban <mask>
 * 		!topic <text>
 * 		!voice <nick>
 *		!devoice <nick>
 *
 * you can configure the module on partyline:
 * .bc <bot> vset [<key> [<value>]]
 * .bc <bot> vchanset <channel> [<key> [<value>]]
 *
 * for all channels, use "vchanset *"
 *
 * description of some keys that are not self-explanatory:
 *
 * variable       description
 * --------       -----------
 * notice         send notices (introduction, errors)
 * max-delay      delay to prevent mode flood
 * ban-type	  %n = nick, %i = ident, %h = host.
 *                example: *!%i@%h sets a ban like: *!patrick@psotnic.com
 * required-flag  flag which is required to execute the commands
 *                for command specific restrictions use *-command-required-flag
 *
 * If you want to disable voicecontrol for a channel, type:
 * .bc <bot> vchanset <channel> voicecontrol OFF
 *
 * If you are going to use this module on more than one bot, you
 * should disable 'notice' and set 'max-delay' (e.g. to 10)
 *
 * Thanks to the following people for testing: AnGelZ, Aretino, death, Lu[4, MnEm0nIc, rocks
 *
*/

#include "../prots.h"
#include "../global-var.h"

extern flagTable FT[];

#define VCTRL_CFG_FILE "vctrl.txt"
#define VCTRL_USER_NOT_FOUND "Sorry, this user is not on the channel."

// global settings
class vsettings : public options
{
 public:

    entBool NOTICE;
    entTime MAX_DELAY;
    entString INTRO;
    entTime CFG_SAVE_DELAY;
    entWord BAN_TYPE;
    entBool DONT_KICK_VOICED_USERS;

    vsettings();
};

vsettings::vsettings()
{
    registerObject(NOTICE = entBool("notice", 1));
    registerObject(MAX_DELAY = entTime("max-delay", 0, 60, 0));
    registerObject(INTRO = entString("intro", 1, 255, "Welcome to the control, you can use:"));
    registerObject(CFG_SAVE_DELAY = entTime("cfg-save-delay", 0, 3600, 20));
    registerObject(BAN_TYPE = entWord("ban-type", 1, 255, "*!%i@%h"));
    registerObject(DONT_KICK_VOICED_USERS = entBool("dont-kick-voiced-users", 0));
}

// channel settings
class vchanset : public CustomDataObject, public options
{
 public:

    entBool VOICE_CONTROL;
    entWord REQUIRED_FLAG;
    entBool VOICE_CMD;
    entWord VOICE_CMD_REQUIRED_FLAG;
    entBool DEVOICE_CMD;
    entWord DEVOICE_CMD_REQUIRED_FLAG;
    entBool KICK_CMD;
    entWord KICK_CMD_REQUIRED_FLAG;
    entBool BAN_CMD;
    entWord BAN_CMD_REQUIRED_FLAG;
    entBool BANMASK_CMD;
    entWord BANMASK_CMD_REQUIRED_FLAG;
    entBool UNBAN_CMD;
    entWord UNBAN_CMD_REQUIRED_FLAG;
    entBool TOPIC_CMD;
    entWord TOPIC_CMD_REQUIRED_FLAG;
    entBool USE_TOPIC_PREFIX;
    entString TOPIC_PREFIX;
    entBool USE_TOPIC_APPENDIX;
    entString TOPIC_APPENDIX;

    vchanset();
    ~vchanset();
};

vchanset::vchanset() : CustomDataObject()
{
    registerObject(VOICE_CONTROL = entBool("voicecontrol", 1));
    registerObject(REQUIRED_FLAG = entWord("required-flag", 1, 1, "-"));
    registerObject(VOICE_CMD = entBool("voice-command", 1));
    registerObject(VOICE_CMD_REQUIRED_FLAG = entWord("voice-command-required-flag", 1, 1, "-"));
    registerObject(DEVOICE_CMD = entBool("devoice-command", 1));
    registerObject(DEVOICE_CMD_REQUIRED_FLAG = entWord("devoice-command-required-flag", 1, 1, "-"));
    registerObject(KICK_CMD = entBool("kick-command", 1));
    registerObject(KICK_CMD_REQUIRED_FLAG = entWord("kick-command-required-flag", 1, 1, "-"));
    registerObject(BAN_CMD = entBool("ban-command", 1));
    registerObject(BAN_CMD_REQUIRED_FLAG = entWord("ban-command-required-flag", 1, 1, "-"));
    registerObject(BANMASK_CMD = entBool("banmask-command", 0));
    registerObject(BANMASK_CMD_REQUIRED_FLAG = entWord("banmask-command-required-flag", 1, 1, "-"));
    registerObject(UNBAN_CMD = entBool("unban-command", 1));
    registerObject(UNBAN_CMD_REQUIRED_FLAG = entWord("unban-command-required-flag", 1, 1, "-"));
    registerObject(TOPIC_CMD = entBool("topic-command", 1));
    registerObject(TOPIC_CMD_REQUIRED_FLAG = entWord("topic-command-required-flag", 1, 1, "-"));
    registerObject(USE_TOPIC_PREFIX = entBool("use-topic-prefix", 0));
    registerObject(TOPIC_PREFIX = entString("topic-prefix", 0, 128));
    registerObject(USE_TOPIC_APPENDIX = entBool("use-topic-appendix", 0));
    registerObject(TOPIC_APPENDIX = entString("topic-appendix", 0, 128, "(%n)"));
}

vchanset::~vchanset()
{
}

time_t vctrl_next_save;
vsettings vset;
module *module_info;

struct vctrl_func
{
    const char *command;
    void (*func)(chan *, chanuser *, char *);
    const char *enabled;
    const char *flag;
};

void vctrl_voice(chan *, chanuser *, char *);
void vctrl_devoice(chan *, chanuser *, char *);
void vctrl_kick(chan *, chanuser *, char *);
void vctrl_ban(chan *, chanuser *, char *);
void vctrl_banmask(chan *, chanuser *, char *);
void vctrl_unban(chan *, chanuser *, char *);
void vctrl_topic(chan *, chanuser *, char *);

void vctrl_setSave(void);
void vctrl_load(void);
void vctrl_save(void);
void vctrl_notice(const char *, const char *, ...);
int vctrl_format(char *, size_t, const char *, chanuser *);
int vctrl_get_delay(void);
bool vctrl_check_flag(CHANLIST *, chanuser *, const char *);

// irc command, function, setting to enable/disable command, setting that contains the required flag
struct vctrl_func vctrl_flist[] = {
    { "!voice", vctrl_voice, "voice-command", "voice-command-required-flag" },
    { "!devoice", vctrl_devoice, "devoice-command", "devoice-command-required-flag" },
    { "!kick",  vctrl_kick, "kick-command", "kick-command-required-flag" },
    { "!ban",   vctrl_ban, "ban-command", "ban-command-required-flag" },
    { "!banmask", vctrl_banmask, "banmask-command", "banmask-command-required-flag" },
    { "!unban", vctrl_unban, "unban-command", "unban-command-required-flag" },
    { "!topic", vctrl_topic, "topic-command", "topic-command-required-flag" },
    { NULL,     NULL, NULL, NULL }
};

void vctrl_setSave() { vctrl_next_save=NOW+vset.CFG_SAVE_DELAY; }

void vctrl_load()
{
    FILE *fh;
    char arg[10][MAX_LEN], buffer[MAX_LEN];
    int line=0;
    options::event *e;
    CHANLIST *cl;

    if(!(fh=fopen(VCTRL_CFG_FILE, "r")))
        return;

    while(fgets(buffer, MAX_LEN, fh))
    {
        e=NULL;
        buffer[strlen(buffer)-1]='\0';
        line++;

        str2words(arg[0], buffer, 10, MAX_LEN);
        if(!*arg[0] || arg[0][0]=='#') continue;

        if(!strcmp(arg[0], "vset"))
            e=vset.setVariable(arg[1], rtrim(srewind(buffer, 2)));

        else if(!strcmp(arg[0], "vchanset"))
        {
            if((cl=userlist.findChanlist(arg[1])))
                (vchanset *)cl->customData(module_info->desc)->setVariable(arg[2], rtrim(srewind(buffer, 3)));
        }
	// else ..

        if(e && !e->ok)
            printf("[-] %s:%d: %s\n", VCTRL_CFG_FILE, line, (const char *) e->reason);
    }

    fclose(fh);
    //net.send(HAS_N, "[*] Loading voicecontrol config", NULL);
}

void vctrl_save()
{
    FILE *fh;
    ptrlist<ent>::iterator i;
    int j;

    if(!(fh=fopen(VCTRL_CFG_FILE, "w")))
    {
        net.send(HAS_N, "[\002vctrl\002] cannot open ", VCTRL_CFG_FILE, " for writing: ", strerror(errno), NULL);
        vctrl_setSave(); // try again later
        return;
    }

    for(i=vset.list.begin(); i; i++)
    {
        if(!i->isDefault() && i->isPrintable())
            fprintf(fh, "vset %s\n", i->print());
    }

    for(j=0; j<MAX_CHANNELS; j++)
    {   
        if(userlist.chanlist[j].name)
        {
            for(i=((vchanset *)userlist.chanlist[j].customData(module_info->desc))->list.begin(); i; i++)
            {
                if(!i->isDefault() && i->isPrintable())
                    fprintf(fh, "vchanset %s %s\n", (const char*) userlist.chanlist[j].name, i->print());
            }
        }
    }

    fclose(fh);
    vctrl_next_save=0;
    net.send(HAS_N, "[\002vctrl\002] Autosaving voicecontrol config", NULL);
}

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    char cmd[MAX_LEN], *rest;
    const char *ptr;
    CHANLIST *cl;
    chan *ch;
    chanuser *u;
    struct vctrl_func *fptr;

    if(!(cl=userlist.findChanlist(to)))
        return;

    if(!((vchanset *)cl->customData(module_info->desc))->VOICE_CONTROL) // vctrl is not enabled for this channel
        return;

    if(!(ch=ME.findChannel(to)))       // bot received the msg but is not on channel
        return;

    if(!(ch->me->flags & IS_OP))    // bot is not opped
        return;

    if(!(u=ch->getUser(from)))     // user is not on channel
        return;

    if(!(u->flags & IS_VOICE))      // user is not voiced
        return;

    if(!vctrl_check_flag(cl, u, "required-flag"))
        return;

    str2words(cmd, msg, 1, MAX_LEN, 0);

    for(fptr=vctrl_flist; fptr->command; fptr++)
    {
        if(match(fptr->command, cmd))
        {
            if((ptr=((vchanset *)cl->customData(module_info->desc))->getValue(fptr->enabled)) && !strcmp(ptr, "OFF"))
                return;

            if(fptr->flag)
            {
                if(!vctrl_check_flag(cl, u, fptr->flag))
                    return;
            }

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

void hook_mode(chan *ch, const char (*mode)[MODES_PER_LINE], const char **user, const char *mask)
{
    CHANLIST *cl;
    chanuser *u;
    char buf[MAX_LEN];
    const char *ptr;
    int i;
    struct vctrl_func *fptr;

    if(!(cl=userlist.findChanlist(ch->name)))
        return;

    if(!((vchanset *)cl->customData(module_info->desc))->VOICE_CONTROL)
        return;

    if(!vset.NOTICE)
        return;

    if(!(ch->me->flags & IS_OP))
        return;

    if(!ch->getUser(mask)) // could be a servermode
        return;

    for(i=0; i<MODES_PER_LINE; i++, *user++)
    {
        if(mode[0][i]=='+' && mode[1][i]=='v')
        {
            if(!(u=ch->getUser(*user)))
                continue;

            if(!vctrl_check_flag(cl, u, "required-flag"))
                continue;

            strncpy(buf, (const char*)vset.INTRO, MAX_LEN-1);
            buf[MAX_LEN-1]='\0';

            for(fptr=vctrl_flist; fptr->command; fptr++)
            {
                if((ptr=((vchanset *)cl->customData(module_info->desc))->getValue(fptr->enabled)) && !strcmp(ptr, "OFF"))
                    continue;
                if(!vctrl_check_flag(cl, u, fptr->flag))
                    continue;

                strncat(buf, " ", MAX_LEN-strlen(buf)-1);
                strncat(buf, fptr->command, MAX_LEN-strlen(buf)-1);
            }

            ME.notice(*user, buf, NULL);
        }
    }
}

void vctrl_voice(chan *ch, chanuser *from, char *text)
{
    chanuser *u;

    if(!(text) || (*(text)=='\0'))
    {
        vctrl_notice(from->nick, "syntax: !voice <nick>");
        return;
    }

    if(!(u=ch->getUser(text)))
    {
        vctrl_notice(from->nick, VCTRL_USER_NOT_FOUND);
        return;
    }

    if(u->flags & IS_OP)	// user has +o
    {
        vctrl_notice(from->nick, "This user has +o, he/she does not need +v.");
        return;
    }
	
    if(u->flags & IS_VOICE) // user has +v already
    {
        vctrl_notice(from->nick, "This user has +v already.");
        return;
    }

    ch->modeQ[PRIO_LOW].add(NOW+vctrl_get_delay(), "+v", u->nick);
}

void vctrl_devoice(chan *ch, chanuser *from, char *text)
{
    chanuser *u;

    if(!(text) || (*(text)=='\0'))
    {
        vctrl_notice(from->nick, "syntax: !devoice <nick>");
        return;
    }

    if(!(u=ch->getUser(text)))
    {
        vctrl_notice(from->nick, VCTRL_USER_NOT_FOUND);
        return;
    }

    if(!(u->flags & IS_VOICE))
    {
        vctrl_notice(from->nick, "This user does not have +v.");
        return;
    }

    if(u->flags & (HAS_O | HAS_V | HAS_F)) // added users should not get devoiced
    {
        vctrl_notice(from->nick, "This user is added, i will not take his/her voice.");
        return;
    }

    ch->modeQ[PRIO_LOW].add(NOW+vctrl_get_delay(), "-v", u->nick);
}

void vctrl_kick(chan *ch, chanuser *from, char *text)
{
    char arg[2][MAX_LEN], kickreason[150];
    chanuser *u;

    if(!(text) || (*(text)=='\0'))
    {
        vctrl_notice(from->nick, "syntax: !kick <nick> [<reason>]");
        return;
    }

    str2words(arg[0], text, 2, MAX_LEN, 0);

    if(!(u=ch->getUser(arg[0])))
    {
        vctrl_notice(from->nick, VCTRL_USER_NOT_FOUND);
        return;
    }

    if(u->flags & (IS_OP | HAS_O | HAS_V | HAS_F))	// do not kick ops or added users
    {
        vctrl_notice(from->nick, "I will not kick ops or users who have flags.");
        return;
    }

    if(vset.DONT_KICK_VOICED_USERS && u->flags & IS_VOICE)
    {
        vctrl_notice(from->nick, "I will not kick voiced users.");
        return;
    }

    snprintf(kickreason, sizeof(kickreason), "kicked by %s: %s", from->nick, *arg[1]?srewind(text, 1):"requested");
    u->setReason(kickreason);
    ch->toKick.sortAdd(u);
}

void vctrl_ban(chan *ch, chanuser *from, char *text)
{
    char arg[2][MAX_LEN], buf[MAX_LEN];
    chanuser *u;
    char banmask[MAX_LEN];

    if(!(text) || (*(text)=='\0'))
    {
        vctrl_notice(from->nick, "syntax: !ban <nick> [<reason>]");
        return;
    }

    str2words(arg[0], text, 2, MAX_LEN, 0);

    if(!(u=ch->getUser(arg[0])))
    {
        vctrl_notice(from->nick, VCTRL_USER_NOT_FOUND);
        return;
    }

    if(u->flags & (IS_OP | HAS_O | HAS_V | HAS_F))  // do not kickban ops or added users
    {
        vctrl_notice(from->nick, "I will not kickban ops or users who have flags.");
        return;
    }

    if(vset.DONT_KICK_VOICED_USERS && u->flags & IS_VOICE)
    {
        vctrl_notice(from->nick, "I will not kickban voiced users.");
        return;
    }

    vctrl_format(banmask, MAX_LEN, vset.BAN_TYPE, u);
    ch->modeQ[PRIO_HIGH].add(NOW, "+b", banmask);
    ch->modeQ[PRIO_HIGH].flush(PRIO_HIGH);

    snprintf(buf, MAX_LEN, "banned by %s: %s", from->nick, *arg[1]?srewind(text, 1):"requested");
    u->setReason(buf);
    ch->toKick.sortAdd(u);
}

void vctrl_banmask(chan *ch, chanuser *from, char *text)
{
    char *banmask, *ptr;
    HANDLE *h;
    ptrlist<chanuser>::iterator u;

    if(!(text) || (*(text)=='\0'))
    {
        vctrl_notice(from->nick, "syntax: !banmask <hostmask>");
        return;
    }

    banmask=strdup(text);

    if((ptr=strchr(banmask, ' ')))
        *ptr='\0';

    if(!match("*!*@*", banmask))
    {
        vctrl_notice(from->nick, "Invalid hostmask.");
        return;
    }

    for(u=ch->users.begin(); u; u++)
    {
        if((ch->userLevel(&u)>0 || (u->flags & IS_OP || (vset.DONT_KICK_VOICED_USERS && u->flags & IS_VOICE))) && u->matchesBan(banmask))
        {
            vctrl_notice(from->nick, "This hostmask matches an added%s opped%s user.", vset.DONT_KICK_VOICED_USERS?",":" or", vset.DONT_KICK_VOICED_USERS?" or voiced":"");
            free(banmask);
            return;
        }
    }
 
    for(h=userlist.first; h; h=h->next)
    {
        if(ch->userLevel(h->flags[GLOBAL] | h->flags[ch->channum])>0 && userlist.wildFindHostExtBan(h, banmask) != -1)
        {
            vctrl_notice(from->nick, "This hostmask matches an added user.");
            free(banmask);
            return;
        }
    }

    ch->modeQ[PRIO_LOW].add(NOW, "+b", banmask);
    free(banmask);
}

void vctrl_unban(chan *ch, chanuser *from, char *text)
{
    if(!(text) || (*(text)=='\0'))
    {
        vctrl_notice(from->nick, "syntax: !unban <mask>");
        return;
    }

    ch->modeQ[PRIO_LOW].add(NOW+vctrl_get_delay(), "-b", text);
}

void vctrl_topic(chan *ch, chanuser *from, char *text)
{
    Pchar buffer;
    char buffer2[MAX_LEN];
    CHANLIST *cl;

    if(!(text) || (*(text)=='\0'))
    {
        vctrl_notice(from->nick, "syntax: !topic <text>");
        return;
    }

    if(penalty>=8)
    {
        vctrl_notice(from->nick, "penalty is too high. try again later.");
        return;
    }


    if(!(cl=userlist.findChanlist(ch->name)))
        return;

    if(((vchanset *)cl->customData(module_info->desc))->USE_TOPIC_PREFIX && ((vchanset *)cl->customData(module_info->desc))->TOPIC_PREFIX.getLen() > 0)
    {
        vctrl_format(buffer2, MAX_LEN, ((vchanset *)cl->customData(module_info->desc))->TOPIC_PREFIX, from);
        buffer.push(buffer2);
        buffer.push(" ");
    }

    buffer.push(text);

    if(((vchanset *)cl->customData(module_info->desc))->USE_TOPIC_APPENDIX && ((vchanset *)cl->customData(module_info->desc))->TOPIC_APPENDIX.getLen() > 0)
    {
        vctrl_format(buffer2, MAX_LEN, ((vchanset *)cl->customData(module_info->desc))->TOPIC_APPENDIX, from);
        buffer.push(" ");
        buffer.push(buffer2);
    }

    net.irc.send("TOPIC ", (const char *) ch->name, " :", buffer.data, NULL);
    penalty+=3;
}

// replaces %n by u->nick, %i by u->ident and %h by u->host
int vctrl_format(char *buffer, size_t maxsize, const char *format, chanuser *u)
{
    size_t count = 0;
    unsigned int i;

    size_t nicklen=strlen(u->nick);
    size_t identlen=strlen(u->ident);
    size_t hostlen=strlen(u->host);

    while(1)
    {
        while(*format && *format!='%')
        {
            if (count<maxsize-1)
                buffer[count++]=*format++;
            else
                return 0;
        }

        if(*format=='\0')
            break;

        format++;

        switch(*format)
        {
            case 'n' :
                       for(i=0; i<nicklen; i++)
                       {
                           if(count<maxsize-1)
                               buffer[count++]=u->nick[i];
                           else
                               return 0;
                       }
                       break;

            case 'i' :
                       for(i=0; i<identlen; i++)
                       {
                           if(count<maxsize-1)
                               buffer[count++]=u->ident[i];
                           else
                               return 0;
                       }
                       break;
            case 'h' :
                       for(i=0; i<hostlen; i++)
                       {
                           if(count<maxsize-1)
                               buffer[count++]=u->host[i];
                           else
                               return 0;
                       }
                       break;
            default :
                     if(count<maxsize-2)
                     {
                         buffer[count++]='%';
                         buffer[count++]=*format;
                     }
        }

        if(*format)
           format++;
    }

    buffer[count]='\0';
    return count;
}

void vctrl_notice(const char *to, const char *msg, ...)
{
    char buffer[MAX_LEN];
    va_list list;

    if(!vset.NOTICE)
        return;

    va_start(list, msg);
    vsnprintf(buffer, MAX_LEN, msg, list);
    va_end(list);
    ME.notice(to, buffer, NULL);
}

bool vctrl_check_flag(CHANLIST *cl, chanuser *u, const char *var)
{
    char needed_flag;
    flagTable *ft;
    const char *flagstr;

    if(!var)
        return true;

    if(!cl)
        flagstr=vset.getValue(var);
    else
        flagstr=((vchanset *)cl->customData(module_info->desc))->getValue(var);

    if(!flagstr)
    {
        net.send(HAS_N, "[\002vctrl\002] unknown variable '", var, "'", NULL);
        return true;
    }

    needed_flag=flagstr[0];

    if(needed_flag=='-')
        return true;

    if(!(ft=userlist.findFlagByLetter(needed_flag, FT)))
    {
        net.send(HAS_N, "[\002vctrl\002] unknown flag in variable '", var, "'", cl?" for channel ":"", cl?(const char*)cl->name:"", " (+",  flagstr, ")", NULL);
        return false;
    }

    if(u->flags & ft->flag)
        return true;
    else
        return false;
}
 
int vctrl_get_delay(void)
{
    int num;

    if(vset.MAX_DELAY==0)
        return 0;

    num=1+(int)((double)vset.MAX_DELAY*rand()/(RAND_MAX+1.0));

    return num;
}

void hook_botnetcmd(const char *from, const char *cmd)
{
    char arg[10][MAX_LEN];
    CHANLIST *cl;
    int i;

    str2words(arg[0], cmd, 10, MAX_LEN, 0);

    if(match(arg[1], "vset"))
    {
        if(vset.parseUser(arg[0], arg[2], srewind(cmd, 3), "vset"))
            vctrl_setSave();
    }

    else if(match(arg[1], "vchanset"))
    {
        if(!strcmp(arg[2], "*"))
        {
            for(i=0; i<MAX_CHANNELS; i++)
            {   
                if(userlist.chanlist[i].name)
                {
                    if((vchanset *)userlist.chanlist[i].customData(module_info->desc)->parseUser(arg[0], arg[3], srewind(cmd, 4), userlist.chanlist[i].name))
                        vctrl_setSave();
                }
            }
        }

        else
        {
            if(!(cl=userlist.findChanlist(arg[2])))
            {
                net.sendOwner(arg[0], "unknown channel", NULL);
                return;
            }

            if((vchanset *)cl->customData(module_info->desc)->parseUser(arg[0], arg[3], srewind(cmd, 4), cl->name))
                vctrl_setSave();
        }
    }
}

void hook_timer()
{
    if(vctrl_next_save!=0 && NOW>=vctrl_next_save)
        vctrl_save();
}

void hook_new_CHANLIST(CHANLIST *me)
{
    me->setCustomData(module_info->desc, new vchanset);
}

// load config file here because modules are loaded before userlist
// also chanlists will be rebuilt every time when the bot retrieves a userlist
void hook_userlistLoaded()
{
    vctrl_load();
}

extern "C" module *init()
{
    int i;
    struct timeval tv;
    module_info=new module("voicecontrol", "patrick <patrick@psotnic.com>", "0.4");

    // for the case that the module is loaded by partyline
    if(userlist.SN)
    {
        for(i=0; i<MAX_CHANNELS; i++)
        {
            if(userlist.chanlist[i].name)
                hook_new_CHANLIST(&userlist.chanlist[i]);
        }

        hook_userlistLoaded();
    }

    module_info->hooks->userlistLoaded=hook_userlistLoaded;
    module_info->hooks->privmsg=hook_privmsg;
    module_info->hooks->mode=hook_mode;
    module_info->hooks->botnetcmd=hook_botnetcmd;
    module_info->hooks->timer=hook_timer;
    module_info->hooks->new_CHANLIST=hook_new_CHANLIST;

    gettimeofday(&tv, NULL);
    srand(tv.tv_usec);

    return module_info;
}

extern "C" void destroy()
{
}
