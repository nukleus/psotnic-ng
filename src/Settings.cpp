

#include "Settings.hpp"

settings::settings()
{
	registerObject(CYCLE_DELAY = entTime("cycle-delay", 0, 60, 10));
	registerObject(REJOIN_DELAY = entTime("rejoin-delay", 0, 60, 0));
	registerObject(REJOIN_FAIL_DELAY = entTime("rejoin-fail-delay", 7, 60, 25));
	registerObject(HUB_CONN_DELAY = entTime("hub-conn-delay", 10, 3600, 20));
	registerObject(IRC_CONN_DELAY = entTime("irc-conn-delay", 10, 3600, 25));
	registerObject(AUTH_TIME = entTime("auth-time", 15, 360, 45));
	registerObject(PRIVATE_CTCP = entBool("private-ctcp", 1));
	registerObject(OPS_PER_MODE = entInt("ops-per-mode", 0, 3, 2));
	registerObject(ASK_FOR_OP_DELAY = entTime("ask-for-op-delay", 1, 60, 4));
	registerObject(GETOP_OP_CHECK = entBool("get-op-check", 1));
	registerObject(CONN_TIMEOUT = entTime("conn-timeout", 16, 600, 180));
	registerObject(KEEP_NICK_CHECK_DELAY = entTime("keep-nick-check-delay", 5, 24*3600, 10));
	registerObject(REMEMBER_OLD_KEYS = entBool("remember-old-keys", 0));
	registerObject(TELNET_OWNERS = entInt("telnet-owners", 0, 2, 1));
	registerObject(MAX_MATCHES = entInt("max-matches", 0, MAX_INT, 20));
	registerObject(PERIP_MAX_SHOWN_CONNS = entInt("perip-max-shown-conns", 0, MAX_INT, 6));
	registerObject(PERIP_BURST_SIZE = entInt("perip-burst-size", 1, MAX_INT, 30));
	registerObject(PERIP_BURST_TIME = entTime("perip-burst-time", 0, MAX_INT, 60));
	registerObject(PERIP_IGNORE_TIME = entTime("perip-ignore-time", 0, MAX_INT, 300));
	registerObject(SYNFLOOD_MAX_CONNS = entInt("synflood-max-conns", 1, MAX_INT, 100));
	registerObject(SYNFLOOD_IGNORE_TIME = entTime("synflood-ignore-time", 0, MAX_INT, 300));
	registerObject(BIE_MODE_BOUNCE_TIME = entTime("bIe-mode-bounce-time", 0, MAX_INT, 3600));
	registerObject(WASOP_CACHE_TIME = entTime("wasop-cache-time", 0, MAX_INT, 3000));
	registerObject(AWAY_TIME = entTime("away-time", 60, MAX_INT, 8*3600));
	registerObject(CHAT_TIME = entTime("chat-time", 60, MAX_INT, 6*3600));
	registerObject(BETWEEN_MSG_DELAY = entTime("between-msg-delay", 10, MAX_INT, 5*60));
	registerObject(RANDOMNESS = entPerc("randomness", -100, 0, -50));
	registerObject(PUBLIC_AWAY = entBool("public-away", 0));
	registerObject(IDENT_CLONES = entInt("ident-clones", 0, MAX_INT, 3));
	registerObject(HOST_CLONES = entInt("host-clones", 0, MAX_INT, 3));
	registerObject(PROXY_CLONES = entInt("proxy-clones", 0, MAX_INT, 4));
	//FIXME: clone-life-time == 0
	registerObject(CLONE_LIFE_TIME = entTime("clone-life-time", 0, MAX_INT, 0));
	registerObject(CRITICAL_BOTS = entInt("critical-bots", 0, MAX_INT, 0));
	registerObject(QUARANTINE_TIME = entTime("quarantine-time", 0, 120, 15));
	registerObject(BACKUP_MODE_DELAY = entTime("backup-mode-delay", 0, 60, 7));
	registerObject(DONT_TRUST_OPS = entInt("dont-trust-ops", 0, 2, 0));
	registerObject(SERVER_LIMIT_TIME = entTime("server-limit-time", 0, MAX_INT, 5));
	registerObject(BOTS_CAN_ADD_SHIT = entBool("bots-can-add-shit", 0));
}

