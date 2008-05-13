#ifndef PSOTNIC_STRUCTS_H
#define PSOTNIC_STRUCTS_H 1

#include "pstring.h"

class comment;

struct psotnicHeader
{
	char id[8];
	unsigned int version;
};

struct unit_table
{
    char unit;
    int  ratio;
};

struct IOBUF
{
	char *buf;
	int pos;
	int len;
};

struct SOCKBUF
{
    int fd;
    char *buf;
    int len;
    int pos;
};

struct EXPANDINFO
{
	pstring<8> system;
	pstring<8> release;
	pstring<8> arch;
	pstring<8> version;
	pstring<8> realname;
};

struct flagTable
{
    const char letter;
    unsigned int flag;
    int level;
    unsigned int enforced;
    const char *desc;
    const char *long_desc;
};

#endif
