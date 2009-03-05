

#include "Adns.hpp"
#include "Chanuser.hpp"
#include "functions.hpp"
#include "global-var.h"
#include "match.h"
#include "module.h"

chanuser::chanuser(const char *str)
#ifdef HAVE_MODULES
	: CustomDataStorage()
#endif
{
	char *a = strchr(str, '!');
	if(a) mem_strncpy(nick, str, (int) abs(str - a) + 1);
	else mem_strcpy(nick, str);

	flags = 0;
	ident = NULL;
	host = NULL;
	hash = ::hash32(nick);
	handle = NULL;
	reason = NULL;
	ip4 = NULL;
	ip6 = NULL;

	HOOK( onNewChanuser( this ) );
}

chanuser::chanuser(const char *m, const chan *ch, const int f, const bool scan)
#ifdef HAVE_MODULES
	: CustomDataStorage()
#endif
{
	char *a = strchr(m, '!');
	char *b = strchr(m, '@');

	reason = NULL;

	if(!a || !b)
	{
		memset(this, 0, sizeof(chanuser));
		return;
	}

	mem_strncpy(nick, m, (int) abs(m - a) +1);
	mem_strncpy(ident, a+1, (int) abs(a - b));
	mem_strcpy(host, b+1);
	flags = f;
	if(scan)
		flags |= userlist.getFlags(m, ch);

	hash = ::hash32(nick);
	handle = NULL;

	switch(isValidIp(host))
	{
		case 4:
			dnsinfo = HOST_IPV4;
			mem_strcpy(ip4, host);
			mem_strcpy(ip6, "");
			break;

		case 6:
			dnsinfo = HOST_IPV6;
			mem_strcpy(ip4, "");
			mem_strcpy(ip6, host);
			break;

		default:
			dnsinfo = HOST_DOMAIN;
			mem_strcpy(ip4, "");
			mem_strcpy(ip6, "");
			break;
	}

	clones_to_check = CLONE_HOST | CLONE_IPV6 | CLONE_IPV4 | CLONE_IDENT | CLONE_PROXY;

	HOOK( onNewChanuser( this ) );
}

chanuser::~chanuser()
{
#ifdef HAVE_MODULES
	if(host)
		HOOK( onDelChanuser( this ) )
#endif
	if(nick) free(nick);
	if(ident) free(ident);
	if(ip4) free(ip4);
	if(ip6) free(ip6);
	if(host)
	{
		free(host);
	}

	if(reason) free(reason);
}

int chanuser::operator==(const chanuser &c) const
{
	return (hash == c.hash) && !strcmp(nick, c.nick);
}

int chanuser::operator<(const chanuser &c) const
{
	return strcmp(nick, c.nick) < 0 ? 1 : 0;
}

void chanuser::getFlags(const chan *ch)
{
	char *m = push(NULL, nick, "!", ident, "@", host, NULL);
	flags &= ~(HAS_ALL);
	flags |= userlist.getFlags(m, ch);
	free(m);
}

unsigned int chanuser::hash32() const
{
	return hash;
}

#ifdef HAVE_DEBUG
void chanuser::display()
{
	char *tmp, buf[MAX_LEN];
	if(flags & IS_OP)
	{
		strcpy(buf, "@");
		tmp = buf + 1;
	}
	else tmp = buf;
	userlist.flags2str(flags, tmp);

	printf("%s!%s@%s (%d) [%s] [%s] [%s]\n", nick, ident, host, hash, buf, ip4, ip6);
}
#endif

void chanuser::setReason(const char *r)
{
	if(reason)
		free(reason);

	mem_strcpy(reason, r);
}

int chanuser::matches(const char *mask) const
{
	static char buf[MAX_LEN];

	snprintf(buf, MAX_LEN, "%s!%s@%s", nick, ident, host);
	return match(mask, buf);
}

int chanuser::matchesBan(const char *mask) const
{
	return matchBan(mask, this);
}

bool chanuser::ok() const
{
	return nick && host && ident;
}

#ifdef HAVE_ADNS
int chanuser::updateDNSEntry()
{
	adns::host2ip *info = resolver->getIp(host);

	if(info)
	{
		if(ip4)
			free(ip4);
		if(ip6)
			free(ip6);

		mem_strcpy(ip4, info->ip4);
		mem_strcpy(ip6, info->ip6);

		if(*ip4)
			dnsinfo |= HOST_IPV4;
		if(*ip6)
			dnsinfo |= HOST_IPV6;

		DEBUG(printf(">>> Updating: %s (%s, %s)\n", nick, ip4, ip6));
		return 1;
	}

	return 0;
}
#endif

