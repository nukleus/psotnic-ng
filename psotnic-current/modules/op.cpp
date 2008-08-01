#include "../prots.h"
#include "../global-var.h"
#include "module.h"

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    if(match("!op*", msg))
    {
	chan *ch = ME.findChannel(to);
	if(ch)
	{
	    printf("found channel: %s\n", to);
	    //do i have op ?
	    if(ch->me->flags & IS_OP)
	    {
		printf("i have op\n");
		chanuser *u = ch->getUser(from);
    		if(u)
		{
		    printf("got user %s\n", u->nick);
		    //does user have +o and is not oped?
		    if(u->flags & HAS_O && !(u->flags & IS_OP))
		    {
			printf("sending +o\n");
			//if so, add +o to mode queue
			ch->modeQ[PRIO_LOW].add(NOW, "+o", u->nick);
		    }
		}
	    }
	    else
		ME.notice(from, "Sorry, but I am not oped", NULL);
	}
	else ME.notice(from, "I am not on that channel ;/", NULL);
    }
}

extern "C" module *init()
{
    module *m = new module("example #1: !op public command", "Grzegorz Rusin <pks@irc.pl, gg:0x17f1ceh>", "0.1.0");
    m->hooks->privmsg = hook_privmsg;
    return m;
}

extern "C" void destroy()
{
}

