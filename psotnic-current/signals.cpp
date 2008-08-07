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

void sigChild()
{
	int status;
	while(waitpid(-1, &status, WNOHANG) > 0) ;
}

void sigTerm()
{
	signal(SIGTERM, SIG_IGN);
	if(net.irc.fd > 0) net.irc.send("QUIT :", (const char *) config.quitreason, NULL);
	net.send(HAS_N, "[!] Got termination signal", NULL);
	stopPsotnic = 1;
}

void sigInt()
{
	signal(SIGINT, SIG_IGN);
	if(net.irc.fd > 0) net.irc.send("QUIT :", (const char *) config.quitreason, NULL);
	net.send(HAS_N, "[!] Terminated by user", NULL);
	stopPsotnic = 1;
}

void sigHup()
{
	userlist.save(config.userlist_file);
	HOOK(receivedSigHup, receivedSigHup());
}

void sigSegv()
{
	signal(SIGSEGV, SIG_IGN);
	
	net.send(HAS_N, "[!] Got segmentation fault signal; please report this crash", NULL);
	net.irc.send("QUIT :Got segmentation fault signal; please report this crash", NULL);
	
#ifdef HAVE_DEBUG
	char gdb[MAX_LEN], cmdlist[256], btfile[256];
	int bt = 0;
	unsigned int core = 0;

	snprintf(cmdlist, 256, "/tmp/.psotnic-%d", (int) getpid());
	FILE *f = fopen(cmdlist, "w");

	if(f)
	{
		fprintf(f, "attach %d\n", getpid());
		fprintf(f, "bt 100\n");
		fprintf(f, "detach\n");
		fprintf(f, "q\n");
		fclose(f);

		snprintf(btfile, 256, ".gdb-backtrace-%d", (int) getpid());
		snprintf(gdb, MAX_LEN, "gdb -q -x %s > %s 2>&1", cmdlist, btfile);
		if(!system(gdb))
			bt = 1;
		unlink(cmdlist);
	}

	//enabling core dumps
	struct rlimit limit;
	if(!getrlimit(RLIMIT_CORE, &limit))
	{
		limit.rlim_cur = limit.rlim_max;
		if(!setrlimit(RLIMIT_CORE, &limit))
			core = limit.rlim_cur;
	}

#endif
	dumpIrcBacktrace();
}

void safeExit()
{
	net.send(HAS_N, "[*] Abnormal program termination", NULL);
	stopPsotnic = 1;
}

void sigCpuTime()
{
	net.send(HAS_N, "[*] Cpu time limit reached, terminating", NULL);
	if(net.irc.fd > 0) net.irc.send("QUIT :Cpu time limit reached, terminating", NULL);	
	stopPsotnic = 1;
}

void sigUpdateFailed()
{
	DEBUG(printf("[D] got: SIGUSR2\n"));
	psotget.end();
}

void sigUpdated()
{
	DEBUG(printf("[D] got: SIGUSR1\n"));
	psotget.end();
#ifdef RESTART_AFTER_UPDATE
	ME.restart();
#endif
}

#define register_signal(x, y) \
{	\
	memset(&sv, 0, sizeof(sv)); \
	sv.sa_handler = y;	\
	sigaction(x, &sv, NULL);	\
}

void signalHandling()
{
	struct sigaction sv;

	register_signal(SIGPIPE, SIG_IGN);
	register_signal(SIGCHLD, (sighandler_t) sigChild);
 	register_signal(SIGTERM, (sighandler_t) sigTerm);
	register_signal(SIGINT, (sighandler_t) sigInt);
	register_signal(SIGHUP, (sighandler_t) sigHup);
	register_signal(SIGUSR1, (sighandler_t) sigUpdated);
	register_signal(SIGUSR2, (sighandler_t) sigUpdateFailed);
#ifndef HAVE_DEBUG
	register_signal(SIGSEGV, (sighandler_t) sigSegv);
#endif
	register_signal(SIGXCPU, (sighandler_t) sigCpuTime);
}
