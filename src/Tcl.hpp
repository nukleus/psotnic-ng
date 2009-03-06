#ifndef TCL_HPP
#define TCL_HPP 

#ifdef HAVE_TCL

#include <tcl.h>

class tcl
{
	Tcl_Interp *tcl_int;
	char *setGlobalVar(char *name, char *value);
	void addCommands();

	public:

	int curid;
	struct timer_t
	{
		char *nick;
		time_t exp;
		int id;
	};

	ptrlist<timer_t> timer;

	int load(char *script);
	Tcl_Interp *getInt();
	int eval(char *cmd);

	void expireTimers();

	tcl();
	~tcl();

};

extern tcl tclparser;

#endif

#endif /* TCL_HPP */
