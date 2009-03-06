/**
 * Date
 *
 * Displays and Calculates dates
 *
 * Commands: !date !gdate !day
 *
 * Copyright (c) ???? CGod <c@sii.ath.cx>
 */

#ifndef DATE_HPP
#define DATE_HPP 

#include "Module.hpp"

namespace Date
{
  class Date : public Module
  {
    public:
    Date(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);

    virtual bool onLoad(string &msg);
    virtual void onPrivmsg(const char *from, const char *to, const char *msg);
    int getday(int y,int m,int d);
  };

  enum Cmds
  {
    CMD_DATE=0,
    CMD_GDATE,
    CMD_DAY,

    N_CMDS
  };

}

#endif /* DATE_HPP */
