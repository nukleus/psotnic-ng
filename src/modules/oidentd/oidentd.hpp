// oidentd spoofing module for psotnic

#ifndef OIDENTD_HPP
#define OIDENTD_HPP 

#include "Module.hpp"

class oidentd : public Module
{
    public:
    oidentd(void *, const char *, const char *, time_t, const char *);
    void oidentdSpoofing();

    virtual bool onLoad(string &msg);
    virtual void onConnecting();
#ifdef OS_RECONNECT
    virtual void onConnected();
#endif
#ifdef OS_METHOD2
    onTimer();
#endif

    private:
    char oidentd_cfg[MAX_LEN];
    time_t oidentd_tidy_up_time;
    string error;
};

#endif /* OIDENTD_HPP */
