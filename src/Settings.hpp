#ifndef SETTINGS_HPP
#define SETTINGS_HPP 

#include "class-ent.h"

class settings : public options
{
	public:
	entBool PRIVATE_CTCP;
	entTime CYCLE_DELAY;
	entTime REJOIN_DELAY;
	entTime REJOIN_FAIL_DELAY;
	entTime HUB_CONN_DELAY;
	entTime IRC_CONN_DELAY;
	entTime AUTH_TIME;
	entInt OPS_PER_MODE;
	entTime ASK_FOR_OP_DELAY;
	entTime CONN_TIMEOUT;
	entTime KEEP_NICK_CHECK_DELAY;
	entBool SAVE_NICK;
	entBool REMEMBER_OLD_KEYS;
	entInt TELNET_OWNERS;
	entBool GETOP_OP_CHECK;
	entInt MAX_MATCHES;
	entInt PERIP_MAX_SHOWN_CONNS;
	entInt PERIP_BURST_TIME;
	entInt PERIP_BURST_SIZE;
	entTime PERIP_IGNORE_TIME;
	entInt SYNFLOOD_MAX_CONNS;
	entTime SYNFLOOD_IGNORE_TIME;
	entTime BIE_MODE_BOUNCE_TIME;
	entTime WASOP_CACHE_TIME;
	entTime CHAT_TIME;
	entTime AWAY_TIME;
	entPerc RANDOMNESS;
	entTime BETWEEN_MSG_DELAY;
	entBool PUBLIC_AWAY;
	entTime CLONE_LIFE_TIME;
	entInt HOST_CLONES;
	entInt IDENT_CLONES;
	entInt PROXY_CLONES;
	entInt CRITICAL_BOTS;
	entTime QUARANTINE_TIME;
	entTime BACKUP_MODE_DELAY;
	entInt DONT_TRUST_OPS;
	entTime SERVER_LIMIT_TIME;
	entBool PRE_023_FINAL_COMPAT;
	entBool PRE_0211_FINAL_COMPAT;
	entBool PRE_0214_FINAL_COMPAT;
	entBool PRE_REV127_COMPAT;
	entBool BOTS_CAN_ADD_SHIT; 
	settings();
};

extern settings set;

#endif /* SETTINGS_HPP */
