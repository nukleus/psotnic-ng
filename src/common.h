#ifndef PSOTNIC_COMMON_H
#define PSOTNIC_COMMON_H 1

#include "defines.h"
#include "pstring.h"

class CHANLIST;
class ptime;
class Comment;
class offence;

struct HANDLE
{
	char *name;
	char *host[MAX_HOSTS];
	char *hostBy[MAX_HOSTS];
	unsigned char pass[16];
	int flags[MAX_CHANNELS+1];
	unsigned int ip;
	unsigned long long int channels;
	HANDLE *next;
	HANDLE *prev;
	char updated;
	ptime *creation;
	Comment *info;
	offence *history;
	char *createdBy;
};
#endif
