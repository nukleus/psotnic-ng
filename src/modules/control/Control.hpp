#ifndef CONTROL_HPP
#define CONTROL_HPP 

#include "module.h"

class Control : public Module
{
    public:
    Control(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);
    virtual bool onLoad(string &msg);
    virtual void onPrivmsg(const char *from, const char *to, const char *msg);
    virtual void onBotnetcmd(const char *from, const char *cmd);
};

#endif /* CONTROL_HPP */
