#ifndef PSOTNIC_CTCP_H
#define PSOTNIC_CTCP_H 1

struct CTCPREPLY
{
	const char *query;
	const char *reply;
};

/*
	0 = none
	1 = psotnic
	2 = irssi (default)
 	3 = epic
	4 = lice
	5 = bitchx
	--
	6 = dzony loker
	7 = luzik
	8 = mirc
	9 = nns
*/

/*
+	%A - arch
+	%R - release
+	%O - operating system
+	%V - client version
+	%T - time
+	%* - echo
+	%I - ident
+	%N - nick
+	%H - host on irc
-	%F - full name
+	%l - idle time
*/

/* irssi */
CTCPREPLY ctcp_irssi[] = {
	{ "VERSION *",		"VERSION irssi %V - running on %O %A" },
	{ "VERSION",		"VERSION irssi %V - running on %O %A" },
	{ "CLIENTINFO *",	"CLIENTINFO PING VERSION TIME USERINFO CLIENTINFO" },
	{ "CLIENTINFO",		"CLIENTINFO PING VERSION TIME USERINFO CLIENTINFO" },
	{ "USERINFO",		"USERINFO irssi %V - running on %O %A" },
	{ "USERINFO *",		"USERINFO irssi %V - running on %O %A" },
	{ "TIME",			"TIME %T" },
	{ "TIME *",			"TIME %T" },
	{ "PING *",			"PING %*" },
	{ "PING",			"PING %*" },
	{ 0, NULL }
};

/* psotnic */
CTCPREPLY ctcp_psotnic[] = {
	{ "VERSION",		"VERSION psotnic %V - running on %O %A" },
	{ "CLIENTINFO",		"CLIENTINFO PING VERSION TIME USERINFO CLIENTINFO" },
	{ "USERINFO",		"USERINFO psotnic %V - running on %O %A" },
	{ "TIME",			"TIME %T" },
	{ "PING * *",   	"PING %*" },
 	{ 0, NULL }
};

/* epic */
CTCPREPLY ctcp_epic[] = {
	{ "CLIENTINFO",			"CLIENTINFO SED UTC ACTION DCC VERSION CLIENTINFO USERINFO ERRMSG FINGER TIME PING ECHO :Use CLIENTINFO <COMMAND> to get more specific information" },
	{ "CLIENTINFO SED",		"CLIENTINFO SED contains simple_encrypted_data" },
	{ "CLIENTINFO UTC",		"CLIENTINFO UTC substitutes the local timezone" },
	{ "CLIENTINFO ACTION",	"CLIENTINFO ACTION contains action descriptions for atmosphere" },
	{ "CLIENTINFO DCC",		"CLIENTINFO DCC requests a direct_client_connection" },
	{ "CLIENTINFO VERSION",	"CLIENTINFO VERSION shows client type, version and environment" },
	{ "CLIENTINFO USERINFO","CLIENTINFO USERINFO returns user settable information" },
	{ "CLIENTINFO ERRMSG",	"CLIENTINFO ERRMSG returns error messages" },
	{ "CLIENTINFO FINGER",	"CLIENTINFO FINGER shows real name, login name and idle time of user" },
	{ "CLIENTINFO TIME",	"CLIENTINFO TIME tells you the time on the user's host" },
	{ "CLIENTINFO PING",	"CLIENTINFO PING returns the arguments it receives" },
	{ "CLIENTINFO ECHO",	"CLIENTINFO ECHO returns the arguments it receives" },
	{ "CLIENTINFO *",		"CLIENTINFO %* is not a valid function" },
	{ "VERSION",			"VERSION ircII %V %O %R - Accept no limitations." },
	{ "USERINFO",			"USERINFO EPIC4 -- Get your laundry brighter, not just whiter!" },
	{ "ERRMSG *",			"ERRMSG %*" },
	{ "ERRMSG",			    "ERRMSG %*" },
	{ "FINGER *",				"FINGER %F (%I@%H) Idle %l seconds" },
	{ "FINGER",				"FINGER %F (%I@%H) Idle %l seconds" },
	{ "TIME",				"TIME %T" },
	{ "TIME *", 	    	"TIME %T" },
	{ "PING *",	    		"PING %*" },
	{ "PING",   	    	"PING %*" },
	{ "ECHO *",				"ECHO %*" },
 	{ "ECHO",				"ECHO %*" },
	{ 0, NULL }
};

/* lice */
CTCPREPLY ctcp_lice[] = {
	{ "CLIENTINFO",			"CLIENTINFO SED UTC ACTION DCC VERSION CLIENTINFO USERINFO ERRMSG FINGER TIME PING ECHO :Use CLIENTINFO <COMMAND> to get more specific information" },
	{ "CLIENTINFO SED",		"CLIENTINFO SED contains simple_encrypted_data" },
	{ "CLIENTINFO UTC",		"CLIENTINFO UTC substitutes the local timezone" },
	{ "CLIENTINFO ACTION",	"CLIENTINFO ACTION contains action descriptions for atmosphere" },
	{ "CLIENTINFO DCC",		"CLIENTINFO DCC requests a direct_client_connection" },
	{ "CLIENTINFO VERSION",	"CLIENTINFO VERSION shows client type, version and environment" },
	{ "CLIENTINFO USERINFO","CLIENTINFO USERINFO returns user settable information" },
	{ "CLIENTINFO ERRMSG",	"CLIENTINFO ERRMSG returns error messages" },
	{ "CLIENTINFO FINGER",	"CLIENTINFO FINGER shows real name, login name and idle time of user" },
	{ "CLIENTINFO TIME",	"CLIENTINFO TIME tells you the time on the user's host" },
	{ "CLIENTINFO PING",	"CLIENTINFO PING returns the arguments it receives" },
	{ "CLIENTINFO ECHO",	"CLIENTINFO ECHO returns the arguments it receives" },
	{ "CLIENTINFO *",		"CLIENTINFO %* is not a valid function" },
	{ "VERSION",			"VERSION ircII %V %O %R - \002LiCe\002 v4.2.0pre7" },
	{ "USERINFO",			"USERINFO \002LiCe\002 - %F" },
	{ "ERRMSG *",			"ERRMSG %*" },
	{ "ERRMSG",			    "ERRMSG %*" },
	{ "FINGER *",			"FINGER %F (%I@%H) Idle %l seconds" },
	{ "FINGER",				"FINGER %F (%I@%H) Idle %l seconds" },
	{ "TIME",				"TIME %T" },
	{ "TIME *",				"TIME %T" },
	{ "PING *",				"PING %*" },
	{ "PING",				"PING %*" },
	{ "ECHO *",				"ECHO %*" },
 	{ "ECHO",				"ECHO %*" },
	{ 0, NULL }
};


/* bitchx */
CTCPREPLY ctcp_bitchx[] = {
	{ "SED",					"SED [ENCRYPTED MESSAGE]" },
	{ "UTC",					"UTC Thu Jan 1 01:00:00 1970" },
	{ "VERSION",				"VERSION \002%V\002 by panasync - %O %R : \002Keep it to yourself!\002" },
	{ "CLIENTINFO",				"CLIENTINFO SED UTC ACTION DCC CDCC BDCC XDCC VERSION CLIENTINFO USERINFO ERRMSG FINGER TIME PING ECHO INVITE WHOAMI OP OPS UNBAN IDENT XLINK UPTIME  :Use CLIENTINFO <COMMAND> to get more specific information" },
	{ "CLIENTINFO SED",			"CLIENTINFO SED contains simple_encrypted_data" },
	{ "CLIENTINFO UTC",			"CLIENTINFO UTC substitutes the local timezone" },
	{ "CLIENTINFO ACTION",		"CLIENTINFO ACTION contains action descriptions for atmosphere" },
	{ "CLIENTINFO DCC",			"CLIENTINFO DCC requests a direct_client_connection" },
	{ "CLIENTINFO CDCC",		"CLIENTINFO CDCC checks cdcc info for you" },
	{ "CLIENTINFO VERSION",		"CLIENTINFO VERSION shows client type, version and environment" },
	{ "CLIENTINFO USERINFO",	"CLIENTINFO USERINFO returns user settable information" },
	{ "CLIENTINFO ERRMSG",		"CLIENTINFO ERRMSG returns error messages" },
	{ "CLIENTINFO FINGER",		"CLIENTINFO FINGER shows real name, login name and idle time of user" },
	{ "CLIENTINFO TIME",		"CLIENTINFO TIME tells you the time on the user's host" },
	{ "CLIENTINFO IDENT",		"CLIENTINFO IDENT change userhost of userlist" },
	{ "CLIENTINFO PING",		"CLIENTINFO PING returns the arguments it receives" },
	{ "CLIENTINFO ECHO",		"CLIENTINFO ECHO returns the arguments it receives" },
	{ "CLIENTINFO INVITE",		"CLIENTINFO INVITE invite to channel specified" },
	{ "CLIENTINFO WHOAMI",		"CLIENTINFO WHOAMI user list information" },
	{ "CLIENTINFO OP",			"CLIENTINFO OP ops the person if on userlist" },
	{ "CLIENTINFO OPS",			"CLIENTINFO OPS ops the person if on userlist" },
	{ "CLIENTINFO UNBAN",		"CLIENTINFO UNBAN unbans the person from channel" },
	{ "CLIENTINFO IDENT",		"CLIENTINFO IDENT change userhost of userlist" },
	{ "CLIENTINFO CLIENTINFO",	"CLIENTINFO CLIENTINFO gives information about available CTCP commands" },
	{ "CLIENTINFO XLINK",		"CLIENTINFO XLINK x-filez rule" },
	{ "CLIENTINFO UPTIME",		"CLIENTINFO UPTIME my uptime" },
	{ "CLIENTINFO *",			"CLIENTINFO %* is not a valid function" },
	{ "USERINFO",				"USERINFO" },
	{ "ERRMSG *",				"ERRMSG %*" },
	{ "ERRMSG",					"ERRMSG %*" },
	{ "FINGER *",				"FINGER %F (%I@%H) Idle %l seconds" },
	{ "FINGER",					"FINGER %F (%I@%H) Idle %l seconds" },
	{ "WHOAMI",					"WHOAMI \002BitchX\002: Access Denied" },
	{ "TIME",					"TIME %T" },
	{ "TIME *",					"TIME %T" },
	{ "PING *",					"PING %*" },
	{ "PING",					"PING %*" },
	{ "ECHO *",					"ECHO %*" },
	{ "ECHO",					"ECHO %*" },
	{ "INVITE",					"INVITE \002BitchX\002: Access Denied" },
	{ "OP",						"OP \002BitchX\002: Access Denied" },
	{ "OPS",					"OPS \002BitchX\002: Access Denied" },
	{ "UNBAN ",					"UNBAN \002BitchX\002: Access Denied" },
	{ 0, NULL }
};

/* dzony loker */
CTCPREPLY ctcp_dzony[] = {
	{ "VERSION",                "VERSION \002••Ðzony Lokér® 5.0••\002 by MarYOUsh® [l33t ScRIPt]" },
	{ "CLIENTINFO PING",        "CLIENTINFO PING returns the arguments it receives" },
	{ "CLIENTINFO VERSION",     "CLIENTINFO VERSION shows client type, version and environment" },
	{ "CLIENTINFO TIME",        "CLIENTINFO TIME tells you the time on the user's host" },
	{ "CLIENTINFO USERINFO",    "CLIENTINFO USERINFO returns user settable information" },
	{ "CLIENTINFO FINGER",      "CLIENTINFO FINGER shows real name, login name and idle time of user" },
	{ "CLIENTINFO CLIENTINFO",  "CLIENTINFO CLIENTINFO gives information about available CTCP commands" },
	{ "CLIENTINFO",             "CLIENTINFO FINGER PING VERSION TIME USERINFO CLIENTINFO" },
	{ "CLIENTINFO *",			"CLIENTINFO %* is not a valid function" },
	{ "ERRMSG *",				"ERRMSG %*" },
	{ "ERRMSG",					"ERRMSG %*" },
	{ "FINGER",					"FINGER %F (%I@%H) Idle %l seconds" },
	{ "TIME",					"TIME %T" },
	{ "TIME *",					"TIME %T" },
	{ "PING *",					"PING %*" },
	{ "PING",					"PING %*" },
	{ "ECHO *",					"ECHO %*" },
	{ "ECHO",					"ECHO %*" },
	{ 0, NULL }
};


/* luzik */
CTCPREPLY ctcp_luzik[] = {
	{ "VERSION",                "VERSION Luzik v3.0 by TUX z http://www.tux.com.pl/" },
	{ "CLIENTINFO PING",        "CLIENTINFO PING returns the arguments it receives" },
	{ "CLIENTINFO VERSION",     "CLIENTINFO VERSION shows client type, version and environment" },
	{ "CLIENTINFO TIME",        "CLIENTINFO TIME tells you the time on the user's host" },
	{ "CLIENTINFO USERINFO",    "CLIENTINFO USERINFO returns user settable information" },
	{ "CLIENTINFO FINGER",      "CLIENTINFO FINGER shows real name, login name and idle time of user" },
	{ "CLIENTINFO CLIENTINFO",  "CLIENTINFO CLIENTINFO gives information about available CTCP commands" },
	{ "CLIENTINFO",             "CLIENTINFO FINGER PING VERSION TIME USERINFO CLIENTINFO" },
	{ "CLIENTINFO *",			"CLIENTINFO %* is not a valid function" },
	{ "ERRMSG *",				"ERRMSG %*" },
	{ "ERRMSG",					"ERRMSG %*" },
	{ "FINGER *",				"FINGER %F (%I@%H) Idle %l seconds" },
	{ "FINGER",					"FINGER %F (%I@%H) Idle %l seconds" },
	{ "TIME",					"TIME %T" },
	{ "TIME *",					"TIME %T" },
	{ "PING *",					"PING %*" },
	{ "PING",					"PING %*" },
	{ "ECHO *",					"ECHO %*" },
	{ "ECHO",					"ECHO %*" },
	{ 0, NULL }
};

/* mirc */
CTCPREPLY ctcp_mirc[] = {
	{ "VERSION",				"VERSION mIRC v6.2 Khaled Mardam-Bey" },
	{ "CLIENTINFO PING",		"CLIENTINFO PING returns the arguments it receives" },
	{ "CLIENTINFO VERSION",     "CLIENTINFO VERSION shows client type, version and environment" },
	{ "CLIENTINFO TIME",		"CLIENTINFO TIME tells you the time on the user's host" },
	{ "CLIENTINFO USERINFO",	"CLIENTINFO USERINFO returns user settable information" },
	{ "CLIENTINFO FINGER",      "CLIENTINFO FINGER shows real name, login name and idle time of user" },
	{ "CLIENTINFO CLIENTINFO",  "CLIENTINFO CLIENTINFO gives information about available CTCP commands" },
	{ "CLIENTINFO",             "CLIENTINFO FINGER PING VERSION TIME USERINFO CLIENTINFO" },
	{ "CLIENTINFO *",			"CLIENTINFO %* is not a valid function" },
	{ "ERRMSG *",				"ERRMSG %*" },
	{ "ERRMSG",					"ERRMSG %*" },
	{ "FINGER *",				"FINGER %F (%I@%H) Idle %l seconds" },
	{ "FINGER",					"FINGER %F (%I@%H) Idle %l seconds" },
	{ "TIME",					"TIME %T" },
	{ "TIME *",					"TIME %T" },
	{ "PING *",					"PING %*" },
	{ "PING",					"PING %*" },
	{ "ECHO *",					"ECHO %*" },
	{ "ECHO",					"ECHO %*" },
	{ 0, NULL }
};
#endif
