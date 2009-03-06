#ifndef PRVSET_HPP
#define PRVSET_HPP 

#include "class-ent.h"

class prvset : public options
{
	public:
	entInt debug_show_irc_write;
	entInt debug_show_irc_read;

	prvset();
};

extern prvset pset;

#endif /* PRVSET_HPP */
