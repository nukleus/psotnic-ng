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

#ifdef HAVE_TCL

int tcl_timers(void *foo, Tcl_Interp *interp, int argc, CONST char *argv[])
{
	char *buf = NULL;
	char *a;
	int i = 0;

	ptrlist<tcl::timer_t>::iterator p = tclparser.timer.begin();

	while(p)
	{
		a = strchr(p->nick, ' ');

		buf = push(buf, (char *) (i ? " " : ""), "{", itoa(abs(p->exp - NOW) / *((int *) foo)), NULL);

		if(a) buf = push(buf, " {", p->nick, "} timer", itoa(p->id), NULL);
		else buf = push(buf, " ", p->nick, " timer", itoa(p->id), NULL);

		p++;
		++i;
	}
	DEBUG(printf("### timers = %s\b", buf));
	Tcl_SetResult(tclparser.getInt(), buf, TCL_VOLATILE);
	free(buf);
	return TCL_OK;
}

int tcl_timer(void *foo, Tcl_Interp *interp, int argc, CONST char *argv[])
{
	if(argc < 3) return TCL_ERROR;

	tcl::timer_t *t = new tcl::timer_t;
	int i, len = 1;

	t->exp = NOW + (atoi(argv[1]) * *((int *) foo));
	t->id = ++tclparser.curid;

	for(i=2; i<argc; ++i) len += strlen(argv[i]);

	t->nick = (char *) malloc(len);

	strcpy(t->nick, argv[2]);
	for(i=3; i<argc; ++i)
	{
        strcat(t->nick, " ");
		strcat(t->nick, argv[i]);
	}

	printf("### timer %d %s\n", atoi(argv[1]) * *((int *) foo), t->nick);
	tclparser.timer.add(t);
	return TCL_OK;
}

int tcl_putserv(void *foo, Tcl_Interp *interp, int argc, CONST char *argv[])
{
	int len = 1, i;

	if(argc == 1) return TCL_ERROR;

	for(i=1; i<argc; ++i) len += strlen(argv[1]);

	char *buf = (char *) malloc(len);
	strcpy(buf, argv[1]);
	for(i=2; i<argc; ++i)
	{
		strcat(buf, " ");
		strcat(buf, argv[i]);
	}
	net.irc.send(buf, NULL);
	free(buf);
	return TCL_OK;
}

int tcl_channels(void *foo, Tcl_Interp *interp, int argc, CONST char *argv[])
{
	int len = 1;
	chan *ch = ME.first;

	if(argc != 1) return TCL_ERROR;

	while(ch)
	{
		len += strlen(ch->name) + 1;
		ch = ch->next;
	}

	char *buf = (char *) malloc(len);

	ch = ME.first ;

	while(ch)
	{
		if(ch == ME.first) strcpy(buf, ch->name);
		else
		{
			strcat(buf, " ");
			strcat(buf, ch->name);
		}
		ch = ch->next;
	}
	DEBUG(printf("### channels = %s\n", buf));
	Tcl_SetResult(tclparser.getInt(), buf, TCL_VOLATILE);
	free(buf);
	return TCL_OK;
}

int tcl_rand(void *foo, Tcl_Interp *interp, int argc, CONST char *argv[])
{
	if(argc == 2)
	{
		char buf[16];
		sprintf(buf, "%d", rand () % atoi(argv[1]));
		DEBUG(printf("### rand %d = %s\n", atoi(argv[1]), buf));
		Tcl_SetResult(tclparser.getInt(), buf, TCL_VOLATILE);
        	return TCL_OK;
	}
	return TCL_ERROR;
}

int tcl_botnick(void *foo, Tcl_Interp *interp, int argc, CONST char *argv[])
{
	const char *tmp=ME.nick;

	if(argc != 1) return TCL_ERROR;
	DEBUG(printf("### botnick = %s\n", (const char*)ME.nick));
	Tcl_SetResult(tclparser.getInt(), (char *)tmp, TCL_VOLATILE);
	return TCL_OK;
}
int tcl_ctcptype(void *foo, Tcl_Interp *interp, int argc, CONST char *argv[])
{
	char buf[16];

	switch(config.ctcptype)
	{
		case 0: strcpy(buf, ""); break;
		case 1: strcpy(buf, "psotnic"); break;
		case 2: strcpy(buf, "irssi"); break;
		case 3: strcpy(buf, "epic"); break;
		case 4: strcpy(buf, "lice"); break;
		case 5: strcpy(buf, "bitchx"); break;
		default: strcpy(buf, ""); break;
	}

	Tcl_SetResult(tclparser.getInt(), buf, TCL_VOLATILE);
	return TCL_OK;
}

int tcl_commands(void *foo, Tcl_Interp *interp, int argc, CONST char *argv[])
{
	Tcl_SetResult(tclparser.getInt(), "ctcptype botnick rand channel putserv timer utimer timers utimers commands", TCL_VOLATILE);
	return TCL_OK;
}

void tcl::addCommands()
{
	int m = 60, s = 1;;

	Tcl_CreateCommand(tcl_int, "botnick", tcl_botnick, NULL, NULL);
	Tcl_CreateCommand(tcl_int, "rand", tcl_rand, NULL, NULL);
	Tcl_CreateCommand(tcl_int, "channels", tcl_channels, NULL, NULL);
	Tcl_CreateCommand(tcl_int, "putserv", tcl_putserv, NULL, NULL);
	Tcl_CreateCommand(tcl_int, "timer", tcl_timer, &m, NULL);
	Tcl_CreateCommand(tcl_int, "utimer", tcl_timer, &s, NULL);
	Tcl_CreateCommand(tcl_int, "timers", tcl_timers, &m, NULL);
	Tcl_CreateCommand(tcl_int, "utimers", tcl_timers, &s, NULL);
	Tcl_CreateCommand(tcl_int, "ctcptype", tcl_ctcptype, NULL, NULL);
	Tcl_CreateCommand(tcl_int, "commands", tcl_commands, NULL, NULL);
}

char *tcl::setGlobalVar(char *name, char *value)
{
	return (char *) Tcl_SetVar(tcl_int, name, value, TCL_GLOBAL_ONLY);
}

int tcl::eval(char *cmd)
{
	return Tcl_Eval(tcl_int, cmd) == TCL_OK;
}


tcl::tcl()
{
	tcl_int = Tcl_CreateInterp();
	curid = 0;

	Tcl_Init(tcl_int);
	Tcl_SetVar(tcl_int, "argv0", "psotnic", TCL_GLOBAL_ONLY);
	addCommands();
}

tcl::~tcl()
{
	Tcl_DeleteInterp(tcl_int);
	tcl_int = NULL;
	curid = 0;
}

int tcl::load(char *script)
{
	int status = Tcl_EvalFile(tcl_int, script);
	if(status == TCL_ERROR)	return 0;
	return 1;
}

Tcl_Interp *tcl::getInt()
{
	return tcl_int;
}

void tcl::expireTimers()
{
	ptrlist<tcl::timer_t>::iterator p = timer.begin(), q;

	while(p)
	{
		q=p;
		q++;

		if(p->exp <= NOW)
		{
			printf("### expireTimers: eval(\"%s\")\n", p->nick);
			printf(">>> %s\n", eval(p->nick) == TCL_OK ? "TCL_OK" : "TCL_ERROR");

			free(p->nick);

			timer.removeLink(p);
		}
		p=q;
	}
}

#endif
