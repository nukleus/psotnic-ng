#ifndef HAVE_STATIC
	#define HAVE_MODULES
#endif

#define _NO_LAME_ERRNO			1

#ifndef HIDE_STDLIB_RAND
#define HIDE_STDLIB_RAND 1

#define rand hide_this_function
#define srand hide_this_function2

#include <stdlib.h>
#undef rand
#undef srand
int rand();
void srand(int a=0, int b=0, int c=0);

#endif

#include "config.h"
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
//#include <arpa/nameser.h>
#include <dirent.h>
#define comment pwd_h_comment
#include <pwd.h>
#undef comment
#include <sys/types.h>
//#include <sys/dir.h>

#include <iostream>
#include <string>
#include <map>

using std::cin;
using std::cout;
using std::endl;
using std::string;

#ifdef HAVE_ADNS_PTHREAD
    #include <resolv.h>
#endif

#ifdef HAVE_ANTIPTRACE
    #include <sys/ptrace.h>
#endif

#ifdef HAVE_SSL
	#include <openssl/ssl.h>
	#include <openssl/err.h>
#endif

#ifdef HAVE_ADNS_PTHREAD
	#include <pthread.h>
#endif
#include <typeinfo>
#include <assert.h>
#ifdef HAVE_MODULES
    #include <dlfcn.h>
#endif

#ifdef HAVE_TCL
    #include <tcl.h>
#endif

#ifndef PSOTNIC_PROTS_H
#define PSOTNIC_PROTS_H 1

#include "pstring.h"
//#include "class-ent.h"
//#include "iterator.h"
#include "isaac.h"
#include "numeric_def.h"
#include "match.h"
#include "blowfish.h"
#include "md5.h"
#include "defines.h"
#include "structs.h"
#include "ptrlist.h"
#include "hashlist.h"
#include "fastptrlist.h"
#include "grass.h"
#include "classes.h"
#include "common.h"
#include "firestring.h"
#include "firedns.h"
#ifdef HAVE_MODULES
#include "module.h"
#endif

#endif
