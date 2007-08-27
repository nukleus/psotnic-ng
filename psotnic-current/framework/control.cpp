#include "includes/psotnic.h"

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    char arg[10][MAX_LEN];
    char buf[MAX_LEN];
    str2words(arg[0], msg, 10, MAX_LEN, 0);
    
    if(!strcmp("!topic", msg))
    {
	chan *ch = findChannel(to);
	if(ch)
	{
	    chanuser *u = findUser(from, ch);
    	    if(u && u->flags & HAS_N)
	    {
		snprintf(buf, MAX_LEN, "%s: topic: %s\n", u->nick, (const char *) ch->topic);
		privmsg(ch->name, buf);
	    }
	}
    }
    else if(!strcmp("!retopic", msg))
    {
	chan *ch = findChannel(to);
        if(ch)
        {
	    chanuser *u = findUser(from, ch);
	    if(u && (u->flags & HAS_N) && (ch->me->flags & IS_OP))
	    {
		if(*ch->topic)
		    setTopic(ch, ch->topic);
		else
		{
		    snprintf(buf, MAX_LEN, "%s: topic is not set", u->nick);
	    	    privmsg(ch->name, buf);
		}
	    }
	}
    }
    else if(!strcmp("!ub", msg))
    {
	chan *ch = findChannel(to);
        if(ch)
        {
	    chanuser *u = findUser(from, ch);
	    if(u && (u->flags & HAS_N) && (ch->me->flags & IS_OP))
	    {
		int i;
		ptrlist<masklist_ent>::iterator m = ch->list[BAN].masks.begin();
		
		while(m)
		{
		    if(!isSticky(m->mask, ch))
			addMode(ch, "-b", m->mask, PRIO_LOW, 0);
		    m++;
		}
	    }
	}
    }
    else if(!strcmp("!massvoice", arg[0]) || !strcmp("!mv", arg[0]))
    {
        chan *ch = findChannel(to);

	if(ch && ch->me->flags & IS_OP)
	{
	    chanuser *f = findUser(from, ch);
	    if(f && f->flags & HAS_N)
	    {
		ptrlist<chanuser>::iterator i = ch->users.begin();
											    
	        while(i)
	        {
	            if(&i && !(i->flags & (IS_VOICE | VOICE_SENT | IS_OP | OP_SENT)) && !(i->flags & HAS_Q))
	        	addMode(ch, "+v", i->nick, PRIO_LOW, 0);
		    
		    i++;
		}
	    }
	}
    }
    else if(!strcmp("!masskickallnonvoicedusersonthischannel", arg[0]))
    {
        chan *ch = findChannel(to);

	if(ch && ch->me->flags & IS_OP)
	{
	    chanuser *f = findUser(from, ch);
	    if(f && f->flags & HAS_N)
	    {
		ptrlist<chanuser>::iterator i = ch->users.begin();
											    
	        while(i)
	        {
	            if(&i && !(i->flags & (IS_VOICE | VOICE_SENT | IS_OP | OP_SENT | HAS_V)))
			addKick(ch, i, "cleaning up");
		    i++;
		}
	    }
	}
    }
}

void hook_botnetcmd(const char *from, const char *cmd)
{
    char arg[10][MAX_LEN];
    char buf[MAX_LEN];
	
    str2words(arg[0], cmd, 10, MAX_LEN, 0);
    
    //from rkick #chan user reason
    if(!strcmp(arg[1], "rkick"))
    {
        if(!strlen(arg[3]))
	    sendToOwner(arg[0], "Syntax: rkick <chan> <nick> [reason]");
	else
	{
	    chan *ch = findChannel(arg[2]);
	    if(ch)
	    {
	        chanuser *u = findUser(arg[3], ch);
	        if(u)
	        {
			addKick(ch, u, srewind(cmd, 4));
		}
		else
		    sendToOwner(arg[0], "Invalid nick");
	    }
	    else
	        sendToOwner(arg[0], "Invalid channel");
	}
    }
}
	

extern "C" module *init()
{
    module *m = new module("example #3: control", "Grzegorz Rusin <pks@irc.pl, gg:0x17f1ceh>", "0.1.0");
    
    m->hooks->privmsg = hook_privmsg;
    m->hooks->botnetcmd = hook_botnetcmd;
    
    return m;
}
