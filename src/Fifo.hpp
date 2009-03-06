#ifndef FIFO_HPP
#define FIFO_HPP 

#include "pstring.h"
#include "ptrlist.h"

class inetconn;

class fifo
{
	public:
	int maxEnt;
	static time_t lastFlush;
	int flushDelay;
	ptrlist<pstring<8> > data;

	fifo(int size=0, int delay=1);
	~fifo();
	int push(const char *lst, ...);
	int wisePush(const char *lst, ...);
	int wildWisePush(char *lst, ...);
	char *pop();
	int flush(inetconn *c);
	char *flush();
};

extern fifo ctcp;
extern fifo invite;

#endif /* FIFO_HPP */
