

#include "Chanlist.hpp"
#include "Chanset.hpp"
#include "classes.h"
#include "global-var.h"
#include "module.h"
#include "Wasop.hpp"

CHANLIST::CHANLIST() :
#ifdef HAVE_MODULES
		CustomDataStorage(),
#endif
		status(0), nextjoin(0), updated(0), chset(0), wasop(0), allowedOps(0)
{
}

void CHANLIST::reset()
{
	if(chset)
		delete(chset);
	if(wasop)
		delete(wasop);
	if(protlist[BAN])
		delete(protlist[BAN]);
	if(protlist[INVITE])
		delete(protlist[INVITE]);
	if(protlist[EXEMPT])
		delete(protlist[EXEMPT]);
	if(protlist[REOP])
		delete(protlist[REOP]);
	if(allowedOps)
		 delete(allowedOps);

#ifdef HAVE_MODULES
	HOOK(onDelCHANLIST(this))
#endif

	chset = NULL;
	wasop = NULL;
	protlist[BAN] = NULL;
	protlist[INVITE] = NULL;
	protlist[EXEMPT] = NULL;
	protlist[REOP] = NULL;
	allowedOps = NULL;

	pass = "";
	name = "";

	status = nextjoin = updated = 0;
}

