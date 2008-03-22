/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "prots.h"
#include "global-var.h"

static char arg[11][MAX_LEN], *a, buf[MAX_LEN];
static chan *ch;
static chanuser *p;
static int i;

void parse_irc(char *data)
{
	if(!strlen(data))
        return;

	net.irc.killTime = NOW + set.CONN_TIMEOUT;

	str2words(arg[0], data, 11, MAX_LEN, 1);

    /* debug */
#ifdef HAVE_DEBUG

    if(debug)
	{
		if(!strcmp(arg[1], "PRIVMSG"))
    	{
        	ch = ME.findChannel(arg[2]);

			if(ch)
        	{
            	if(!strcasecmp(arg[3], "!debug"))
            	{
                	printf("### DEBUG ###\n");
                	printf("CHANNELS: %d\n", ME.channels);
                	ME.display();
                	ch->display();
					return;
            	}
				if(!strcmp(arg[3], "!op"))
				{
					chanuser u(arg[0]);
					ch->modeQ[PRIO_LOW].add(NOW + atoi(arg[4]), "+o", u.nick);
				}
			}

			if(!strcmp(arg[3], "!re"))
			{
				ME.recheckFlags();
				return;
			}
			
			if(!strcmp(arg[3], "!crash"))
			{
			//	char *buf = NULL;
			//	*buf = 9;
			}
		}
	}
#endif

    /* reaction */
    if(!strcmp(arg[1], "JOIN"))
    {
        int netjoin = arg[2][0] != ':';
        if(netjoin)
            a = arg[2];
        else
            a = arg[2] + 1;

		if(!strcasecmp(ME.mask, arg[0]))
        {
			if(!ME.findNotSyncedChannel(a))
			{
            	if((i = userlist.findChannel(a)) != -1)
            	{
                	ME.createNewChannel(a);
                	if(!(userlist.chanlist[i].status & WHO_SENT))
					{
						net.irc.send("WHO ", a, NULL);
                		penalty++;
					}
            	}
				//if thats !channel maybe we have to change its name
				//to !0WN3Dchannel
				//fixme: is that necessary?
				else if(*a == '!' && strlen(a) > 6)
				{
					buf[0] = '!';
					strcpy(buf+1, a + 6);
					if((i = userlist.findChannel(buf)) != -1)
					{
						userlist.chanlist[i].name = a;
						ME.createNewChannel(a);
						if(!(userlist.chanlist[i].status & WHO_SENT))
						{
							net.irc.send("WHO ", a, NULL);
                			penalty++;
						}
					}
				}
				else
				{
					net.irc.send("PART ", a, " :wtf?", NULL);
					penalty += 3;
				}
			}
			else
			{
				net.send(HAS_N, "\0039 >> Double join to ", a, " <<\003", NULL);
			}
        }
        else
        {
            ch = ME.findChannel(a);
            if(ch)
                ch->gotJoin(arg[0], netjoin ? NET_JOINED : 0);
#ifdef HAVE_DEBUG
			else if(!ME.findNotSyncedChannel(a))
				net.send(HAS_N, "\0039 >>> Join observed to non exitsing channel ", a, "<<\003", NULL);
#endif
		}
        return;
    }
    if(!strcmp(arg[1], "MODE"))
    {
        ch = ME.findChannel(arg[2]);
        if(ch)
        {
            a = push(NULL, arg[4], " ", arg[5], " ", arg[6], " ", arg[7], NULL);
            ch->gotMode(arg[3], a, arg[0]);
            free(a);
        }
        return;
    }
    if(!strcmp(arg[1], "KICK"))
    {
        if(!strcasecmp(ME.nick, arg[3]))
        {
			ch = ME.findChannel(arg[2]);
			if(ch)
				ch->buildAllowedOpsList(arg[0]);
			ME.removeChannel(arg[2]);
           	ME.rejoin(arg[2], set.REJOIN_DELAY);
        }
        else
        {
            ch = ME.findChannel(arg[2]);
            if(ch) {
                HOOK(kickMsg, kickMsg(ch, arg[3], arg[0], srewind(data, 4)));
		stopParsing=false;
                ch->gotKick(arg[3], arg[0]);
            }
        }
        return;
    }
    if(!strcmp(arg[1], "PART"))
    {
		HOOK(pre_part, pre_part(arg[0], arg[2]));
		stopParsing=false;

		HOOK(pre_partMsg, pre_partMsg(arg[0], arg[2], srewind(data,3), false));
		stopParsing=false;
		
        if(!strcasecmp(ME.mask, arg[0]))
        {
            ME.removeChannel(arg[2]);
			penalty += 4;
        }
        else
        {
            ch = ME.findChannel(arg[2]);
            if(ch)
            {
                mem_strncpy(a, arg[0], abs(arg[0] - strchr(arg[0], '!')) + 1);
                ch->gotPart(a, 0);
                free(a);
            }

        }
		HOOK(post_part, post_part(arg[0], arg[2]));
		stopParsing=false;

		HOOK(post_partMsg, post_partMsg(arg[0], arg[2], srewind(data,3), false));
		stopParsing=false;
        return;
    }
    if(!strcmp(arg[1], "NICK"))
    {
        ME.gotNickChange(arg[0], arg[2]);
        return;
    }
    if(!strcasecmp(arg[1], "352"))
    {
        ch = ME.findNotSyncedChannel(arg[3]);
        if(ch && !ch->synced())
        {

			wasoptest *w = userlist.chanlist[ch->channum].allowedOps;
			if(w && w->since + w->TOL <= NOW)
			{
				delete userlist.chanlist[ch->channum].allowedOps;
				userlist.chanlist[ch->channum].allowedOps = NULL;
			}

			a = push(NULL, arg[7], "!", arg[4], "@", arg[5], NULL);
            p = ch->gotJoin(a, (strchr(arg[8], '@') ? IS_OP : 0) | (strchr(arg[8], '+') ? IS_VOICE : 0));

			if(!strcasecmp(arg[7], ME.nick))
            	ch->me = p;

			free(a);
        }
        return;
    }
    if(!strcmp(arg[1], "315"))
    {
        ch = ME.findNotSyncedChannel(arg[3]);
        if(ch)
		{
			if(ch->synced())
			{
				//net.send(HAS_N, "\0039[!] BUG, BUG >> Double WHO on ", (const char *) ch->name, " << BUG, BUG", NULL);
				return;
			}
			if(!ch->users.entries())
			{
				net.send(HAS_N, "[D] Empty WHO RPL", NULL);
				bk;
				return;
			}

			ch->synlevel = 1;

			if(userlist.chanlist[ch->channum].allowedOps)
			{
				delete userlist.chanlist[ch->channum].allowedOps;
				userlist.chanlist[ch->channum].allowedOps = NULL;
			}
			if(!ch->opedBots.entries())
				userlist.chanlist[ch->channum].status &= ~SET_TOPIC;


			HOOK(justSynced, justSynced(ch));
			stopParsing=false;
		}
        return;
    }
    if(!strcmp(arg[1], "324"))
    {
        ch = ME.findChannel(arg[3]);
        if(ch)
        {
			ch->limit = 0;
			ch->updateKey("");
            ch->setFlags(arg[4]);

			a = arg[4];

			for(i=5; *a && i < 7; ++a)
			{
				switch(*a)
				{
					case 'l':
						ch->limit = atol(arg[i++]);
						break;
					case 'k':
						ch->updateKey(arg[i++]);
						break;
					default: break;
				}
			}

            ++ch->synlevel;
        	if(ch->limit == -1)
                ch->nextlimit = -1;
			else
                ch->nextlimit = NOW + set.ASK_FOR_OP_DELAY;

			/*
			if(!ch->toKick.entries() && ch->opedBots.entries() + ch->botsToOp.entries() == 1 &&
					!(ch->flags & (FLAG_N | FLAG_S | FLAG_T)))
			{
				ch->modeQ[PRIO_LOW].add(NOW, "+s");
				ch->modeQ[PRIO_LOW].add(NOW, "+n");
				ch->modeQ[PRIO_LOW].add(NOW, "+t");
			}
			*/
		}
		else DEBUG(printf("unknown 324 for %s\n", arg[3]));
        return;
    }
    if(!strcmp(arg[1], "QUIT"))
    {
		ME.gotUserQuit(arg[0], srewind(data, 2));
        return;
    }
    if(!strcmp(arg[0], "PING"))
    {
        net.irc.send("PONG ", arg[1], NULL);
        return;
    }
    if(!strcmp(arg[1], "433"))
    {
        if(net.irc.status & STATUS_REGISTERED)
            ME.nextNickCheck = NOW + set.KEEP_NICK_CHECK_DELAY;
        else
        {
            if(config.altnick.getLen() && !strcmp(arg[3], config.nick) && strcmp(arg[3], config.altnick))
                net.irc.send("NICK ", (const char*) config.altnick, NULL);
            else
                ME.registerWithNewNick(arg[3]);
           sleep(1);
        }
        return;
    }
    if(!strcmp(arg[1], "437"))
    {
        if(net.irc.status & STATUS_REGISTERED)
        {
            i = userlist.findChannel(arg[3]);
            if(i == -1)
            {
                /* Nick is temp...*/
                ME.nextNickCheck = NOW + set.KEEP_NICK_CHECK_DELAY;
            }
            else
            {
                /* Channel is temp... */
                ME.rejoin(arg[3], set.REJOIN_FAIL_DELAY);
            }
        }
        else
			ME.registerWithNewNick(arg[3]);
		
		return;
    }
	if(!strcmp(arg[1], "432"))
	{
		if(!(net.irc.status & STATUS_REGISTERED))
		{
			strncpy(arg[3], config.nick, MAX_LEN);
			ME.registerWithNewNick(arg[3]);
		}
	}

	if(!strcmp(arg[1], "043"))
	{
		/* collision, try to get nick back in 30 mins */
		ME.nextNickCheck = NOW + 1800;
		return;
	}

	if(!strcmp(arg[1], "001") && !(net.irc.status & STATUS_REGISTERED))
    {
		if(!match("*!*@*", arg[9]))
		{
			net.send(HAS_N, "[-] ", arg[0], " is not an ircnet like server !!!", NULL);
			net.irc.close("Only ircnet like servers are supported");
			return;
		}

		if(userlist.me()->flags[GLOBAL] & HAS_P)
			hostNotify = 1;
		else
			hostNotify = 0;

		mem_strcpy(net.irc.name, arg[0]);
		mem_strcpy(net.irc.origin, arg[0]);
        net.irc.status |= STATUS_REGISTERED;
        net.irc.lastPing = NOW;
		chanuser u(arg[9], NULL, 0, false);
		ME.nick = u.nick;
		ME.ident = u.ident;
		ME.host = u.host;
		ME.mask = arg[9];
        srand();

		net.propagate(NULL, S_CHNICK, " ", (const char *) ME.nick, " ", net.irc.name, NULL);
        if(strcmp(ME.nick, config.nick))
            ME.nextNickCheck = NOW + set.KEEP_NICK_CHECK_DELAY;
        else
            ME.nextNickCheck = 0;

		if(creation)
        {
			printf("[*] Please do `/msg %s mainowner <handle> <password>'\n", (const char *) ME.nick);
			printf("[*] eg. `/msg %s mainowner %s foobar'\n", (const char *) ME.nick, getenv("USER"));
        }
		else
			net.send(HAS_N, "[*] Connected to ", net.irc.name, " as ", (const char *) ME.nick, NULL);

		if(antiidle.away)
		{
			net.irc.send("AWAY :", antiidle.away, NULL);
			penalty = 4;
		}
		else penalty = 2;

		ME.checkMyHost("*", true);

		net.irc.send("stats L ", (const char *) ME.nick, NULL);
		penalty += 2;

		if(!creation)
		{
			HOOK(connected, connected());
			stopParsing=false;
		}

		return;
    }
	if(!strcmp(arg[1], "211") && strlen(arg[3]))
	{
		char *at = strchr(arg[3], '@');

		if(at)
			ME.ircip.assign(at+1, strlen(at+1)-1);
		return;
	}

	if(!strcmp(arg[1], "002"))
	{
		if(!strcmp(arg[9], "2.11") || match("2.11.*", arg[9]))
			net.irc.status |= STATUS_211;

		return;
	}
	if(!strcmp(arg[1], "042"))
	{
		ME.uid = arg[3];
		return;
	}
	if(!strcmp(arg[1], "332"))
	{
		chan *ch = ME.findNotSyncedChannel(arg[3]);
		if(ch)
		{
			ch->topic = srewind(data, 4)+1;
			HOOK(topicChange, topicChange(ch, ch->topic, NULL, NULL));
			stopParsing=false;
		}
		return;
	}

	if(!strcmp(arg[1], "TOPIC"))
	{
		chan *ch = ME.findChannel(arg[2]);
		if(ch)
		{
			chanuser *u = ch->getUser(arg[0]);
			pstring<> oldtopic(ch->topic);
			ch->topic = srewind(data, 3)+1;

			HOOK(topicChange, topicChange(ch, ch->topic, u, oldtopic));
			stopParsing=false;
		}
	}

    if(!strcmp(arg[1], "INVITE"))
    {
		chan *ch = NULL;
        if((i = userlist.findChannel(arg[3])) != -1 && !(ch = ME.findChannel(arg[3])))
        {
            if(!(userlist.chanlist[i].status & JOIN_SENT) && userlist.isRjoined(i))
            {
				net.irc.send("JOIN ", arg[3], " ", (const char *) userlist.chanlist[i].pass, NULL);
                userlist.chanlist[i].status |= JOIN_SENT;
            }
        }

		HOOK(invite, invite(arg[0], arg[3], ch, i == -1 ? NULL : &userlist.chanlist[i]));
		stopParsing=false;
        return;
    }

	if(!strcmp(arg[1], "NOTICE"))
	{
		HOOK(notice, notice(arg[0], arg[2], srewind(data, 3) + 1));

		if(stopParsing)
		{
			stopParsing=false;
			return;
		}

		if(strchr(arg[0], '.') && !strchr(arg[0], '@'))
		{
			chan *ch = ME.findChannel(arg[2]);

			if(ch)
			{
				if(!strcmp(arg[6], "invitation"))
				{
					if(strchr(arg[8], '@'))
						ME.overrider = arg[8];
					else
						ME.overrider = "-";

					return;
				}
			}
		}

		return;
	}

    if(!strcmp(arg[1], "PRIVMSG"))
    {
		/* CTCP */
        if(arg[3][0] == '\001')
        {
            if(data[strlen(data)-1] != '\001')
                return;
            data[strlen(data)-1] = '\0';
            for(i=0; i<3; )
                if(*data++ == ' ')
                    ++i;
            parse_ctcp(arg[0], data + 2, arg[2]);
            return;
        }

	HOOK(privmsg, privmsg(arg[0], arg[2], srewind(data, 3) + 1));

	if(stopParsing)
	{
		stopParsing=false;
		return;
	}

		if(!strcmp(ME.nick, arg[2]))
		{
			/* op pass #chan */
			if(!strcmp(arg[3], "op") || !strcmp(arg[3], ".op") || !strcmp(arg[3], "!op"))
        	{
				if(strlen(arg[5]))
				{
    	        	ch = ME.findChannel(arg[5]);
        	    	if(ch)
            		{
	                	p = ch->getUser(arg[0]);
    	            	if(p && p->flags & HAS_O && !(p->flags & IS_OP))
        	        	{
            	        	HANDLE *h = userlist.matchPassToHandle(arg[4], arg[0], 0);
                	    	if(h) ch->modeQ[PRIO_LOW].add(NOW, "+o", p->nick);
	                	}
					}
				}
				else
				{
					i=0;
					foreachSyncedChannel(ch)
					{
						p = ch->getUser(arg[0]);
						if(p && p->flags & HAS_O && !(p->flags & IS_OP))
            	    	{
	                    	HANDLE *h = userlist.matchPassToHandle(arg[4], arg[0], 0);
            	        	if(h) ch->modeQ[PRIO_LOW].add(NOW + i, "+o", p->nick);
							i+=4;
                		}
					}
				}
				return;

	        }
			
			if(!strcmp(arg[3], "voice") || !strcmp(arg[3], ".voice") || !strcmp(arg[3], "!voice"))
        	{
				if(strlen(arg[5]))
				{
    	        	ch = ME.findChannel(arg[5]);
        	    	if(ch)
            		{
	                	p = ch->getUser(arg[0]);
    	            	if(p && p->flags & (HAS_V | HAS_O) && !(p->flags & IS_VOICE))
        	        	{
            	        	HANDLE *h = userlist.matchPassToHandle(arg[4], arg[0], 0);
                	    	if(h) ch->modeQ[PRIO_LOW].add(NOW, "+v", p->nick);
	                	}
					}
				}
				else
				{
					i=0;
					foreachSyncedChannel(ch)
					{
						p = ch->getUser(arg[0]);
						if(p && p->flags & (HAS_V | HAS_O) && !(p->flags & IS_VOICE))
            	    	{
	                    	HANDLE *h = userlist.matchPassToHandle(arg[4], arg[0], 0);
            	        	if(h) ch->modeQ[PRIO_LOW].add(NOW + i, "+v", p->nick);
							i+=4;
                		}
					}
				}
				return;
	        }
			
			

	        /* invite pass #chan */
			if((!strcmp(arg[3], "invite") || !strcmp(arg[3], ".invite") || !strcmp(arg[3], "!invite")) && strlen(arg[5]))
        	{
	            ch = ME.findChannel(arg[5]);
    	        if(ch)
	            {
    	            HANDLE *h = userlist.matchPassToHandle(arg[4], arg[0], 0);
        	        if(h && (h->flags[MAX_CHANNELS] & HAS_F || h->flags[ch->channum] & HAS_F))
            	    {
	                    ch->invite(arg[0]);
    	            }
        	    }
	            return;
    	    }
	        /* key pass chan */
			if((!strcmp(arg[3], "key") || !strcmp(arg[3], ".key") || !strcmp(arg[3], "!key")) && strlen(arg[5]))
	        {
    	        ch = ME.findChannel(arg[5]);
        	    if(ch && ch->key && *ch->key)
            	{
	                HANDLE *h = userlist.matchPassToHandle(arg[4], arg[0], 0);
    	            if(h && (h->flags[MAX_CHANNELS] & HAS_F || h->flags[ch->channum] & HAS_F))
						ctcp.push("NOTICE ", arg[0], " :", arg[5], "'s key: ", (const char *) ch->key, NULL);
	            }
				return;
	        }
		/* pass oldpass newpass */
		if(config.bottype == BOT_MAIN && (!strcmp(arg[3], "pass") || !strcmp(arg[3], ".pass") || !strcmp(arg[3], "!pass")) && strlen(arg[4]))
		{
		    HANDLE *h;
		    if(strlen(arg[5])) // pass change
		    {
			h = userlist.matchPassToHandle(arg[4], arg[0], 0);
			
			if(h)
			{
			    if(strlen(arg[5]) < 8)
			    {
		    		ctcp.push("NOTICE ", arg[0], " :New password must be at least 8 characters long!", NULL);
			    }
			    else
			    {
				char buf[MAX_LEN];
    	        		userlist.changePass(arg[3], arg[5]);
				net.send(HAS_N, "[*] \002",(const char *) h->name, "\002 has changed his password", NULL);
				net.send(HAS_B, S_PASSWD, " ", arg[1], " ", quoteHexStr(h->pass, buf), NULL);

				ctcp.push("NOTICE ", arg[0], " :Password changed", NULL);
                     		++userlist.SN;
                		userlist.nextSave = NOW + SAVEDELAY;			    
			    }
			}
			return;
		    }
		    else // pass set
		    {
			h = userlist.findHandleByHost(arg[0]);
			if(h && h != userlist.first && (!strcmp((const char*) h->pass, "0000000000000000") || !strlen((const char*) h->pass))) // no pass
			{
			    if(strlen(arg[4]) < 8)
			    {
		    		ctcp.push("NOTICE ", arg[0], " :Password must be at least 8 characters long!", NULL);			    
			    }
			    else
			    {
				char buf[MAX_LEN];
    	        		userlist.changePass(arg[3], arg[5]);
				net.send(HAS_N, "[*] \002",(const char *) h->name, "\002 has set his password", NULL);
				net.send(HAS_B, S_PASSWD, " ", arg[1], " ", quoteHexStr(h->pass, buf), NULL);

				ctcp.push("NOTICE ", arg[0], " :Password set", NULL);
                     		++userlist.SN;
                		userlist.nextSave = NOW + SAVEDELAY;			    				
			    }
			}
			return;
		    }
		}   
	    
			/* chat */
			if(config.bottype == BOT_MAIN && (!strcmp(arg[3], "chat") || !strcmp(arg[3], ".chat") || !strcmp(arg[3], "!chat")))
			{
				if(userlist.hasPartylineAccess(arg[0]))
				{
					snprintf(buf, MAX_LEN, ":\001DCC CHAT CHAT %d %d\001", inet_network(config.myipv4), (int) config.listenport);
					ctcp.push("PRIVMSG ", arg[0], " ", buf, NULL);
				}
				return;
			}

    	    /* mainowner */
        	if(creation && !strcmp(arg[3], "mainowner") && strlen(arg[5]))
	        {
    	        if(!strcmp(arg[4], "idiots"))
        	    {
            	    net.irc.send("PRIVMSG ", arg[0], " :Invalid handle", NULL);
                	return;
	            }
    	        if(strlen(arg[5]) < 8)
        	    {
            	    net.irc.send("PRIVMSG ", arg[0], " :Password must be at least 8 characters long", NULL);
                	return;
	            }

    	        HANDLE *h = userlist.addHandle(arg[4], 0, 0, 0, 0, arg[4]);
        	    if(h)
            	{
                	sprintf(buf, "*%s", strchr(arg[0], '!'));
	                userlist.addHost(h, buf, arg[4], NOW);
    	            userlist.changePass(arg[4], arg[5]);
        	        userlist.changeFlags(arg[4], "aofmnstx", "");
	
    	            net.irc.send("PRIVMSG ", arg[0], " :Account created", NULL);
        	        printf("[*] Added user `%s' with host `%s' and password `%s'\n", arg[4], buf, arg[5]);
					printf("[*] Now do `/chat %s' and supply owner pass from the config file and %s's password\n", (const char *) ME.nick, arg[4]);

                	++userlist.SN;
	                userlist.save(config.userlist_file);

#ifdef HAVE_DEBUG
					if(!debug)
#endif
                    lurk();
        	        creation = 0;
            	    ++userlist.SN;
	            }
				return;
	        }
		}

        return;
    }
    /* some numeric replies */
    if((i = atoi(arg[1])))
    {
        ch = ME.findChannel(arg[3]);
        if(ch)
        {
			protmodelist::entry *global, *local;

            switch(i)
            {
            	case RPL_BANLIST:
					ch->list[BAN].add(arg[4], "", protmodelist::isSticky(arg[4], BAN, ch) ? 0 : set.BIE_MODE_BOUNCE_TIME);
    	            return;
				case RPL_ENDOFBANLIST:
            	    ++ch->synlevel;
                	return;
            	case RPL_EXCEPTLIST:
					local=ch->protlist[EXEMPT]->find(arg[4]);
					global=userlist.protlist[EXEMPT]->find(arg[4]);
					ch->list[EXEMPT].add(arg[4], "", ((local && local->sticky) || (global && global->sticky)) ? 0 : set.BIE_MODE_BOUNCE_TIME);
					if(ch->chset->USER_EXEMPTS==2 && !local && !global)
						ch->modeQ[PRIO_LOW].add(NOW+penalty+10, "-e", arg[4]);
                	return;
            	case RPL_ENDOFEXCEPTLIST:
                	++ch->synlevel;
                	return;
            	case RPL_INVITELIST:
					local=ch->protlist[INVITE]->find(arg[4]);
					global=userlist.protlist[INVITE]->find(arg[4]);
					ch->list[INVITE].add(arg[4], "", ((local && local->sticky) || (global && global->sticky)) ? 0 : set.BIE_MODE_BOUNCE_TIME);
					if(ch->chset->USER_INVITES==2 && !local && !global)
						ch->modeQ[PRIO_LOW].add(NOW+penalty+10, "-I", arg[4]);
                	return;
            	case RPL_ENDOFINVITELIST:
                	++ch->synlevel;
                	return;
				case RPL_REOPLIST:
					local=ch->protlist[REOP]->find(arg[4]);
					global=userlist.protlist[REOP]->find(arg[4]);
					ch->list[REOP].add(arg[4], "", ((local && local->sticky) || (global && global->sticky)) ? 0 : set.BIE_MODE_BOUNCE_TIME);
					if(ch->chset->USER_REOPS==2 && !local && !global)
						ch->modeQ[PRIO_LOW].add(NOW+penalty+10, "-R", arg[4]);
                	return;
				case RPL_ENDOFREOPLIST:
					++ch->synlevel;
                	return;
				case ERR_NOSUCHNICK:
					penalty -= 2;
					return;
				case ERR_NOSUCHCHANNEL:
					penalty--;
					return;
				default:
					break;
			}
		}
		//not on channel
		else
		{
			//if arg[3] is channel name, set rejoin delay
			if(userlist.findChannel(arg[3]) != -1)
				ME.rejoin(arg[3], set.REJOIN_FAIL_DELAY);
			//else if(chan::valid(arg[3]))
			//	net.send(HAS_N, "[!] \002>> Strange server resposne: ", data, " <<\002", NULL);

			srand();

			switch(i)
			{
				//+i: S_INVITE <seed> <channel>
				case ERR_INVITEONLYCHAN:
				{
					if(userlist.findChannel(arg[3]) != -1)
						net.propagate(NULL, S_INVITE, " ", itoa(rand() % 2048), " ", arg[3], NULL);
					return;
				}
				//+b: S_UNBANME <seed> <nick!ident@host> <channel> [ip] [uid]
				case ERR_BANNEDFROMCHAN:
				{
					if(userlist.findChannel(arg[3]) != -1)
						net.propagate(NULL, S_UNBANME, " ", itoa(rand() % 2048), " ", (const char *) ME.mask,
									  " ", arg[3],  " ", (const char *) ME.ircip, " ", (const char *) ME.uid, NULL);
						return;
				}
				//+l: S_BIDLIMIT <seed> <channel>
				case ERR_CHANNELISFULL:
				{
					if(userlist.findChannel(arg[3]) != -1)
						net.propagate(NULL, S_BIDLIMIT, " ", itoa(rand() % 2048), " ", arg[3], NULL);
					return;
				}
				//+k: S_KEY <seed> <channel>
				case ERR_BADCHANNELKEY:
				{
					if(userlist.findChannel(arg[3]) != -1)
						net.propagate(NULL, S_KEY, " ", itoa(rand() % 2048), " ", arg[3], NULL);
					return;
				}
				//k:lined
				case ERR_YOUREBANNEDCREEP:
				{
					net.irc.status |= STATUS_KLINED;
					mem_strcpy(net.irc.name, arg[0]);
					a = srewind(data, 10);
					if(a)
						net.irc.close(a);
					else
						net.irc.close("K-lined");
					return;
				}
				default:
                	break;
            }
        }
    }

	HOOK(crap, crap(data));
	stopParsing=false;
}
