/***************************************************************************
 *   Copyright (C) 2006 Esio			                           *
 *   esio@hoth.amu.edu.pl                                                  *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "prots.h"
#include "global-var.h"

offence::entry::entry(const char *_chan, const char *_mode, time_t _time, unsigned int _count, int _fromFlags, int _toFlags, const bool _global)
{
	mem_strcpy(chan, _chan);
	mem_strcpy(mode, _mode);
	time = _time;
	count = _count;
	fromFlags = _fromFlags;
	toFlags = _toFlags;
	global = _global;
}

offence::entry::~entry()
{
	if(chan) free(chan);
	if(mode) free(mode);
}


int offence::entry::operator==(const entry &ent) const
{
	if(strlen(ent.mode) && ent.count && ent.time)
	{
		if(!strcmp(ent.chan, chan) && !strcmp(ent.mode, mode) && count == ent.count && abs(ent.time - time) < OFFENCE_DUPLICATE_TIME)
//		if(!strcmp(ent.chan, chan) && !strcmp(ent.mode, mode) && count == ent.count && ent.time == time)
		return 1;
	}
	else
	{
		if(!strcmp(ent.chan, chan))
			return 1;
	}
	return 0;
}

int offence::entry::operator<(const entry &e) const
{
	// zrobmy na odwrot to bedzie sortowac od najmlodszych do najstarszych
	//return time < e.time ? 1 : 0;
	return time > e.time ? 1 : 0;
}

offence::offence()
{
	data.removePtrs();
}

int offence::add(const char *_chan, const char *_mode, time_t _time, unsigned int _count, int _fromFlags, int _toFlags, const bool _global)
{
	if(strlen(_mode) > 80) return 0;
	if(strlen(_chan) > CHAN_LEN) return 0;
	if(isRealStr(_chan) == 0) return 0;
	
	entry *e = new entry(_chan, _mode, _time, _count, _fromFlags, _toFlags, _global);
	
	/* should we remove duplicate entries ? */
//	while(data.find(*e))
//		data.remove(*e);

	if(data.entries() >= MAX_OFFENCES) 
	{
		ptrlist<offence::entry>::iterator ret = data.getItem(data.entries() -1);
		if(ret)
			data.remove(ret);
	}
	data.sortAdd(e);
	return 1;
}


int offence::del(const char *_chan, const char *_mode, time_t _time, unsigned int _count)
{
	entry e(_chan, _mode, _time, _count, 0, 0);

	return data.remove(e);
}

#ifdef HAVE_DEBUG
void offence::entry::display()
{
	printf("offence->chan: %s\n", chan);
	printf("offence->mode: %s\n", mode);
	printf("offence->time: %ld\n", time);
	printf("offence->count: %d\n", count);
	printf("offence->fromFlags: %d\n", fromFlags);
	printf("offence->toFlags: %d\n", toFlags);

}
#endif

offence::entry *offence::get(const char *_chan, const char *_mode, time_t _time, unsigned int _count)
{
	entry e(_chan, _mode, _time, _count, 0, 0);
	ptrlist<entry>::iterator ret = data.find(e);

	if(ret)
		return ret;
	else
		return NULL;
}

