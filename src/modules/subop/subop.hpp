#ifndef SUBOP_HPP
#define SUBOP_HPP 

#include "Module.hpp"

class SubOp : public Module
{
  public:
  SubOp(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);

  virtual bool onLoad(string &msg);
  virtual void onPrivmsg(const char *from, const char *to, const char *msg);
};

#endif /* SUBOP_HPP */
