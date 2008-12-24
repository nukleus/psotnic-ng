/* plog - a logging module for psotnic
 *
 * TODO: - ME.removeChannel() can be executed before the bot has left the channel,
 *         if it retrieves traffic of this channel, the logfile will be opened again
 *         never closed (just an assumption)
 *       - variable for umask for log files and directories
 */

#include "plog.h"

Plog *plog;

void hook_connected()
{
    if(plog->client->set->LOG && plog->client->set->LOG_CONNECTED)
        plog->client->log("connected to %s as %s", net.irc.name, (*ME.mask)?get_hostmask(ME.mask):(const char*)ME.nick);
}

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    Plog::chanRecord *logrec;
    char *stripped_msg, to_str[MAX_LEN];

    stripped_msg=strip_color(msg, strlen(msg), false);

    if(chan::isChannel(to))
    {
         if((logrec=plog->findLoggingChannel(to)))
             logrec->log("%s: %s", get_hostmask(from), stripped_msg);
    }

    else if(plog->client->set->LOG && plog->client->set->LOG_GOT_MSG)
    {
        to_str[0]='\0';

        if(strcasecmp(to, ME.nick))
            snprintf(to_str, MAX_LEN, " to %s", to);

        if(match("*!*@*", from))
        {
            chanuser u(from, NULL, 0, false);

            // ignore anti-idle
            if(!strcasecmp(u.nick, ME.nick))
                return;

            if(!(*to_str) && (match("op *", msg) || match("?op *", msg)
                || match("voice *", msg) || match("?voice *", msg)
                || match("invite *", msg) || match("?invite *", msg)
                || match("key *", msg) || match("?key *", msg)
                || match("pass *", msg) || match("?pass *", msg)
                || match("mainowner", msg)))
                 return;
        }

        plog->client->log("message from %s%s: %s", get_hostmask(from), *to_str?to_str:"", stripped_msg);
    }

    free(stripped_msg);
}

void hook_notice(const char *from, const char *to, const char *msg)
{
    Plog::chanRecord *logrec;
    char *stripped_msg, to_str[MAX_LEN];

    stripped_msg=strip_color(msg, strlen(msg), false);

    if(chan::isChannel(to))
    {
        if((logrec=plog->findLoggingChannel(to)) && logrec->set->LOG_NOTICE)
            logrec->log("-%s- %s", get_hostmask(from), stripped_msg);
    }

    else if(plog->client->set->LOG && plog->client->set->LOG_GOT_NOTICE)
    {
        to_str[0]='\0';

        if(strcasecmp(to, ME.nick))
            snprintf(to_str, MAX_LEN, " to %s", to);

        plog->client->log("notice from %s%s: %s", get_hostmask(from), *to_str?to_str:"", stripped_msg);
    }

    free(stripped_msg);
}

void hook_ctcp(const char *from, const char *to, const char *msg)
{
    Plog::chanRecord *logrec;
    char *stripped_msg, to_str[MAX_LEN];

    if(match("ACTION *", msg) && match("*!*@*", from)) // /me
    {
        stripped_msg=strip_color(msg, strlen(msg), false);

        if(chan::isChannel(to))
        {
            if((logrec=plog->findLoggingChannel(to)) && logrec->set->LOG_MSG)
                logrec->log("* %s %s", get_hostmask(from), stripped_msg+7);
        }

        else if(plog->client->set->LOG && plog->client->set->LOG_GOT_MSG)
        {
            to_str[0]='\0';

            if(strcasecmp(to, ME.nick))
                snprintf(to_str, MAX_LEN, " to %s", to);

            plog->client->log("msg from %s: * %s %s", get_hostmask(from), stripped_msg+7);
        }

        free(stripped_msg);
    }
    else // CTCP
    {
        char *tmp, *ptr, *rest;

        // split up first word of CTCP line (e.g. "PING")
        tmp=strdup(msg);
        ptr=strchr(tmp, ' ');

        if(ptr)
            *ptr='\0';

        rest=srewind(msg, 1);

        if(chan::isChannel(to))
        {
            if((logrec=plog->findLoggingChannel(to)) && logrec->set->LOG_CTCP)
                logrec->log("%s requested CTCP %s from %s: %s", get_hostmask(from), tmp, to, rest?rest:"");
        }

        else if(plog->client->set->LOG && plog->client->set->LOG_GOT_CTCP)
            plog->client->log("%s requested CTCP %s from %s: %s", get_hostmask(from), tmp, to, rest?rest:"");

        free(tmp);
    }
}

void hook_invite(const char *from, const char *to, chan *ch, CHANLIST *cl)
{
    if(!cl && plog->client->set->LOG && plog->client->set->LOG_GOT_INVITED) // unknown channel
        plog->client->log("%s invited me to join %s", get_hostmask(from), to);
}

/** notifies when the bot has disconnected.
 * This function will only be used here if the ircd did not send ERROR.
 */

void hook_disconnect(const char *reason)
{
    char *stripped_reason=NULL;

    if(plog->client->set->LOG && plog->client->set->LOG_DISCONNECTED)
    {
        if(!plog->client->parsedQuit)
        {
            if(reason)
                stripped_reason=strip_color(reason, strlen(reason), false);

            plog->client->log("Disconnected from server %s (%s)", net.irc.name, stripped_reason?stripped_reason:"");

            if(stripped_reason)
                free(stripped_reason);
        }

        else
            plog->client->parsedQuit=false;
    }
}

/** parses raw irc traffic.
 * Used because the information of psotnic's hooks are too limited.
 */

void hook_raw(const char *data)
{
    char arg[11][MAX_LEN];
    Plog::chanRecord *logrec=NULL;
    chan *ch=NULL;
    str2words(arg[0], data, 11, MAX_LEN, 1);

    if(!strcmp(arg[1], "NICK"))
    {
        chanuser *u=new chanuser(arg[0], NULL, 0, false);

        if(plog->client->set->LOG && plog->client->set->LOG_NICK_CHANGED && !strcmp(ME.nick, u->nick))
            plog->client->log("I am now known as %s", arg[2]);

        free(u);

        for(ch=ME.first; ch; ch=ch->next)
        {
            if((u=ch->getUser(arg[0])))
            {
                logrec=plog->findLoggingChannel(ch->name);
                if(logrec && logrec->set->LOG_NICK_CHANGE)
                    logrec->log("-!- %s is now known as %s", get_hostmask(arg[0]), arg[2]);
            }
        }
    }

    else if(!strcmp(arg[1], "JOIN"))
    {
        chanuser u(arg[0], NULL, 0, false);
        char *a;
        int netjoin=arg[2][0]!=':';

        if(netjoin)
            a=arg[2];
        else
            a=arg[2]+1;

        if((logrec=plog->findLoggingChannel(a)) && logrec->set->LOG_JOIN)
            logrec->log("-!- %s has %sjoined %s", get_hostmask(arg[0]), netjoin?"net":"", a);
    }

    else if(!strcmp(arg[1], "MODE"))
    {
        if(chan::isChannel(arg[2]))
        {
            logrec=plog->findLoggingChannel(arg[2]);

            if(logrec && logrec->set->LOG_MODE)
            {
                char *tmp=strdup(srewind(data, 3));
                int len=strlen(tmp);

                if(tmp[len-1]==' ')
                    tmp[len-1]='\0';

                 logrec->log("-!- mode/%s [%s] by %s", arg[2], tmp, get_hostmask(arg[0]));
                 free(tmp);
            }
        }

        else if(plog->client->set->LOG && plog->client->set->LOG_USER_MODE) // usermode
        {
            char by_str[MAX_LEN];
            by_str[0]='\0';

            if(strcasecmp(arg[0], ME.nick))
                snprintf(by_str, MAX_LEN, " by %s", arg[0]);

            plog->client->log("Mode change [%s] for user %s%s", srewind(data, 3)+1, arg[2], *by_str?by_str:"");
        }
    }

    else if(!strcmp(arg[1], "KICK"))
    {
        char *comment, *stripped_comment=NULL;
        logrec=plog->findLoggingChannel(arg[2]);

        if(logrec && logrec->set->LOG_KICK)
        {
            comment=srewind(data, 4);

            if(comment)
            {
                *comment++;
                stripped_comment=strip_color(comment, strlen(comment), false);
            }

            logrec->log("-!- %s was kicked from %s by %s [%s]", arg[3], arg[2], get_hostmask(arg[0]), stripped_comment?stripped_comment:"");

            if(stripped_comment)
                free(stripped_comment);
        }
    }

    if(!strcmp(arg[1], "PART"))
    {
        char *partmsg, *stripped_partmsg=NULL;

        logrec=plog->findLoggingChannel(arg[2]);

        if(logrec && logrec->set->LOG_PART)
        {
            partmsg=srewind(data, 3);

            if(partmsg)
            {
                *partmsg++;
                stripped_partmsg=strip_color(partmsg, strlen(partmsg), false);
            }

            logrec->log("-!- %s has left %s [%s]", get_hostmask(arg[0]), arg[2], stripped_partmsg?stripped_partmsg:"");

            if(stripped_partmsg)
                free(stripped_partmsg);
        }
    }

    else if(!strcmp(arg[1], "QUIT"))
    {
        chanuser *u;
        char *quitmsg, *stripped_quitmsg=NULL;

        quitmsg=srewind(data, 2);

        if(quitmsg)
        {
            *quitmsg++;
            stripped_quitmsg=strip_color(quitmsg, strlen(quitmsg), false);
        }

        for(ch=ME.first; ch; ch=ch->next)
        {
            if((u=ch->getUser(arg[0])))
            {
                logrec=plog->findLoggingChannel(ch->name);

                if(logrec && logrec->set->LOG_QUIT)
                    logrec->log("-!- %s has quit [%s]", get_hostmask(arg[0]), stripped_quitmsg?stripped_quitmsg:"");
            }
        }

        if(stripped_quitmsg)
            free(stripped_quitmsg);
    }

    else if(!strcmp(arg[0], "ERROR"))
    {
        char *errormsg, *stripped_errormsg=NULL;

        if(plog->client->set->LOG && plog->client->set->LOG_DISCONNECTED)
        {
            errormsg=srewind(data, 1);

            if(errormsg)
            {
                *errormsg++;
                stripped_errormsg=strip_color(errormsg, strlen(errormsg), false);
            }

            plog->client->log("Disconnected from server %s (%s)", net.irc.name, stripped_errormsg?stripped_errormsg:"");

            if(stripped_errormsg)
                free(stripped_errormsg);

            // do not log the disconnect in hook_disconnect() again.
            plog->client->parsedQuit=true;
        }
    }

    else if(!strcmp(arg[1], "KILL"))
    {
        char reason[MAX_LEN], *stripped_reason;

        if(plog->client->set->LOG && plog->client->set->LOG_GOT_KILLED && !strcasecmp(ME.nick, arg[2]))
        {
            strncpy(reason, srewind(data, 4)+1, MAX_LEN-1);
            reason[strlen(reason)-1]='\0'; // remove ')'
            stripped_reason=strip_color(reason, strlen(reason), false);
            plog->client->log("I have been killed by %s (%s)", arg[0], stripped_reason);
            free(stripped_reason);
        }
    }

    else if(!strcmp(arg[1], "TOPIC"))
    {
        char *topic, *stripped_topic=NULL;

        logrec=plog->findLoggingChannel(arg[2]);

        if(logrec && logrec->set->LOG_TOPIC)
        {
            topic=srewind(data, 3);

            if(topic)
            {
                *topic++;
                stripped_topic=strip_color(topic, strlen(topic), false);
            }

            logrec->log("-!- %s changed the topic of %s to: %s", get_hostmask(arg[0]), arg[2], stripped_topic?stripped_topic:"");

            if(stripped_topic)
                free(stripped_topic);
        }
    }

    else if(!strcmp(arg[1], "332")) // RPL_TOPIC
    {
        char *topic, *stripped_topic=NULL;

        logrec=plog->findLoggingChannel(arg[3]);

        if(logrec && logrec->set->LOG_TOPIC)
        {
            topic=srewind(data, 4);

            if(topic)
            {
                *topic++;
                stripped_topic=strip_color(topic, strlen(topic), false);
            }

            logrec->log("-!- Topic for %s: %s", arg[3], stripped_topic?stripped_topic:"");

            if(stripped_topic)
                free(stripped_topic);
        }
    }

    else if(!strcmp(arg[1], "333")) // RPL_TOPIC_WHO_TIME
    {
        struct tm *tm;
        time_t t;
        char *time_str;
        int len;

        logrec=plog->findLoggingChannel(arg[3]);

        if(logrec && logrec->set->LOG_TOPIC)
        {
            t=atol(arg[5]);
            tm=localtime(&t);
            time_str=strdup(asctime(tm));
            len=strlen(time_str);

            if(len>0)
                time_str[len-1]='\0';

            logrec->log("-!- Topic set by %s [%s]", get_hostmask(arg[4]), time_str);
            free(time_str);
        }
    }

    else if(!strcmp(arg[1], "353")) // RPL_NAMREPLY
    {
        logrec=plog->findLoggingChannel(arg[4]);

        if(logrec && logrec->set->LOG_NICKLIST_ON_JOIN)
        {
            ch=ME.findChannel(arg[4]);

            // check if the bot just joined the channel
            if(!ch || !ch->synced())
                logrec->log("-!- Users: %s", srewind(data, 5)+1);
        }
    }

    else if(!strcmp(arg[1], "432")) // ERR_ERRONEOUSNICKNAME
    {
        if(!(net.irc.status & STATUS_REGISTERED) && plog->client->set->LOG && plog->client->set->LOG_ERR_NICK)
            plog->client->log("Erroneous Nickname: %s", arg[3]);
    }

    else if(!strcmp(arg[1], "465")) // ERR_YOUREBANNEDCREEP
    {
        if(plog->client->set->LOG && plog->client->set->LOG_GOT_KLINED)
        {
            strcpy(arg[4], arg[4]+1);
            arg[4][strlen(arg[4])-1]='\0';
            plog->client->log("I am K-lined on %s with hostmask %s (%s)", arg[0], arg[4], srewind(data, 10));
        }
    }

    else if(!strcmp(arg[1], "471") && plog->client->set->LOG && plog->client->set->LOG_JOIN_FAILED)  // ERR_CHANNELISFULL
        plog->client->log("Cannot join to channel %s (Channel is full)", arg[3]);

    else if(!strcmp(arg[1], "473") && plog->client->set->LOG && plog->client->set->LOG_JOIN_FAILED)  // ERR_INVITEONLYCHAN
        plog->client->log("Cannot join to channel %s (You must be invited)", arg[3]);

    else if(!strcmp(arg[1], "474") && plog->client->set->LOG && plog->client->set->LOG_JOIN_FAILED)  // ERR_BANNEDFROMCHAN
        plog->client->log("Cannot join to channel %s (You are banned)", arg[3]);

    else if(!strcmp(arg[1], "475") && plog->client->set->LOG && plog->client->set->LOG_JOIN_FAILED)  // ERR_BADCHANNELKEY
        plog->client->log("Cannot join to channel %s (Bad channel key)", arg[3]);

    else if(!strcmp(arg[1], "484") && plog->client->set->LOG && plog->client->set->LOG_CONNECTION_RESTRICTED) // ERR_RESTRICTED
        plog->client->log("My connection is restricted");
}

/** parses .bc command.
 */

void hook_botnetcmd(const char *from, const char *cmd)
{
    char arg[10][MAX_LEN];
    ptrlist<Plog::chanRecord>::iterator logrec_iter;
    Plog::chanRecord *logrec;

    str2words(arg[0], cmd, 10, MAX_LEN, 0);

    if(strcasecmp(arg[1], "plog"))
        return;

    if(!arg[2] || !(*arg[2]) || !strcasecmp(arg[2], "help"))
    {
        net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " Available commands:", NULL);
        net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, "  .bc ", (const char*)config.handle, " plog set [<key> [<value>]]", NULL);
        net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, "  .bc ", (const char*)config.handle, " plog clientset [<key> [<value>]]", NULL);
        net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, "  .bc ", (const char*)config.handle, " plog addchan <channel>", NULL);
        net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, "  .bc ", (const char*)config.handle, " plog chans", NULL);
        net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, "  .bc ", (const char*)config.handle, " plog delchan <channel>", NULL);
        net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, "  .bc ", (const char*)config.handle, " plog chanset <channel> [<key> [<value>]]", NULL);
    }

    else if(!strcasecmp(arg[2], "set"))
    {
        if(plog->globalSet->parseUser(arg[0], arg[3], srewind(cmd, 4), "set"))
            plog->setSave();
    }

    else if(!strcasecmp(arg[2], "clientset"))
    {
        if(plog->client->set->parseUser(arg[0], arg[3], srewind(cmd, 4), "clientset"))
            plog->setSave();
    }

    else if(!strcasecmp(arg[2], "addchan"))
    {
        if(!arg[3] || !(*arg[3]))
        {
            net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " addchan <channel>", NULL);
            return;
        }

        if((logrec=plog->addChannel(arg[3])))
        {
            net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " Channel ", logrec->target, " has been added.", NULL);

            if(!logrec->set->LOG)
            {
                net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " Please note that logging is still disabled.", NULL);
                net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " You may now choose what you want to log. To see all options type: .bc ", (const char*) config.handle, " plog chanset ", logrec->target, NULL);
                net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " And then enable logging: .bc ", (const char*) config.handle, " plog chanset ", logrec->target, " log ON", NULL);
            }

            plog->setSave();
        }

        else
            net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " Channel exists already.", NULL);
    }

    else if(!strcasecmp(arg[2], "chans") || !strcasecmp(arg[2], "channels"))
    {
        ptrlist<Plog::chanRecord>::iterator logrec;

        net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " Channels:", NULL);

        for(logrec=plog->chanlist.begin(); logrec; logrec++)
           net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, "  ", logrec->target, NULL);
    }

    else if(!strcasecmp(arg[2], "delchan"))
    {
        if(!arg[3] || !(*arg[3]))
        {
            net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " delchan <channel>", NULL);
            return;
        }

        if(plog->delChannel(arg[3]))
            net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " Channel ", arg[3], " has been deleted.", NULL);

        else
            net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " Channel does not exist.", NULL);
    }

    else if(!strcasecmp(arg[2], "chanset"))
    {
        if(!arg[3] || !(*arg[3]))
        {
            net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " chanset <channel> [<key> [<value>]]", NULL);
            return;
        }

        if(!strcmp(arg[3], "*"))
        {
            for(logrec_iter=plog->chanlist.begin(); logrec_iter; logrec_iter++)
            {   
                if(logrec_iter->set->parseUser(arg[0], arg[4], srewind(cmd, 5), logrec_iter->target))
                    plog->setSave();
            }
        }

        else
        {
            if(!(logrec=plog->findChannel(arg[3])))
            {
                net.sendOwner(arg[0], PLOG_PARTYLINE_PREFIX, " Unknown channel.", NULL);
                return;
            }

            if(logrec->set->parseUser(arg[0], arg[4], srewind(cmd, 5), logrec->target))
                plog->setSave();
        }
    }
}

void hook_timer()
{
    plog->autoSaveConf();
}

/** chan constructor.
 * just for the sake of completeness.
 */

void chanConstructor(chan *me)
{
}

/** chan destructor.
 * This function will be used to find out when the bot has left the channel.
 * Logfile will be closed then.
 *
 * FIXME: This function is sometimes executed while the bot is still on channel.
 *        What do we do with the upcomining traffic then? At the moment logfile
 *        will be opened again and never be closed.
 */

void chanDestructor(chan *me)
{
    Plog::chanRecord *logrec=plog->findLoggingChannel(me->name);

    if(logrec)
        logrec->stop();
}

extern "C" module *init()
{
    module *m=new module("plog", "patrick <patrick@psotnic.com>", "0.1");
    m->hooks->privmsg=hook_privmsg;
    m->hooks->notice=hook_notice;
    m->hooks->ctcp=hook_ctcp;
    m->hooks->invite=hook_invite;
    m->hooks->rawirc=hook_raw;
    m->hooks->connected=hook_connected;
    m->hooks->disconnected=hook_disconnect;
    m->hooks->botnetcmd=hook_botnetcmd;
    m->hooks->timer=hook_timer;
    m->hooks->new_chan=chanConstructor;
    m->hooks->del_chan=chanDestructor;

    plog=new Plog();
    plog->loadConf();

    return m;
}

extern "C" void destroy()
{
    plog->chanlist.clear();
    delete plog->client;
    delete plog->globalSet;
    delete plog;
}
