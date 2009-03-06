#ifndef IGNORE_HPP
#define IGNORE_HPP 

#include "ptrlist.h"

/*! Ignore handling. */
class ign
{
	public:
	class entry
	{
		public:
		unsigned int ip;
		int count;
		time_t nextConn;
		time_t when;

		int operator==(const entry &e) const;
		int operator==(const unsigned int &IP) const;
		entry(unsigned int IP);
		time_t creation();
	};

	ptrlist<entry> data;

	int nextConn;
	int count;

	entry *hit(unsigned int ip);
	void removeHit(unsigned int ip);
	int isIgnored(unsigned int ip);
	void expire();
	void calcCount();

	void parseUser(char *who, char *cmd, char *ip);

	ign();
};

extern ign ignore;

#endif /* IGNORE_HPP */
