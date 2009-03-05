#ifndef CHANUSER_HPP
#define CHANUSER_HPP 

#include "classes.h"
#include "CustomDataStorage.hpp"

struct HANDLE;

class chanuser
#ifdef HAVE_MODULES
	: public CustomDataStorage
#endif
{
	public:
	char *nick;
	char *ident;
	char *host;
	char *reason;
	unsigned int flags;
	HANDLE *handle;
	unsigned int hash;
	char *ip4;
	char *ip6;
	unsigned int dnsinfo;
	unsigned int clones_to_check;

	chanuser(const char *n, const char *m, const char *h, const int f=0);
	chanuser(const char *m, const chan *ch, const int f=0, const bool scan=0);
	chanuser(const char *str);
	void getFlags(const chan *ch);
	~chanuser();
	int operator==(const chanuser &c) const;
	int operator<(const chanuser &c) const;
	unsigned int hash32() const;
	void setReason(const char *);
	int matches(const char *mask) const;
	int matchesBan(const char *mask) const;
#ifdef HAVE_ADNS
	int updateDNSEntry();
#endif
	bool ok() const;

#ifdef HAVE_DEBUG
	void display();
#endif
};

#endif /* CHANUSER_HPP */
