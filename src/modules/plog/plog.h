#include "../../prots.h"
#include "../../global-var.h"
#include <string>

#define PLOG_CONF "plog.conf"
#define PLOG_PARTYLINE_PREFIX "[plog]"

char *get_hostmask(const char *);
char *strip_color(const char *, int, int);
bool mkdirhier(char *);
char *dirname(char *);

struct Plog
{
    /** global settings.
     */

    struct globalSettings : public options
    {
        entWord LOG_PATH;
        entWord TIMESTAMP_FORMAT;
        entWord OPEN_STRING;
        entWord CLOSE_STRING;
        entWord DAY_CHANGED_STRING;

        globalSettings();
    };

    /** record.
     * This is a base class/struct which will be not be used directly.
     */

    struct record
    {
        /** options which every record has.
         */

        struct plogOptions : public options
        {
            entBool LOG;
            plogOptions();
        };

        FILE *fh; // file handler
        char *target; // channel name or config.handle
        char *realFilename; // the current expanded file name
        time_t last; // when last message was written
        globalSettings *globalSet;
        plogOptions *ptr; // TODO: rename

        record(const char *);

        bool start();
        void stop();
        void log(const char *, ...);
        void rotateCheck();
        char *getRealFilename();
    };

    /** client record.
     * This is a derived class of "record" which is used to log irc traffic that
     * is sent to the bot and not to a channel.
     */

    struct clientRecord : public record
    {
        struct settings : public plogOptions
        {
            entBool LOG_CONNECTED;
            entBool LOG_DISCONNECTED;
            entBool LOG_GOT_KILLED;
            entBool LOG_GOT_KLINED;
            entBool LOG_ERR_NICK;
            entBool LOG_GOT_INVITED;
            entBool LOG_CONNECTION_RESTRICTED;
            entBool LOG_GOT_MSG;
            entBool LOG_GOT_NOTICE;
            entBool LOG_GOT_CTCP;
            entBool LOG_USER_MODE;
            entBool LOG_JOIN_FAILED;
            entBool LOG_NICK_CHANGED;
    
            settings();
        };

        clientRecord(const char *target);
        ~clientRecord();
        settings *set;
        bool parsedQuit;
    };

    /** channel record.
     * This is a derived class of "record" which is used to log  irc traffic of channels
     */

    struct chanRecord : public record
    {
        struct settings : public plogOptions
        {
            entBool LOG_MODE;
            entBool LOG_KICK;
            entBool LOG_JOIN;
            entBool LOG_PART;
            entBool LOG_QUIT;
            entBool LOG_MSG;
            entBool LOG_NOTICE;
            entBool LOG_CTCP;
            entBool LOG_NICK_CHANGE;
            entBool LOG_NICKLIST_ON_JOIN;
            entBool LOG_TOPIC;
    
            settings();
        };
    
        chanRecord(const char *target);
        ~chanRecord();
        settings *set;
    };

    Plog();
    globalSettings *globalSet;
    clientRecord *client;
    ptrlist<chanRecord> chanlist;
    time_t nextSave; //! time when to perform next saveConf()

    chanRecord *addChannel(const char *);
    chanRecord *findChannel(const char *);
    chanRecord *findLoggingChannel(const char *);
    bool delChannel(const char *);
    void loadConf();
    void saveConf();
    void autoSaveConf();
    void setSave();
};
