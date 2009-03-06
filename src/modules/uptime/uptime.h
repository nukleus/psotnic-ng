#ifndef UPTIME_H
#define UPTIME_H 

#include <sys/param.h>

#ifdef linux
    #define PROC_FS
#else
    #define SYSCTL
    #include <sys/sysctl.h>
#endif

#include "Module.hpp"

class Uptime : public Module
{
	private:
	bool has_global_flag(chanuser *, int);
	int get_uptime();

	public:
	Uptime( void *, const char *, const char *, time_t, const char * );

	virtual bool onLoad( string &msg );
	virtual void onPrivmsg( const char *from, const char *to, const char *msg );
};

#endif /* UPTIME_H */
