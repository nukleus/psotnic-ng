

#include "Prvset.hpp"

prvset::prvset()
{
	registerObject(debug_show_irc_write = entInt("debug_show_irc_write", 0, 3, 0));
	registerObject(debug_show_irc_read = entInt("debug_show_irc_read", 0, 3, 0));
}

