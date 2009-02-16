
#include "../prots.h"
#include "../global-var.h"
#include "../module.h"

class Control : public Module
{
    public:
    Control(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);
    virtual bool onLoad(string &msg);
    virtual void onPrivmsg(const char *from, const char *to, const char *msg);
    virtual void onBotnetcmd(const char *from, const char *cmd);
};

Control::Control(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir) : Module(handle, file, md5sum, loadDate, dataDir)
{
}

bool Control::onLoad(string &msg)
{
    return true;
}

void Control::onPrivmsg(const char *from, const char *to, const char *msg)
{
    char arg[10][MAX_LEN];
    char buf[MAX_LEN];
    str2words(arg[0], msg, 10, MAX_LEN, 0);
    
    if(!strcmp("!topic", msg))
    {
	chan *ch = ME.findChannel(to);
	if(ch)
	{
	    chanuser *u = ch->getUser(from);
    	    if(u && u->flags & HAS_N)
	    {
		snprintf(buf, MAX_LEN, "%s: topic: %s\n", u->nick, (const char *) ch->topic);
		ME.privmsg(ch->name, buf, NULL);
	    }
	}
    }
    else if(!strcmp("!retopic", msg))
    {
	chan *ch = ME.findChannel(to);
        if(ch)
        {
	    chanuser *u = ch->getUser(from);
	    if(u && (u->flags & HAS_N) && (ch->me->flags & IS_OP))
	    {
		if(*ch->topic)
		    net.irc.send("TOPIC ", (const char *) ch->name, " :", (const char*) ch->topic, NULL);
		else
		{
		    snprintf(buf, MAX_LEN, "%s: topic is not set", u->nick);
	    	    ME.privmsg(ch->name, buf, NULL);
		}
	    }
	}
    }
    else if(!strcmp("!ub", msg))
    {
	chan *ch = ME.findChannel(to);
        if(ch)
        {
	    chanuser *u = ch->getUser(from);
	    if(u && (u->flags & HAS_N) && (ch->me->flags & IS_OP))
	    {
		ptrlist<masklist_ent>::iterator m = ch->list[BAN].masks.begin();
		
		while(m)
		{
		    if(!protmodelist::isSticky(m->mask, BAN, ch))
			ch->modeQ[PRIO_LOW].add(NOW, "-b", m->mask);
		    m++;
		}
	    }
	}
    }
    else if(!strcmp("!massvoice", arg[0]) || !strcmp("!mv", arg[0]))
    {
        chan *ch = ME.findChannel(to);

	if(ch && ch->me->flags & IS_OP)
	{
	    chanuser *f = ch->getUser(from);
	    if(f && f->flags & HAS_N)
	    {
		ptrlist<chanuser>::iterator i = ch->users.begin();
											    
	        while(i)
	        {
	            if(&i && !(i->flags & (IS_VOICE | VOICE_SENT | IS_OP | OP_SENT)) && !(i->flags & HAS_Q))
			ch->modeQ[PRIO_LOW].add(NOW, "+v", i->nick);
		    
		    i++;
		}
	    }
	}
    }
    else if(!strcmp("!masskickallnonvoicedusersonthischannel", arg[0]))
    {
        chan *ch = ME.findChannel(to);

	if(ch && ch->me->flags & IS_OP)
	{
	    chanuser *f = ch->getUser(from);
	    if(f && f->flags & HAS_N)
	    {
		ptrlist<chanuser>::iterator i = ch->users.begin();
											    
	        while(i)
	        {
	            if(&i && !(i->flags & (IS_VOICE | VOICE_SENT | IS_OP | OP_SENT | HAS_V)))
	            {
			i->setReason("cleaning up");
			ch->toKick.sortAdd(i);
	            }
		    i++;
		}
	    }
	}
    }
}

void Control::onBotnetcmd(const char *from, const char *cmd)
{
    char arg[10][MAX_LEN];
	
    str2words(arg[0], cmd, 10, MAX_LEN, 0);
    
    //from rkick #chan user reason
    if(!strcmp(arg[1], "rkick"))
    {
        if(!strlen(arg[3]))
	    net.sendOwner(arg[0], "Syntax: rkick <chan> <nick> [reason]", NULL);
	else
	{
	    chan *ch = ME.findChannel(arg[2]);
	    if(ch)
	    {
	        chanuser *u = ch->getUser(arg[3]);
	        if(u)
	        {
			u->setReason(srewind(cmd, 4));
			ch->toKick.sortAdd(u);
		}
		else
		    net.sendOwner(arg[0], "Invalid nick", NULL);
	    }
	    else
	        net.sendOwner(arg[0], "Invalid channel", NULL);
	}
    }
}

MOD_LOAD( Control );
MOD_DESC( "control", "example #3" );
MOD_AUTHOR( "Grzegorz Rusin", "pks@irc.pl, gg:0x17f1ceh" );
MOD_VERSION( "0.1.0" );

