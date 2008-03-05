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

static char mode[2][MODES_PER_LINE];
static char *arg[MODES_PER_LINE];
static modeq::modeq_ent *mqc[MODES_PER_LINE];
static char *a, *b;
static char sign;
static char modeBuf[3];
static chanuser *nickHandle = NULL, *argHandle, serverHandle("");
static chanuser *multHandle[6];
static ptrlist<chanuser>::iterator p;
static int i, j, just_opped, oped, mypos, n[6], hash, ops, mq, tmp;
static unsigned int number; // offence counter

/* function shourtcuts :) */
/* has minus flag x = 1, y = 0*/
/* has plus flag x = 1, y = 1*/
#define __hasFlag(x, letter, y) chset->PROTECT_CHMODES.hasFlag(x, letter, y)
/* has minus or plus flag .. x =1, y = 0  z = 1*/
#define __hasFlags(x, letter, y, z) (chset->PROTECT_CHMODES.hasFlag(x, letter, y) || chset->PROTECT_CHMODES.hasFlag(x, letter, z))
/* getKey */
#define __getKey() chset->PROTECT_CHMODES.getKey()
/* getLimit */
#define __getLimit() chset->PROTECT_CHMODES.getLimit()
/* end of macros */

void chan::gotMode(const char *modes, const char *args, const char *mask)
{
	/* initialization */
	oped = just_opped = 0;
	mypos = -1;
	mq = 0;
	memset(mode, 0, sizeof(mode));
	memset(arg, 0, sizeof(arg));
	sign = '+';
	
	number = 0;
	
	/* user, ircerver detection */
	nickHandle = getUser(mask);
	if(!nickHandle) nickHandle = &serverHandle;

	/* mode striper */
 	for(i=j=0,ops=0; i<MODES_PER_LINE && (unsigned int) j < strlen(modes); i++, j++)
	{
		if(modes[j] == '+' || modes[j] == '-')
  		{
			sign = modes[j];
   			j++;
		}
		mode[0][i] = sign;
		mode[1][i] = modes[j];
		if(sign == '+' && mode[1][i] == 'o') ops++;
	}
	
	/* we backup args in `b' var cause address will be changed in arg striper */
	if((int) chset->IDIOTS && nickHandle != &serverHandle)
	{
    	    if(args && *args)
    		b = push(NULL, (const char *) "mode ", modes, (const char *) " ", args, NULL);
	    else
	        b = push(NULL, (const char *) "mode ", modes, NULL);
						    
	    /* should we remove spaces from end of reason? */    
	    //b = rtrim(b);
	}

	/* arg striper */
	for(i = 0; i<MODES_PER_LINE; i++)
	{
		if(strchr(ARG_MODES, mode[1][i]) && !(mode[0][i] == '-' && mode[1][i] == 'l'))
		{
			a = strchr(args, ' ');
			if(a) mem_strncpy(arg[i], args, abs(args - a)+1);
			else
			{
				if(strlen(args)) mem_strcpy(arg[i], args);
				break;
			}
			args = a+1;
		}
	}

	/* mode parser */
	for(i=0; i<MODES_PER_LINE; i++)
	{
		if(mode[0][i] == '+')
		{
   			switch(mode[1][i])
			{
				///////////////////
				case 'o':
				argHandle = getUser(arg[i]);
				if((argHandle->flags & (HAS_B | IS_OP)) == HAS_B)
				{
					botsToOp.remove(argHandle);
					opedBots.sortAdd(argHandle);
					oped++;
					initialOp = NOW;
					if(opedBots.entries() == 1 && chset->TAKEOVER && argHandle == me)
						userlist.chanlist[channum].status |= SET_TOPIC;
				}

				if(argHandle->flags & HAS_O)
					toOp.remove(argHandle);

				tmp = argHandle->flags & IS_OP;

				argHandle->flags |= IS_OP;
				argHandle->flags &= ~OP_SENT;

				if((!(nickHandle->flags & HAS_M) || (ops > 1 && !(nickHandle->flags & HAS_N)))
				 && !tmp && ((nickHandle == &serverHandle) ?
				 	(chset->STOP_NETHACK ? (chset->WASOPTEST ? !wasop->remove(argHandle) : 0) : 0) : chset->BITCH))
				{
					if(((nickHandle == &serverHandle) ? 1 : !(argHandle->flags & HAS_O)) &&
						!(argHandle->flags & HAS_F))
					{
						if(!(argHandle->flags & (HAS_B | HAS_N))) toKick.sortAdd(argHandle);
						if(*nickHandle->nick) 
						{
						    number++;
						    toKick.sortAdd(nickHandle);
						}
					}
				}
				if(argHandle->flags & HAS_D) toKick.sortAdd(argHandle);

				if(chset->BITCH && set.DONT_TRUST_OPS && argHandle->flags & HAS_O && !tmp &&
					nickHandle != &serverHandle && !(nickHandle->flags & HAS_F))
				{
					if(set.DONT_TRUST_OPS == 2 && !(argHandle->flags & HAS_S))
						toKick.sortAdd(argHandle);
					if(set.DONT_TRUST_OPS == 1 && !(argHandle->flags & HAS_S) && !(argHandle->flags & HAS_B))
						toKick.sortAdd(argHandle);

					toKick.sortAdd(nickHandle);
					number++;
				}

				if(me == argHandle)
				{
					if(chset->TAKEOVER || since + set.ASK_FOR_OP_DELAY <= NOW ||
							opedBots.entries() < 5 || toKick.entries() > 4)
								mypos = oped - 1;
				}
				break; //+o

				///////////////////
				case 'v':
				argHandle = getUser(arg[i]);
				argHandle->flags |= IS_VOICE;
				argHandle->flags &= ~VOICE_SENT;

				if(argHandle->flags & HAS_Q && !(nickHandle->flags & HAS_N))
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "-v", argHandle->nick);
				break; //+v

				///////////////////
				case 'b':
				{
					bool sticky = protmodelist::isSticky(arg[i], BAN, this);
					list[BAN].add(arg[i], nickHandle->nick, sticky ? 0 : set.BIE_MODE_BOUNCE_TIME);
					sentList[BAN].remove(arg[i]);
					if(gotBan(arg[i], nickHandle) && !sticky)
					{
						mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-b", arg[i]);
						number++;
					}
				}
				break; //+b

				///////////////////
				case 'k':
				updateKey(arg[i]);
				if(__hasFlags(1, 'k', 0, 1)) 
				{
				    if(!(nickHandle->flags & (HAS_N | HAS_B)))
				    {
					if(__hasFlag(1, 'k', 1))
					    mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+k", __getKey());
					if(__hasFlag(1, 'k', 0))
					    mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-k", arg[i]);
				    
					if(nickHandle != &serverHandle)
					{
					    toKick.sortAdd(nickHandle);
					    number++;
					}
				    }
				}
				break; //+k

				///////////////////
				case 'l':

				j = limit;
				limit = strtol(arg[i], NULL, 10);
				if(errno == ERANGE || limit == j)
				{
					break;
				}

				/* +n, +b or irc server has changed the limit */
				if((nickHandle->flags & (HAS_N + HAS_B)) || nickHandle == &serverHandle)
				{
					/* are we in control of the limit? */
					if(chset->LIMIT || __hasFlag(1, 'l', 1))
					{
						/* bot has changed the limit */
						if(nickHandle->flags & HAS_B)
						{
                        			    if(set.PRE_0211_FINAL_COMPAT)
                        			    {
							    if(limit == -1)
                                				nextlimit = -1;
							    else if(nickHandle == me)
                                				nextlimit = NOW + chset->LIMIT_TIME * chset->LIMIT_BOTS;
							    else
                                				nextlimit = NOW + chset->LIMIT_TIME;
                        			    }
                        			    else
                        			    {
                            				if(limit == -1)
                                			    nextlimit = -1;
                            				else
                                			    nextlimit = NOW + chset->LIMIT_TIME_UP;
                        			    }  
						}
						/* owner has changed the limit */
						else if(nickHandle->flags & HAS_N)
						{
							if(limit == -1) nextlimit = -1;
							else nextlimit = NOW + chset->OWNER_LIMIT_TIME;

							/* enforce this limit */
							enforceLimits();
						}
						/* server has changed the limit */
						else if(nickHandle == &serverHandle)
							nextlimit = NOW + chset->OWNER_LIMIT_TIME;
					}
				}
				/* some lame has changed the limit */
				else if(__hasFlags(1, 'l', 0, 1))
				{
					if(__hasFlag(1, 'l', 1))
					    mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+l", itoa(__getLimit()));
					if(__hasFlag(1, 'l', 0))
					    mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-l");
					toKick.sortAdd(nickHandle);
					number++;	
				}
				else if(chset->LIMIT && !__hasFlags(1, 'l',0, 1))
				{
					mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+l", itoa(j));
					toKick.sortAdd(nickHandle);
					number++;					
				}
				break; //+l

				///////////////////
				case 'I':
				{
					protmodelist::entry *global=userlist.protlist[INVITE]->find(arg[i]), *local=protlist[INVITE]->find(arg[i]);

					if(chset->USER_INVITES==1 && !(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
					{
						mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-I", arg[i]);
						toKick.sortAdd(nickHandle);
						number++;
					}

					else if(chset->USER_INVITES==2)
					{
						if(!local && !global)
						{
							if(!(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
							{
								toKick.sortAdd(nickHandle);
								number++;
							}

							mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-I", arg[i]);
						}
					}

					list[INVITE].add(arg[i], nickHandle->nick, ((local && local->sticky) || (global && global->sticky)) ? 0 : set.BIE_MODE_BOUNCE_TIME);
					sentList[INVITE].remove(arg[i]);
				}
				break; //+I
				
				//////////////////
				case 'e':
				{
					protmodelist::entry *global=userlist.protlist[EXEMPT]->find(arg[i]), *local=protlist[EXEMPT]->find(arg[i]);

					if(chset->USER_EXEMPTS==1 && !(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
					{
						mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-e", arg[i]);
						toKick.sortAdd(nickHandle);
						number++;
					}

					else if(chset->USER_EXEMPTS==2)
					{
						if(!local && !global)
						{
							if(!(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
							{
								toKick.sortAdd(nickHandle);
								number++;
							}

							mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-e", arg[i]);
						}
					}

					list[EXEMPT].add(arg[i], nickHandle->nick, ((local && local->sticky) || (global && global->sticky)) ? 0 : set.BIE_MODE_BOUNCE_TIME);
					sentList[EXEMPT].remove(arg[i]);
				}
				break; //+e

				//////////////////
				case 'R':
				{
					protmodelist::entry *global=userlist.protlist[REOP]->find(arg[i]), *local=protlist[REOP]->find(arg[i]);

					if(chset->USER_REOPS==1 && !(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
					{
						mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-R", arg[i]);
						toKick.sortAdd(nickHandle);
						number++;
					}

					else if(chset->USER_REOPS==2)
					{
						if(!local && !global)
						{
							if(!(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
							{
								toKick.sortAdd(nickHandle);
								number++;
							}

							mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-R", arg[i]);
						}
					}

					list[REOP].add(arg[i], nickHandle->nick, ((local && local->sticky) || (global && global->sticky)) ? 0 : set.BIE_MODE_BOUNCE_TIME);
					sentList[REOP].remove(arg[i]);
				}
				break; //+R

				//////////////////
				case 'i':
				if(__hasFlag(1, 'i', 0) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_HIGH].add(0, "-i");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++;
					}
				}
				addFlags("i");
				flags &= ~SENT_I;
				break; //+i

				//////////////////
				case 'n':
				if(__hasFlag(1, 'n', 0) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "-n");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}
				}
				addFlags("n");
				flags &= ~SENT_N;
				break; //+n

				//////////////////
				case 't':
				if(__hasFlag(1, 't', 0) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "-t");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}
				}
				addFlags("t");
				flags &= ~SENT_T;
				break; //+t

				//////////////////
				case 's':
				if(__hasFlag(1, 's', 0) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "-s");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}
				}
				addFlags("s");
				flags &= ~SENT_S;
				break; //+s

				//////////////////
				case 'p':
				if(__hasFlag(1, 'p', 0) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "-p");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}
				}
				addFlags("p");
				flags &= ~SENT_P;
				break; //+p

				//////////////////
				case 'm':
				if(__hasFlag(1, 'm', 0) && !(nickHandle->flags & (HAS_M | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "-m");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}
				}
				addFlags("m");
				flags &= ~SENT_M;
				break; //+m
			}
		}
		else if(mode[0][i] == '-')
		{
			switch(mode[1][i])
			{
				//////////////////
				case 'o':
				argHandle = getUser(arg[i]);
				toKick.remove(argHandle);

				/* user had @ */
				if(argHandle->flags & IS_OP)
				{
					/* is it a bot ? */
					if(argHandle->flags & HAS_B)
					{
						opedBots.remove(argHandle);
						botsToOp.sortAdd(argHandle);
						if(oped > 0) --oped;
						mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+o", arg[i]);
					}
					else if(argHandle->flags & HAS_R && userLevel(argHandle) > userLevel(nickHandle))
					{
						mqc[mq++] = modeQ[PRIO_LOW].add(0, "+o", arg[i]);
					}
				}

				argHandle->flags &= ~IS_OP;

				//punish
				if((userLevel(nickHandle) < userLevel(argHandle) || nickHandle->flags & HAS_B)
					&& *nickHandle->nick)
				{
						toKick.sortAdd(nickHandle);
						number++;
				}
				if(argHandle == me)
				{
					mypos = -1;
					p = toKick.begin();
					while(p)
					{
						p->flags &= ~(KICK_SENT | OP_SENT | VOICE_SENT);
						p++;
					}
					sentKicks = 0;
					if(opedBots.entries()) initialOp = NOW;
					else initialOp = 0;

					flags &= ~CHFLAGS_SENT_ALL;

					sentList[BAN].clear();
					sentList[INVITE].clear();
					sentList[EXEMPT].clear();
					sentList[REOP].clear();
				}
				break; //-o

				//////////////////
				case 'v':
				argHandle = getUser(arg[i]);
				argHandle->flags &= ~IS_VOICE;
				break; //-v

				//////////////////
				case 'k':
				updateKey("");
				if(__hasFlag(1, 'k', 1) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+k", __getKey());
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++;
					}
				}
				break; //-k

				//////////////////
				case 'l':
				j = limit;
				limit = 0;

				if(!(nickHandle->flags & (HAS_N | HAS_B)))
				{
					if(__hasFlag(1, 'l', 1))
					{
						mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+l", itoa(__getLimit()));	
						toKick.sortAdd(nickHandle);
						number++;
					}
					else if(__hasFlag(1, 'l', 0)); // allready got -l, so do nothing
					else 
					{
					    if(chset->LIMIT)
					    {
						mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+l", itoa(j));	
					    	toKick.sortAdd(nickHandle);
						number++;
					    }
					}
				}
				else
				{
					if(chset->LIMIT || __hasFlag(1, 'l', 1))
					{
						if(nickHandle->flags & HAS_N) nextlimit = NOW + chset->OWNER_LIMIT_TIME;
						else if(nickHandle->flags & HAS_B) nextlimit = NOW + (rand() % 5) + 1;
					}
				}
				flags &= ~SENT_MINUS_L;
				break; //-l

				//////////////////
				case 'b':
				protmodelist::entry *ban;
				list[BAN].remove(arg[i]);

				if((ban=protmodelist::findSticky(arg[i], BAN, this)))
				{
					if(!(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
					{
						nickHandle->setReason(ban->fullReason());
						toKick.sortAdd(nickHandle);
						number++;
					}
					mqc[mq++] = modeQ[PRIO_HIGH].add(0 ,"+b", arg[i]);
				}
				break; //-b

				//////////////////
				case 'e':
				if(list[EXEMPT].remove(arg[i]) && chset->USER_EXEMPTS!=0)
				{
					protmodelist::entry *global=userlist.protlist[EXEMPT]->find(arg[i]), *local=protlist[EXEMPT]->find(arg[i]);

					if(chset->USER_EXEMPTS==1 || (chset->USER_EXEMPTS==2 && (local || global)))
					{
						if(!(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
						{
							if(local && local->sticky)
								nickHandle->setReason(local->fullReason());
							else if(global && global->sticky)
								nickHandle->setReason(global->fullReason());

							toKick.sortAdd(nickHandle);
							mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+e", arg[i]);
							number++;
						}
						else if(chset->USER_EXEMPTS==2)
							mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+e", arg[i]);
					}
				}
				break; //-e

				//////////////////
				case 'I':
				if(list[INVITE].remove(arg[i]) && chset->USER_INVITES!=0)
				{
					protmodelist::entry *global=userlist.protlist[INVITE]->find(arg[i]), *local=protlist[INVITE]->find(arg[i]);

					if(chset->USER_INVITES==1 || (chset->USER_INVITES==2 && (local || global)))
					{
						if(!(nickHandle->flags&(HAS_N|HAS_B))&&*nickHandle->nick)
						{
							if(local&&local->sticky)
								nickHandle->setReason(local->fullReason());
							else if(global&&global->sticky)
								nickHandle->setReason(global->fullReason());

							toKick.sortAdd(nickHandle);
							mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+I", arg[i]);
							number++;
					}
					else if(chset->USER_INVITES==2)
						mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+I", arg[i]);
					}
				}
				break; //-I

				//////////////////
				case 'R':
				if(list[REOP].remove(arg[i]) &&	chset->USER_REOPS!=0)
				{
					protmodelist::entry *global=userlist.protlist[REOP]->find(arg[i]), *local=protlist[REOP]->find(arg[i]);

					if(chset->USER_REOPS==1 || (chset->USER_REOPS==2 && (local || global)))
					{
						if(!(nickHandle->flags & (HAS_N | HAS_B)) && *nickHandle->nick)
						{
							if(local && local->sticky)
								nickHandle->setReason(local->fullReason());
							else if(global && global->sticky)
								nickHandle->setReason(global->fullReason());

							toKick.sortAdd(nickHandle);
							mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+R", arg[i]);
							number++;
						}
						else if(chset->USER_REOPS==2)
							mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+R", arg[i]);
					}
				}
				break; //-R

				//////////////////
				case 'i':
				if(__hasFlag(1, 'i', 1) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_HIGH].add(0, "+i");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++;
					}
				}
				removeFlags("i");
				flags &= ~(SENT_MINUS_I | CRITICAL_LOCK);
				break; //-i

				//////////////////
				case 'n':
				if(__hasFlag(1, 'n', 1) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "+n");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++;
					}
				}
				removeFlags("n");
				flags &= ~SENT_MINUS_N;
				break; //-n

				//////////////////
				case 't':
				if(__hasFlag(1, 't', 1) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "+t");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}
				}
				removeFlags("t");
				flags &= ~SENT_MINUS_T;
				break; //-t

				//////////////////
				case 's':
				if(__hasFlag(1, 's', 1) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "+s");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}    
				}
				removeFlags("s");
				flags &= ~SENT_MINUS_S;
				break; //-s

				//////////////////
				case 'p':
				if(__hasFlag(1, 'p', 1) && !(nickHandle->flags & (HAS_N | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "+p");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}
				}
				removeFlags("p");
				flags &= ~SENT_MINUS_P;
				break; //-p

				//////////////////
				case 'm':
				if(__hasFlag(1, 'm', 1) && !(nickHandle->flags & (HAS_M | HAS_B)))
				{
					mqc[mq++] = modeQ[PRIO_LOW].add(0, "+m");
					if(*nickHandle->nick)
					{
					    toKick.sortAdd(nickHandle);
					    number++; 
					}
				}
				removeFlags("m");
				flags &= ~SENT_MINUS_M;
				break; //-m
			} //switch
		} //mode
	} //for


	/* channel modes protection */
	if(mq)
	{
		if(myTurn(chset->GUARDIAN_BOTS, nickHandle->hash32()))
		{
			for(i=0; i<mq; ++i)
				mqc[i]->expire = NOW;

			modeQ[PRIO_HIGH].flush(PRIO_HIGH);
		}
		else
		{
			j = ((myPos() + hash32(name)) % (opedBots.entries() + 1) + 1) * set.BACKUP_MODE_DELAY;
			for(i=0; i<mq; ++i)
			{
				mqc[i]->expire = NOW + j;
				mqc[i]->backupmode = true;
			}
		}
	}

	//defense op code
	if(toKick.entries() < 4 && me->flags & IS_OP)
	{
		if(chset->BOT_AOP_MODE == 2 && mypos == 0 && botsToOp.entries())
		{
			j = getRandomItems(multHandle, botsToOp.begin(), botsToOp.entries(), 3);
			op(multHandle, j);
		}
	}
	//takeover op
	else if(mypos != -1 && me->flags & IS_OP)
	{
		if(chset->TAKEOVER || chset->BOT_AOP_MODE)
		{
			divide((int *) &n, botsToOp.entries(), oped, set.OPS_PER_MODE);
			if(n[mypos])
			{
				if(mypos == 0) j = getRandomItems(multHandle, botsToOp.begin(), n[mypos], set.OPS_PER_MODE);
				else if(mypos == 1)	j = getRandomItems(multHandle, botsToOp.getItem(n[mypos-1]), n[mypos], set.OPS_PER_MODE);
				else j = getRandomItems(multHandle, botsToOp.getItem(n[mypos-1] + n[mypos-2]), n[mypos], set.OPS_PER_MODE);
				op(multHandle, j);
			}
		}
	}

	//big kick
	if(toKick.entries() > 6 && me->flags & IS_OP)
	{
		j = getRandomItems(multHandle, toKick.begin(), toKick.entries()-sentKicks, 6, KICK_SENT);
		kick6(multHandle, j);
	}

	//kick for sth bad
	else if(toKick.entries() && me->flags & IS_OP)
	{
		p = toKick.begin();

		i = hash = 0;
		while(p)
		{
			if(!(p->flags & KICK_SENT))
			{
				multHandle[i] = p;
				hash += p->hash32();
				if(++i == 6) break;
			}
			p++;
		}

		if(myTurn(chset->PUNISH_BOTS, hash))
		{
		    kick6(multHandle, i);
		    
		    /* idiots code */
		    if(nickHandle != &serverHandle && (int) chset->IDIOTS && number) 
		    {
			    if(userlist.isMain(userlist.me()))
		    		userlist.addIdiot(mask, (const char *) name, b, number);
	    		    else
        		    {
            			if(net.hub.fd && net.hub.isMain())
                		    net.hub.send(S_ADDIDIOT, " ", mask, " ", (const char *) name, " ", itoa(number), " ", b, NULL);
            			else
				{
                		    for(i=0; i<net.max_conns; ++i)
                			if(net.conn[i].isMain() && net.conn[i].fd)
                    			{
                        		    net.conn[i].send(S_ADDIDIOT, " ", mask, " ", (const char *) name, " ", itoa(number), " ", b, NULL);
                        		    break;
                    			}
                		}	
        		    }
		    }	
		}
	}


	HOOK(mode, mode(this, mode, const_cast<const char**> (arg)));
	stopParsing=false;

	HOOK(modeWho, modeWho(this, mode, const_cast<const char**> (arg), mask));
	stopParsing=false;

	for(i=0; i<MODES_PER_LINE; ++i)
	{
		if(mode[0][i] == '-' || mode[0][i] == '+')
		{
			modeBuf[0] = mode[0][i];
			modeBuf[1] = mode[1][i];
			modeBuf[2] = '\0';

			DEBUG(printf(">>> removeBackupModesFor(%s, %s)\n", modeBuf, arg[i]));
			modeQ[PRIO_LOW].removeBackupModesFor(modeBuf, arg[i]);
			modeQ[PRIO_HIGH].removeBackupModesFor(modeBuf, arg[i]);
		}
		if(arg[i]) free(arg[i]);
	}
	
	if((int) chset->IDIOTS && nickHandle != &serverHandle) free(b);
}
