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

bool CONFIG::load(const char *file, bool decrypted)
{
	char arg[10][MAX_LEN], buf[MAX_LEN];
	int line = 0, n;
	int errors = 0;
	int alts = 0;
	options::event *e;
	inetconn f;

	config.file = file;

	if(creation || decrypted)
		printf("[*] Loading decrypted config from '%s'\n", file);

	if(f.open(file, O_RDWR) < 1)
	{
		printf("[-] Cannot open config file: %s\n", strerror(errno));
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
			printf("[-] %s:%d: %s\n", file, line, (const char *) e->reason);
			++errors;
		}
	}

	/* Basic checks */
	if(!nick.getLen())
	{
		printf("[-] Nick is not set\n");
		++errors;
	}

	if(errors)
	{
		plonk:
		printf("[-] Failed to load config\n");
		exit(1);
	}

	polish();

	if(creation && !decrypted)
	{
		printf("[*] Crypting config file\n");

		// make a backup
		snprintf(buf, MAX_LEN, "%s.dec", (const char *) config.file);
		unlink(buf);

		if(rename(config.file, buf))
		{
			printf("[-] cannot create backup file '%s': %s\n", buf, strerror(errno));
			exit(1);
		}

		e = config.save();

		if(!e || !e->ok)
		{
			printf("[-] Cannot save config file: %s\n", (const char *) e->reason);
			exit(1);
		}

		printf("[+] All done\n");
		printf("[*] Please move %s.dec to safe place and start bot without -c option\n", file);
		printf("[+] Terminating.\n");
		exit(0);
	}

	if(imUp())
	{
		printf("[-] Signs on the sky tell me that i am already up\n");
		printf("[*] Terminating\n");
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
			printf("[*] Acting as SLAVE\n");
			config.bottype = BOT_SLAVE;
		}
		else
		{
			printf("[*] Acting as MAIN\n");
			config.bottype = BOT_MAIN;
		}
	}
	else
	{
		printf("[*] Acting as LEAF\n");
		config.bottype = BOT_LEAF;
	}

	if(config.bottype != BOT_LEAF)
	{
		if(alts)
		{
			printf("[-] Alternating slaves can only be used for leafes\n");
			goto plonk;
		}
	}
	else
	{
		if(!*config.hub.getHandle() && alts)
		{
			printf("[-] Invalid hub's handle\n");
			goto plonk;
		}
	}
	printf("[+] Config loaded\n");

	if(config.listenport)
	{
		printf("[*] Opening listening socket at %s:%d\n", (const char *) config.myipv4, (int) config.listenport);
		if((net.listenfd = startListening(config.myipv4, config.listenport)) < 1)
		{
			printf("[-] Cannot open socket (%s)\n", strerror(errno));
			exit(1);
		}
		printf("[+] Socket awaits incomming connections\n");
	}

#ifdef HAVE_SSL
	if(config.ssl_listenport)
	{
		printf("[*] Creating SSL context\n");
		
		if(!(inet::server_ctx = SSL_CTX_new(SSLv23_method())) || !SSL_CTX_set_mode(inet::server_ctx, SSL_MODE_ENABLE_PARTIAL_WRITE))
		{
			printf("[-] Creation of SSL conext failed\n");
			exit(1);
		}

		printf("[*] Loading RSA private key from `server.key'\n");
		if(!SSL_CTX_use_RSAPrivateKey_file(inet::server_ctx, "server.key", SSL_FILETYPE_PEM))
		{
			printf("[-] Error while loading key\n");
			exit(1);
		}
		
		printf("[*] Loading server certificate from `server.cert'\n");
		if(!SSL_CTX_use_certificate_file(inet::server_ctx, "server.crt", SSL_FILETYPE_PEM))
		{
			printf("[-] Error while loading cert\n");
			exit(1);
		}
		
		printf("[*] Opening listening SSL socket at %s:%d\n", (const char *) config.myipv4, (int) config.ssl_listenport);
		if((net.ssl_listenfd = startListening(config.myipv4, config.ssl_listenport)) < 1)
		{
			printf("[-] Cannot open socket (%s)\n", strerror(errno));
			exit(1);
		}
		printf("[+] Socket awaits incomming SSL connections\n");

	}
#endif

	return 1;
}
