#ifndef MASKLIST_HPP
#define MASKLIST_HPP 

#include "ptrlist.h"

class masklist_ent
{
	public:
	char *mask;
	time_t expire;
	time_t when;
	char *who;
	bool sent;

	masklist_ent(const char *m, const char *w, time_t t);
	~masklist_ent();
};

class masklist
{
	public:
	ptrlist<masklist_ent> masks;
	bool received;

	int add(const char *mask, const char *who, time_t);
	int remove(char *mask);
	masklist_ent *find(const char *mask);
	masklist_ent *wildMatch(char *mask);
	masklist_ent *matchBan(char *mask, char *ip, char *uid);
	masklist_ent *match(const char *mask);
	
	ptrlist<masklist_ent>::iterator expire(ptrlist<masklist_ent>::iterator m=0);
//	masklist_ent *exactFind(char *mask);
	int remove(masklist_ent *m);
	void clear();
	masklist();
};

#endif /* MASKLIST_HPP */
