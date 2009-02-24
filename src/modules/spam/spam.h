#ifndef SPAM_H
#define SPAM_H

class Spam : public Module
{
    public:
    Spam(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);
    ~Spam();

    virtual bool onLoad(string &msg);
    virtual void onCtcp(const char *from, const char *to, const char *msg);
    virtual void onPrivmsg(const char *from, const char *to, const char *msg);
    virtual void onNewChanuser(chanuser *me);
    virtual void onDelChanuser(chanuser *me);

    int countCrap(const char *str);
    void prepareCustomData();

    protected:
    regex_t spamChannel;
    regex_t spamWWW;
    regex_t spamOp;

    regmatch_t spamMatch;
};


#endif /* SPAM_H */
