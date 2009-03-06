#ifndef RAW_HPP
#define RAW_HPP 

#include "Module.hpp"

class Raw : public Module
{
    public:
    Raw(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);

    virtual bool onLoad(string &msg);
    virtual void onBotnetcmd(const char *from, const char *cmd);

    int calculatePenalty(const char *data);
    void calculatePenaltyOfChanmode(char *modes, int args, int *mypenalty);
    void calculatePenaltyOfUsermode(char *modes, int *mypenalty);
};

#endif /* RAW_HPP */
