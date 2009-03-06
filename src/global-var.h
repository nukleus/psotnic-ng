#ifndef PSOTNIC_GLOBAL_VAR_H
#define PSOTNIC_GLOBAL_VAR_H 1

#include <time.h>

#include "config.h"
#include "defines.h"

extern time_t NOW;
extern char *thisfile;
extern int hostNotify;
extern int stopPsotnic;
extern bool stopParsing;

#ifdef HAVE_DEBUG
extern int debug;
#endif
extern int creation;

#ifdef HAVE_IRC_BACKTRACE
extern char irc_buf[IRC_BUFS][MAX_LEN];
extern int current_irc_buf;
#endif

#endif
