#ifndef CHANLIST_HPP
#define CHANLIST_HPP 

#include "pstring.h"
#include "defines.h"
#include "CustomDataStorage.hpp"

class chanset;
class wasoptest;
class protmodelist;

class CHANLIST
#ifdef HAVE_MODULES
	: public CustomDataStorage
#endif
{
	public:
	pstring<> name;
	pstring<> pass;
	int status;
	int nextjoin;
	char updated;
	chanset *chset;
	wasoptest *wasop;
	wasoptest *allowedOps;
	protmodelist *protlist[4];

	CHANLIST();
	void reset();
};

#endif /* CHANLIST_HPP */
