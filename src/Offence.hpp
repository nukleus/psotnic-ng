#ifndef OFFENCE_HPP
#define OFFENCE_HPP 

#include "ptrlist.h"

class offence
{
	public:
	class entry
	{
		public:
		char *chan; //channel
		char *mode; // irc line
		time_t time; //offence time
		unsigned int count; // offence in line counter (1-5)
		int fromFlags; // flags before change
		int toFlags; //flags after change
		bool global;

		entry(const char *_chan, const char *_mode, time_t _time, unsigned int _count, int _fromFlags, int _toFlags, const bool _global=false);
		~entry();

		int operator==(const entry &ent) const;
		int operator<(const entry &e) const;
#ifdef HAVE_DEBUG
		void display();
#endif
	};

	ptrlist<entry> data;

	int add(const char *_chan, const char *_mode, time_t _time, unsigned int _count, int _fromFlags, int _toFlags, const bool _global=false);
	int del(const char *_chan, const char *_mode, time_t _time, unsigned int _count);

	offence::entry *get(const char *_chan, const char *_mode="", time_t _time=0, unsigned int _count=0);
	offence();
};

#endif /* OFFENCE_HPP */
