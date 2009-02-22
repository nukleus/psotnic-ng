/* nogarbage module for psotnic

 This module can prevent the following annoying things:

 -colored/bold/underlined/reverse text
 -channel notices/ctcps
 -public away notices
 -excessive usage of capital letters
 -spam (on channel or query)
 -nickflood
 -textflood
 -bad nicks

 the module can be configured by partyline. to see all available options type:
 .bc <bot> ng:*
 or e.x. '.bc <bot> ng:color' to see all options about color protection

 you can change options like this:

 bantime for color protection: .bc <bot> ng:color bantime 10m
 or bantime of everything: .bc <bot> ng:* bantime 10m

 you can add channels by typing: .bc <bot> ng:color +channel #psotnic
 if no channel is added, the module will work on all channels except those
 which were added like: .bc <bot> ng:color +echannel #other

 punish-method can be the following (idea by allprotections.tcl):

 1: warn - kick - kickban 
 2: warn - kick 
 3: warn - kickban 
 4: kick - kickban 
 5: kickban

 there are no patterns for spam and public away msgs added by default.

 to add the one of spam.cpp, type:
 .bc <bot> ng:spam +pattern #[[:alpha:]]
 .bc <bot> ng:spam +pattern (!|\\s|^)(op|opme|@+)(\\s|$)
 .bc <bot> ng:spam +pattern http://|www[.*]|ftp://

 and for public away msgs:
 .bc <bot> ng:pubaway +pattern (away|gone|back)

 if you want that the bot rejoins channels every 10 min:
 .bc <bot> ng:query-spam cycle-delay 10m

 Thanks to the following people for testing: MnEm0nIc
*/

#include "../prots.h"
#include "../global-var.h"
#include <string>
#include <regex.h>

#define MY_CONF "ng.txt"
#define RECORD_EXPIRY 3600
#define NG_WARNED 1
#define NG_KICKED 2
//#define NG_BANNED 4
#define NG_MAX_PATTERNS 10
#define NG_SAVE_DELAY 10

module *m;

class ng_options : public options
{
 public:
    int type;
    std::string name;
    entInt PUNISH_METHOD;
    entTime BANTIME;
    entString WARNMSG;
    entString KICKREASON;
    entString BANREASON;
    entWord BANMASK;
    entMult channel_storage;
    entWord channel[MAX_CHANNELS];
    entMult echannel_storage;
    entWord echannel[MAX_CHANNELS];
    entInt X, Y;
    ng_options();
    bool enabled(const char *);
};

ng_options::ng_options()
{
    registerObject(BANMASK = entWord("banmask", 1, 64, "*!%ident@%host"));
    registerObject(channel_storage = entMult("channel"));

    for(int i=0; i<MAX_CHANNELS; i++)
    {
        registerObject(channel[i] = entWord("channel", 1, CHAN_LEN));
        channel[i].setDontPrintIfDefault(true);
        channel_storage.add(&channel[i]);
    }

    registerObject(echannel_storage = entMult("echannel"));

    for(int i=0; i<MAX_CHANNELS; i++)
    {
        registerObject(echannel[i] = entWord("echannel", 1, CHAN_LEN));
        echannel[i].setDontPrintIfDefault(true);
        echannel_storage.add(&echannel[i]);
    }
}

bool ng_options::enabled(const char *name)
{
    int i;
    bool no_channels_added=true;

    // skip echannels
    for(i=0; i<MAX_CHANNELS; i++)
    {
        if(!strcasecmp(echannel[i], name))
            return false;
    }

    for(i=0; i<MAX_CHANNELS; i++)
    {
        if(!channel[i].isDefault())
        {
            no_channels_added=false;

            if(!strcasecmp(channel[i], name))
                return true;
        }
    }

    return no_channels_added;
}

class ng_options_pattern : public ng_options
{
public:
    entMult pattern_storage;
    entString pattern[NG_MAX_PATTERNS];

    ng_options_pattern()
    {
        registerObject(pattern_storage = entMult("pattern"));
        for(int i=0; i<NG_MAX_PATTERNS; ++i)
        {
            registerObject(pattern[i] = entWord("pattern", 1, 255));
            pattern[i].setDontPrintIfDefault(true);
            pattern_storage.add(&pattern[i]);
        }
    }

    bool matchPattern(const char *msg)
    {
        regex_t preg;
        regmatch_t pmatch;
        int error;
        char error_str[MAX_LEN];

        for(int i=0; i<NG_MAX_PATTERNS; i++)
        {
            if(!pattern[i].isDefault())
            {
                if((error=regcomp(&preg, pattern[i], REG_ICASE | REG_EXTENDED))!=0)
                {
                    regerror(error, &preg, error_str, MAX_LEN);
                    net.send(HAS_N, "[!] Invalid ", name.c_str(), " pattern '", (const char*)pattern[i],"' (", error_str, ")", NULL);
                    continue;
                }

                if(regexec(&preg, msg, 1, &pmatch, 0)==0)
                    return true;

                regfree(&preg);
            }
        }
        return false;
    }

};

class _color : public ng_options
{
public:
    _color()
    {
        type=0;
        name="ng:color";
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 1));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please do not use colors!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Colors are not allowed in this channel."));
        registerObject(BANREASON = entString("banreason", 1, 255, "Colors are not allowed in this channel."));
    }
} color;

class _bold : public ng_options
{
public:
    _bold()
    {
        type=1;
        name="ng:bold";
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 1));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please do not use bold characters!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Bold characters are not allowed in this channel."));
        registerObject(BANREASON = entString("banreason", 1, 255, "Bold characters are not allowed in this channel."));
    }
} bold;

class _underline : public ng_options
{
public:
    _underline()
    {
        type=2;
        name="ng:underline";
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 1));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please do not use underlined characters!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Underlined characters are not allowed in this channel."));
        registerObject(BANREASON = entString("banreason", 1, 255, "Underlined characters are not allowed in this channel."));
    }
} underline;

class _reverse : public ng_options
{
public:
    _reverse()
    {
        type=3;
        name="ng:reverse";
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 1));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please do not use reversed text!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Reversed text is not allowed in this channel."));
        registerObject(BANREASON = entString("banreason", 1, 255, "Reversed text is not allowed in this channel."));
    }
} reverse;

class _channotice : public ng_options
{
public:
    _channotice()
    {
        type=4;
        name="ng:channotice";
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 4));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please do not send notices to the whole channel!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Channel notices are not allowed in this channel."));
        registerObject(BANREASON = entString("banreason", 1, 255, "Channel notices are not allowed in this channel."));
    }
} channotice;

class _chanctcp : public ng_options
{
public:
    _chanctcp()
    {
        type=5;
        name="ng:chanctcp";
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 5));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please do not send ctcps to the whole channel!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Ctcp requests to the whole channel are prohibited!"));
        registerObject(BANREASON = entString("banreason", 1, 255, "Ctcp requests to the whole channel are prohibited!"));
    }
} chanctcp;

class _pubaway : public ng_options_pattern
{
public:
    _pubaway()
    {
        type=6;
        name="ng:pubaway";
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 1));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please do not send away notices to the whole channel!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Yes. Now you are away ;-)"));
        registerObject(BANREASON = entString("banreason", 1, 255, "Yes. Now you are away ;-)"));
    }
} pubaway;

class _caps : public ng_options
{
public:
    entPerc PERCENT;
    entInt LINE_LENGTH;

    _caps()
    {
        type=7;
        name="ng:caps";
        registerObject(PERCENT = entPerc("percent", -100, 0, -60));
        registerObject(LINE_LENGTH = entInt("line-length", 1, 512, 10));
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 4));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 15*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please turn your capslock off!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Excessive caps usage."));
        registerObject(BANREASON = entString("banreason", 1, 255, "DO NOT SHOUT!!"));
    }
} caps;

class _spam : public ng_options_pattern
{
public:
    _spam()
    {
        type=8;
        name="ng:spam";
	registerObject(X = entInt("lines", 1, 50, 2));
        registerObject(Y = entInt("seconds", 1, 120, 0));
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 5));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please stop spamming!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Don't spam!"));
        registerObject(BANREASON = entString("banreason", 1, 255, "Don't spam!"));
    }
} spam;

class _nickflood : public ng_options
{
public:
    _nickflood()
    {
        type=9;
        name="ng:nickflood";
	registerObject(X = entInt("nickchanges", 1, 50, 3));
        registerObject(Y = entInt("seconds", 1, 120, 20));
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 4));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please stop changing your nick frequently!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "nick flood (%count nicks in %seconds seconds)"));
        registerObject(BANREASON = entString("banreason", 1, 255, "nick flood (%count nicks in %seconds seconds)"));
    }
} nickflood;

class _textflood : public ng_options
{
public:
    _textflood()
    {
        type=10;
        name="ng:textflood";
	registerObject(X = entInt("lines", 1, 50, 8));
        registerObject(Y = entInt("seconds", 1, 120, 30));
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 4));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please stop flooding!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "text flood (%count lines in %seconds seconds)"));
        registerObject(BANREASON = entString("banreason", 1, 255, "text flood (%count lines in %seconds seconds)"));
    }
} textflood;

class _badnick : public ng_options_pattern
{
public:
    _badnick()
    {
        type=11;
        name="ng:badnick";
        // XXX: banmask nick!*@*
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 4));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please change your nick!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "bad nick"));
        registerObject(BANREASON = entString("banreason", 1, 255, "bad nick"));
    }
} badnick;

class _query_spam : public ng_options_pattern
{
public:
    entTime CYCLEDELAY;

    _query_spam()
    {
        type=12;
        name="ng:query-spam";
        registerObject(PUNISH_METHOD = entInt("punish-method", 1, 5, 5));
        registerObject(BANTIME = entTime("bantime", 0, 3600, 30*60));
        registerObject(WARNMSG = entString("warnmsg", 1, 255, "Please do not spam!"));
        registerObject(KICKREASON = entString("kickreason", 1, 255, "Spam on query!"));
        registerObject(BANREASON = entString("banreason", 1, 255, "Spam on query!"));
        registerObject(CYCLEDELAY = entTime("cycle-delay", 0, 3600, 0));
        // should it use the same patterns as ng:spam?
    }

    bool noPatternsAdded()
    {
        int i;

        for(i=0; i<NG_MAX_PATTERNS; i++)
        {
            if(!pattern[i].isDefault())
                    return false;
        }

        return true;
    }
} query_spam;

struct foo_func { ng_options *opt; };

// all features must be stored here
struct foo_func flist[]={
    &color,
    &bold,
    &underline,
    &reverse,
    &channotice,
    &chanctcp,
    &caps,
    &pubaway,
    &spam,
    &nickflood,
    &textflood,
    &badnick,
    &query_spam,
};

int flist_size=sizeof(flist)/sizeof(flist[0]);

class chanuserCustomData : public CustomDataObject
{
 public:
    class item
    {
     public:
        item()
        {
            count=0;
            level=0;
            timestamp=0;
        }

      	void refresh(time_t maxtimer)
      	{
                           //XXX: needed?
      	    if(maxtimer && (timestamp==0 || (NOW >= timestamp + maxtimer)))
      	    {
      	        timestamp=NOW;
      	        count=1;
      	        return;
      	    }
      	    count++;
      	}

        int count;
        int level;
        time_t timestamp;
    };

    chanuserCustomData()
    {
        info=new item[flist_size];
    }
    item *info;
};

class chanlistCustomData : public CustomDataObject
{
 public:
    class entry
    {
     public:
         char *nick;
         char *ident;
         char *host;
         void *customData;
         time_t timestamp;
         entry(char *, char *, char *, void *);
    };

    ptrlist<entry> data;
    chanlistCustomData();
    void add(char *, char *, char *, void *);
    entry *find(char *, char *, char *);
    void remove(entry *, bool);
    void delExpiredUsers();
    void flush();
};

chanlistCustomData::entry::entry(char *_nick, char *_ident, char *_host, void *_customData)
{
    nick=strdup(_nick);
    ident=strdup(_ident);
    host=strdup(_host);
    customData=_customData;
    timestamp=NOW;
}

chanlistCustomData::chanlistCustomData()
{
    data.removePtrs();
}

void chanlistCustomData::add(char *nick, char *ident, char *host, void *customData)
{
    int i;
    bool add=false;
    entry *e;

    for(i=0; i<flist_size; i++)
    {
        if(((chanuserCustomData*)customData)->info[i].level)
        {
            add=true;
            break;
        }
    }

    if(add)
    {
        e=new entry(nick, ident, host, customData);
        data.addLast(e);
    }
}

chanlistCustomData::entry *chanlistCustomData::find(char *nick, char *ident, char *host)
{
    for(ptrlist<entry>::iterator i=data.begin(); i; i++)
    {
        if(!strcmp(i->nick, nick) && !strcmp(i->ident, ident) && !strcmp(i->host, host))
            return i;
    }

    return NULL;
}

void chanlistCustomData::remove(entry *e, bool freeCustomData)
{
    free(e->nick);
    free(e->ident);
    free(e->host);

    if(freeCustomData)
        free(e->customData);

    data.remove(e);
}

void chanlistCustomData::delExpiredUsers()
{
    ptrlist<entry>::iterator i=data.begin(), j;
    int k, l;

    while(i)
    {
        j=i;
        j++;

        if(NOW>=i->timestamp+RECORD_EXPIRY)
            remove(i, true);

        i=j;
    }
}

void chanlistCustomData::flush()
{
    ptrlist<entry>::iterator i=data.begin(), j;

    while(i)
    {
        j=i;
        j++;
        remove(i, true);
        i=j;
    }
}

// customData for chan
class chanCustomData : public CustomDataObject
{
public:
    chanCustomData()
    {
        updateCycleTime();
    }

    void updateCycleTime()
    {
        if(query_spam.CYCLEDELAY>0)
            nextCycle=NOW+query_spam.CYCLEDELAY;
        else
            nextCycle=0;
    }

    time_t nextCycle;
};

class nogarbage : public Module
{
	public:
	nogarbage(void *, const char *, const char *, time_t, const char *);
	~nogarbage();
	void action(chanuser *, chan *, ng_options *);
	void ban(chan *, chanuser *, ng_options *);
	void kick(chan *, chanuser *, ng_options *);
	int calc_caps(const char *);
	void handle_query_spam(const char *, const char *);

	private:
	time_t next_save;
	int current_cycle_chan;
};

nogarbage::nogarbage(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir) :
	Module(handle, file, md5sum, loadDate, dataDir), next_save(0), current_cycle_chan(0)
{
	load_conf();

	chan *ch;
	ptrlist<chanuser>::iterator u;
	chanuser *cu;
	CHANLIST *cl;

	foreachNamedCHanlist(cl)
		onNewCHANLIST(cl);

	for (ch=ME.first, ch, ch=cg->next)
	{
		onNewChan(ch);

		for (u=ch->users.begin(); u; u++)
		{
			hook

void nogarbage::ng_load_conf()
{
    FILE *fh;
    char arg[10][MAX_LEN], buffer[MAX_LEN];
    int i, line=0;
    options::event *e;

    if(!(fh=fopen(MY_CONF, "r")))
        return;

    while(fgets(buffer, MAX_LEN, fh))
    {
        buffer[strlen(buffer)-1]='\0';
        line++;

        str2words(arg[0], buffer, 10, MAX_LEN);
        if(arg[0][0] == '#' || !*arg[0])
            continue;

        for(i=0; i<flist_size; i++)
        {
            if(flist[i].opt->name==arg[0])
            {
                e=flist[i].opt->setVariable(arg[1], rtrim(srewind(buffer, 2)));

                if(!e->ok)
                   printf("[-] %s:%d: %s\n", (const char*)MY_CONF, line, (const char *) e->reason);

                break;
            }
        }
    }

    fclose(fh);
}

void ng_save_conf()
{
    FILE *fh;
    ptrlist<ent>::iterator i;
    int j;

    if(!(fh=fopen(MY_CONF, "w")))
    {
        net.send(HAS_N, "[-] cannot open ", (const char*)MY_CONF, " for writing: ", strerror(errno), NULL);
        ng_next_save=NOW+NG_SAVE_DELAY;
        return;
    }

    for(j=0; j<flist_size; j++)
    {
        for(i=flist[j].opt->list.begin(); i; i++)
        {
            if(!i->isDefault() && i->isPrintable())
                fprintf(fh, "%s %s\n", flist[j].opt->name.c_str(), i->print());
        }
   }

   fclose(fh);
   ng_next_save=0;
   net.send(HAS_N, "[*] Autosaving nogarbabe config", NULL); // m->name
}

void hook_botnetcmd(const char *from, const char *cmd)
{
    char arg[10][MAX_LEN];
    int i;

    str2words(arg[0], cmd, 10, MAX_LEN, 0);

    if(!strcasecmp(arg[1], "ng:*"))
    {
        for(i=0; i<flist_size; i++)
            if(flist[i].opt->parseUser(arg[0], arg[2], srewind(cmd, 3), flist[i].opt->name.c_str()))
                ng_next_save=NOW+NG_SAVE_DELAY;
        return;
    }

    for(i=0; i<flist_size; i++)
    {
        if(flist[i].opt->name==arg[1])
        {
            if(flist[i].opt->parseUser(arg[0], arg[2], srewind(cmd, 3), flist[i].opt->name.c_str()))
                ng_next_save=NOW+NG_SAVE_DELAY;
        }
    }
}

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    chan *ch;
    chanuser *cu;

    if(!(ch=ME.findChannel(to)))
    {
        if(!strcasecmp(ME.nick, to))
            ng_handle_query_spam(from, msg);
        return;
    }

    if(!(ch->me->flags&IS_OP))
        return;

    if(!(cu=ch->getUser(from)))
        return;

    if(cu->flags & (HAS_V | HAS_O | IS_OP))
        return;

    if(color.enabled(ch->name) && match("*\003*", msg))
        ng_action(cu, ch, &color);
    // XXX: why "else"?
    else if(bold.enabled(ch->name) && match("*\002*", msg))
        ng_action(cu, ch, &bold);

    else if(underline.enabled(ch->name) && match("*\037*", msg))
        ng_action(cu, ch, &underline);

    else if(reverse.enabled(ch->name) && match("*\x16*", msg))
        ng_action(cu, ch, &reverse);

    else if(caps.enabled(ch->name) && ng_calc_caps(msg)>=(-1)*caps.PERCENT)
        ng_action(cu, ch, &caps);

    else if(spam.enabled(ch->name) && spam.matchPattern(msg))
        ng_action(cu, ch, &spam);

    else if(textflood.enabled(ch->name))
        ng_action(cu, ch, &textflood);
}

void hook_notice(const char *from, const char *to, const char *msg)
{
    chan *ch;
    chanuser *cu;

    if(!(ch=ME.findChannel(to)))
    {
        if(!strcasecmp(ME.nick, to))
            ng_handle_query_spam(from, msg);
        return;
    }

    if(!(ch->me->flags&IS_OP))
        return;

    if(!(cu=ch->getUser(from)))
        return;

    if(cu->flags & (HAS_V | HAS_O | IS_OP))
        return;

    if(channotice.enabled(ch->name))
        ng_action(cu, ch, &channotice);

    else
    {
        if(color.enabled(ch->name) && match("*\003*", msg))
            ng_action(cu, ch, &color);

        else if(bold.enabled(ch->name) && match("*\002*", msg))
            ng_action(cu, ch, &bold);

        else if(underline.enabled(ch->name) && match("*\037*", msg))
            ng_action(cu, ch, &underline);

        else if(reverse.enabled(ch->name) && match("*\x16*", msg))
            ng_action(cu, ch, &reverse);

        else if(caps.enabled(ch->name) && ng_calc_caps(msg)>=(-1)*caps.PERCENT)
            ng_action(cu, ch, &caps);

        else if(spam.enabled(ch->name) && spam.matchPattern(msg))
            ng_action(cu, ch, &spam);

        else if(textflood.enabled(ch->name))
            ng_action(cu, ch, &textflood);
    }
}

void hook_ctcp(const char *from, const char *to, const char *msg)
{
    char buffer[MAX_LEN];
    chan *ch;
    chanuser *cu;

    if(!(ch=ME.findChannel(to)))
    {
        if(!strcasecmp(ME.nick, to))
        {
            if(match("ACTION *", msg))
                ng_handle_query_spam(from, msg+7);
        }

        return;
    }

    if(!(ch->me->flags&IS_OP))
        return;

    if(!(cu=ch->getUser(from)))
        return;

    if(cu->flags & (HAS_V | HAS_O | IS_OP))
        return;

    if(match("ACTION *", msg))
    {
        strncpy(buffer, msg+7, MAX_LEN-1);
	buffer[MAX_LEN-1]='\0';

        if(pubaway.enabled(ch->name) && pubaway.matchPattern(buffer))
            ng_action(cu, ch, &pubaway);

        else if(color.enabled(ch->name) && match("*\003*", buffer))
            ng_action(cu, ch, &color);

        else if(bold.enabled(ch->name) && match("*\002*", buffer))
            ng_action(cu, ch, &bold);

        else if(underline.enabled(ch->name) && match("*\037*", buffer))
            ng_action(cu, ch, &underline);

        else if(reverse.enabled(ch->name) && match("*\x16*", buffer))
            ng_action(cu, ch, &reverse);

        else if(caps.enabled(ch->name) && ng_calc_caps(buffer)>=(-1)*caps.PERCENT)
            ng_action(cu, ch, &caps);

        else if(spam.enabled(ch->name) && spam.matchPattern(buffer))
            ng_action(cu, ch, &spam);

        else if(textflood.enabled(ch->name))
            ng_action(cu, ch, &textflood);
    }

    else if(chanctcp.enabled(ch->name))
        ng_action(cu, ch, &chanctcp);
}

void ng_action(chanuser *cu, chan *ch, ng_options *opt)
{
    if(opt->X!=0) // lines per second
    {
        ((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].refresh(opt->Y);
        if(((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].count<opt->X)
            return;
    }

    switch(opt->PUNISH_METHOD)
    {
        case 1 : if(!(((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level&NG_WARNED))
                 {
                     ((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level|=NG_WARNED;
                     ME.notice(cu->nick, opt->WARNMSG, NULL);
                 }

                 else if(!(((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level&NG_KICKED))
                 {
                     ((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level|=NG_KICKED;
                     ng_kick(ch, cu, opt); 
                 }
                 else// if(!(i->level&NG_BANNED))
                     ng_ban(ch, cu, opt);
                 break;

        case 2 : if(!(((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level&NG_WARNED))
                 {
                      ((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level|=NG_WARNED;
                      ME.notice(cu->nick, opt->WARNMSG, NULL);
                 }
                 else
                 {
                     //i->level|=NG_KICKED;
                     ng_kick(ch, cu, opt);
                 }
                 break;

        case 3 : if(!(((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level&NG_WARNED))
                 {
                     ((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level|=NG_WARNED;
                     ME.notice(cu->nick, opt->WARNMSG, NULL);
                 }
                 else
                     ng_ban(ch, cu, opt);
                 break;

        case 4 : if(!(((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level&NG_KICKED))
                 {
                     ((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].level|=NG_KICKED;
                     ng_kick(ch, cu, opt);
                 }
                 else
                     ng_ban(ch, cu, opt);
                 break;

        case 5 : ng_ban(ch, cu, opt);
                 break;
    }

    ((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].count=0;
}

void ng_ban(chan *ch, chanuser *cu, ng_options *opt)
{
    std::string str1, str2;
    std::string::size_type pos1, pos2;

    str1=(const char*)opt->BANMASK;
    if((pos1=str1.find("%nick"))!=std::string::npos)
        str1.replace(pos1, strlen("%nick"), cu->nick);
    if((pos1=str1.find("%ident"))!=std::string::npos)
        str1.replace(pos1, strlen("%ident"), cu->ident);
    if((pos1=str1.find("%host"))!=std::string::npos)
        str1.replace(pos1, strlen("%host"), cu->host);

    str2=(const char*)opt->BANREASON;
    if((pos2=str2.find("%count"))!=std::string::npos)
        str2.replace(pos2, strlen("%count"), itoa(((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].count));
    if((pos2=str2.find("%seconds"))!=std::string::npos)
        str2.replace(pos2, strlen("%seconds"), itoa(NOW-((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].timestamp));

    if(!set.BOTS_CAN_ADD_SHIT
       || !protmodelist::addShit(ch->name, str1.c_str(), "nogarbage", opt->BANTIME, str2.c_str()))
    {
        ch->modeQ[PRIO_HIGH].add(NOW, "+b", str1.c_str());
        ch->modeQ[PRIO_LOW].add(NOW+opt->BANTIME, "-b", str1.c_str())->backupmode=true;
        ch->modeQ[PRIO_HIGH].flush(PRIO_HIGH);

        ch->kick(cu, str2.c_str());
        cu->setReason(str2.c_str());
    }
}

void ng_kick(chan *ch, chanuser *cu, ng_options *opt)
{
    std::string str=(const char*)opt->KICKREASON;
    std::string::size_type pos;

    if((pos=str.find("%count"))!=std::string::npos)
        str.replace( pos, strlen("%count"), itoa(((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].count));
    if((pos=str.find("%seconds"))!=std::string::npos)
        str.replace( pos, strlen("%seconds"), itoa(NOW-((chanuserCustomData*)cu->customData(m->desc))->info[opt->type].timestamp));

    ch->kick(cu, str.c_str());
    cu->setReason(str.c_str());
}

int ng_calc_caps(const char *msg)
{
    int msglen=strlen(msg), capscnt=0, i, percent;

    if(msglen<caps.LINE_LENGTH)
        return -1;

    for(i=0; i<msglen; i++)
        if(isupper(msg[i]))
            capscnt++;

    percent=100*capscnt/msglen;

    return percent;
}

void ng_handle_query_spam(const char *from, const char *msg)
{
    chan *ch;
    chanuser *cu;

    if(query_spam.noPatternsAdded() || query_spam.matchPattern(msg))
    {   
        for(ch=ME.first; ch; ch=ch->next)
        {
            if(query_spam.enabled(ch->name) && (cu=ch->getUser(from)))
            {
                if(cu->flags & (HAS_V | HAS_O | IS_OP))
                    continue;

                ng_action(cu, ch, &query_spam);
            }
        }
        
    }
}

void hook_nick(const char *from, const char *to)
{
    chan *ch;
    chanuser *u;

    for(ch=ME.first; ch; ch=ch->next)
    {
        if((u=ch->getUser(to)))
        {
            if(badnick.enabled(ch->name) && badnick.matchPattern(to))
                ng_action(u, ch, &badnick);

            if(nickflood.enabled(ch->name))
                ng_action(u, ch, &nickflood);
        }
    }
}

void hook_join(chanuser *u, chan *ch, const char *mask, int netjoin)
{
    if(badnick.enabled(ch->name) && badnick.matchPattern(u->nick))
        ng_action(u, ch, &badnick);
}

void hook_timer()
{
    CHANLIST *cl;
    chan *ch;

    foreachNamedChanlist(cl)
        ((chanlistCustomData*)cl->customData(m->desc))->delExpiredUsers();

    if(ng_next_save!=0 && NOW>=ng_next_save)
        ng_save_conf();

    for(; ng_current_cycle_chan<MAX_CHANNELS && penalty<6; ng_current_cycle_chan++)
    {
        if(!userlist.chanlist[ng_current_cycle_chan].name || !query_spam.enabled(userlist.chanlist[ng_current_cycle_chan].name) || !(ch=ME.findChannel(userlist.chanlist[ng_current_cycle_chan].name)))
            continue;

        if(ch->opedBots.entries()>1 && NOW>=((chanCustomData*)ch->customData(m->desc))->nextCycle)
        {
           if(((chanCustomData*)ch->customData(m->desc))->nextCycle!=0)
           {
               net.irc.send("PART ", (const char *) ch->name, " :spam check", NULL);
               penalty+=4;
               ME.rejoin(ch->name, set.CYCLE_DELAY);
           }
           ((chanCustomData*)ch->customData(m->desc))->updateCycleTime();
        }
    }

    if(ng_current_cycle_chan>=MAX_CHANNELS)
        ng_current_cycle_chan=0;
}

void hook_kick(chan *ch, chanuser *kicked, chanuser *kicker, const char *reason)
{
    CHANLIST *cl=userlist.findChanlist(ch->name);

    if(cl)
        ((chanlistCustomData*)cl->customData(m->desc))->add(kicked->nick, kicked->ident, kicked->host, ((chanuserCustomData*)kicked->customData(m->desc)));
}

void hook_pre_part(const char *mask, const char *channel, const char *msg, bool quit)
{
    CHANLIST *cl=userlist.findChanlist(channel);
    chan *ch=ME.findChannel(channel);
    chanuser *u;

    if(cl && ch && (u=ch->getUser(mask)))
        ((chanlistCustomData*)cl->customData(m->desc))->add(u->nick, u->ident, u->host, ((chanuserCustomData*)u->customData(m->desc)));
}

void hook_chanuserConstructor(const chan *ch, chanuser *u)
{
    chanlistCustomData::entry *i;
    CHANLIST *cl=userlist.findChanlist(ch->name);

    if(cl && (i=((chanlistCustomData *)cl->customData(m->desc))->find(u->nick, u->ident, u->host)))
    {
        u->setCustomData(m->desc, (chanlistCustomData *) i->customData);
        //u->customData=i->customData; TODO: rm
        ((chanlistCustomData*)cl->customData(m->desc))->remove(i, false);
    }
    else
        u->setCustomData(m->desc, new chanuserCustomData);
}

void hook_new_CHANLIST(CHANLIST *me)
{
    me->setCustomData(m->desc, new chanlistCustomData);
}

void hook_del_CHANLIST(CHANLIST *me)
{
    if(me->customData(m->desc))
    {
        ((chanlistCustomData*)me->customData(m->desc))->flush();
        delete (chanlistCustomData*) me->customData(m->desc);
        me->delCustomData(m->desc);
    }

    ng_current_cycle_chan=0;
}

void hook_new_chan(chan *me)
{
    me->setCustomData(m->desc, new chanCustomData);
}

void hook_del_chan(chan *me)
{
    if(me->customData(m->desc))
    {
        delete (chanCustomData *)me->customData(m->desc);
        me->delCustomData(m->desc);
    }
}

void prepareCustomData()
{
    chan *ch;
    ptrlist<chanuser>::iterator u;
    chanuser *cu;
    CHANLIST *cl;

   foreachNamedChanlist(cl)
       hook_new_CHANLIST(cl);

   for(ch=ME.first; ch; ch=ch->next)
   {
       hook_new_chan(ch);

       for(u=ch->users.begin(); u; u++)
           hook_chanuserConstructor(ch, u);
    }
}

extern "C" module *init()
{
    m=new module("nogarbage", "patrick <patrick@psotnic.com>", "0.1-pre3");

    ng_next_save=0;
    ng_current_cycle_chan=0;
    ng_load_conf();
    prepareCustomData();

    m->hooks->chanuserConstructor=hook_chanuserConstructor;
    m->hooks->new_chan=hook_new_chan;
    m->hooks->del_chan=hook_del_chan;
    m->hooks->new_CHANLIST=hook_new_CHANLIST;
    m->hooks->del_CHANLIST=hook_del_CHANLIST;

    m->hooks->botnetcmd=hook_botnetcmd;
    m->hooks->privmsg=hook_privmsg;
    m->hooks->notice=hook_notice;
    m->hooks->ctcp=hook_ctcp;
    m->hooks->nick=hook_nick;
    m->hooks->join=hook_join;
    m->hooks->timer=hook_timer;
    m->hooks->kick=hook_kick;
    m->hooks->pre_part=hook_pre_part;
    m->hooks->chanuserConstructor=hook_chanuserConstructor;
    return m;
}

extern "C" void destroy()
{
    CHANLIST *cl;

    foreachNamedChanlist(cl)
        ((chanlistCustomData*)cl->customData(m->desc))->flush();
}
