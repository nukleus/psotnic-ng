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

#include <errno.h>
#include <fcntl.h>

#include "prots.h"
#include "global-var.h"
#include "functions.hpp"

void CONFIG::load(const char *file, bool decrypted)
{
	char arg[10][MAX_LEN], buf[MAX_LEN];
	int line = 0, n;
	int errors = 0;
	int alts = 0;
	options::event *e;
	inetconn f;

	config.file = file;

	if(creation || decrypted)
		printMessage("Loading decrypted config from '%s'", file);

	if(f.open(file, O_RDWR) < 1)
	{
		printError("Cannot open config file: %s", strerror(errno));
		exit(1);
	}

	if(!creation && !decrypted)
		f.enableLameCrypt();

	while(1)
	{
		n = f.readln(buf, MAX_LEN);
		if(n == -1) break;
		if(!n) continue;

		++line;

		str2words(arg[0], buf, 10, MAX_LEN);
		if(arg[0][0] == '#' || !*arg[0]) continue;

		e = setVariable(arg[0], rtrim(srewind(buf, 1)));
		if(!e->ok)
		{
			printBad("%s:%d: %s", file, line, (const char *) e->reason);
			++errors;
		}
	}

	/* Basic checks */
	if(!nick.getLen())
	{
		printBad("Nick is not set");
		++errors;
	}

	if(errors)
	{
		plonk:
		printBad("Failed to load config");
		exit(1);
	}

	polish();

	if(creation && !decrypted)
	{
		printMessage("Crypting config file");

		// make a backup
		snprintf(buf, MAX_LEN, "%s.dec", (const char *) config.file);
		unlink(buf);

		if(rename(config.file, buf))
		{
			printBad("Cannot create backup file '%s': %s", buf, strerror(errno));
			exit(1);
		}

		e = config.save();

		if(!e || !e->ok)
		{
			printBad("Cannot save config file: %s", (const char *) e->reason);
			exit(1);
		}

		printSuccess("All done");
		printSuccess("Please move %s.dec to safe place and start bot without -c option", file);
		printSuccess("Terminating.");
		exit(0);
	}

	if(imUp())
	{
		printBad("Signs on the sky tell me that i am already up");
		printMessage("Terminating");
		exit(1337);
	}

	if(config.listenport
#ifdef HAVE_SSL
		|| config.ssl_listenport
#endif
	)
	{
		if(config.hub.getPort())
		{
			printMessage("Acting as SLAVE");
			config.bottype = BOT_SLAVE;
		}
		else
		{
			printMessage("Acting as MAIN");
			config.bottype = BOT_MAIN;
		}
	}
	else
	{
		printMessage("Acting as LEAF");
		config.bottype = BOT_LEAF;
	}

	if(config.bottype != BOT_LEAF)
	{
		if(alts)
		{
			printBad("Alternating slaves can only be used for leafes");
			goto plonk;
		}
	}
	else
	{
		if(!*config.hub.getHandle() && alts)
		{
			printBad("Invalid hub's handle");
			goto plonk;
		}
	}
	printSuccess("Config loaded");

	if(config.listenport)
	{
		printMessage("Opening listening socket at %s:%d", (const char *) config.myipv4, (int) config.listenport);
		if((net.listenfd = startListening(config.myipv4, config.listenport)) < 1)
		{
			printBad("Cannot open socket (%s)", strerror(errno));
			exit(1);
		}
		printSuccess("Socket awaits incomming connections");
	}

#ifdef HAVE_SSL
	if(config.ssl_listenport)
	{
		printMessage("Creating SSL context");
		
		if(!(inet::server_ctx = SSL_CTX_new(SSLv23_method())) || !SSL_CTX_set_mode(inet::server_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE))
		{
			printBad("Creation of SSL conext failed");
			exit(1);
		}

		printMessage("Loading RSA private key from `server.key'");
		if(!SSL_CTX_use_RSAPrivateKey_file(inet::server_ctx, "server.key", SSL_FILETYPE_PEM))
		{
			printBad("Error while loading key");
			exit(1);
		}
		
		printMessage("Loading server certificate from `server.cert'");
		if(!SSL_CTX_use_certificate_file(inet::server_ctx, "server.crt", SSL_FILETYPE_PEM))
		{
			printBad("Error while loading cert");
			exit(1);
		}
		
		printMessage("Opening listening SSL socket at %s:%d", (const char *) config.myipv4, (int) config.ssl_listenport);
		if((net.ssl_listenfd = startListening(config.myipv4, config.ssl_listenport)) < 1)
		{
			printBad("Cannot open socket (%s)", strerror(errno));
			exit(1);
		}
		printSuccess("Socket awaits incomming SSL connections");

	}
#endif
}
