#ifndef WASOP_HPP
#define WASOP_HPP 

#include "ptrlist.h"

class wasoptest
{
	class entry
	{
		public:
		char *mask;
		time_t when;

		entry(char *n, char *i, char *h);
		entry(char *m, int alloc=1);
		~entry();
	};

	public:
	ptrlist<entry> data;
	int TOL;
	time_t since;

	int add(chanuser *p);
	int add(char *nick, char *ident, char *host);
	int add(char *mask, int alloc=1);

	int remove(chanuser *user);
	int remove(char *nick, char *ident, char *host);
	int remove(char *mask);

	static int checkSplit(const char *reason);
	void expire();
	bool isEmpty();

	wasoptest(int life=60*45);
};

#endif /* WASOP_HPP */
