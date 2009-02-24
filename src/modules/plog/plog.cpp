#include "plog.h"

Plog::Plog()
{
    globalSet=new globalSettings();
    client=new clientRecord(config.handle.getLen() ? config.handle : config.nick);
    client->globalSet=this->globalSet;
    chanlist.removePtrs();
    nextSave=0;
}

Plog::globalSettings::globalSettings()
{
    registerObject(LOG_PATH = entWord("path", 1, 255, "logs/$0.log"));
    registerObject(TIMESTAMP_FORMAT = entWord("timestamp-format", 1, 255, "[%H:%M]"));
    registerObject(OPEN_STRING = entWord("open-string", 1, 255, "--- Log opened %a %b %d %H:%M:%S %Y"));
    registerObject(CLOSE_STRING = entWord("close-string", 1, 255, "--- Log closed %a %b %d %H:%M:%S %Y"));
    registerObject(DAY_CHANGED_STRING = entWord("day-changed-string", 1, 255, "--- Day changed %a %b %d %Y"));
}

Plog::record::record(const char *name)
{
    fh=NULL;
    target=strdup(name);
}

Plog::record::plogOptions::plogOptions()
{
    registerObject(LOG = entBool("log", 0));
}

Plog::clientRecord::clientRecord(const char *target) : record(target)
{
    set=new settings();
    ptr=set;
    parsedQuit=false;
}

Plog::clientRecord::~clientRecord()
{
    stop();

    if(target)
        free(target);

    delete set;
}

Plog::clientRecord::settings::settings()
{
    // connecting?
    // connection established?
    // TODO: connect errors (there is no hook yet)
    registerObject(LOG_CONNECTED = entBool("log-connected", 1));
    registerObject(LOG_DISCONNECTED = entBool("log-disconnected", 1));
    registerObject(LOG_GOT_KILLED = entBool("log-got-killed", 1));
    registerObject(LOG_GOT_KLINED = entBool("log-got-klined", 1));
    registerObject(LOG_NICK_CHANGED = entBool("log-nick-changed", 1));
    registerObject(LOG_ERR_NICK = entBool("log-erroneous-nickname", 1));
    registerObject(LOG_GOT_INVITED = entBool("log-got-invited", 1));
    registerObject(LOG_CONNECTION_RESTRICTED = entBool("log-connection-restricted", 1));
    registerObject(LOG_GOT_MSG = entBool("log-got-msg", 1));
    registerObject(LOG_GOT_NOTICE = entBool("log-got-notice", 1));
    registerObject(LOG_GOT_CTCP = entBool("log-got-ctcp", 1));
    registerObject(LOG_USER_MODE = entBool("log-user-mode", 1));
    registerObject(LOG_JOIN_FAILED = entBool("log-join-failed", 0));
}

Plog::chanRecord::chanRecord(const char *target) : Plog::record(target)
{
    set=new settings();
    ptr=set;
}

Plog::chanRecord::~chanRecord()
{
    stop();

    if(target)
        free(target);

    delete set;
}

Plog::chanRecord::settings::settings()
{
    registerObject(LOG_MODE = entBool("log-mode", 1));
    registerObject(LOG_KICK = entBool("log-kick", 1));
    registerObject(LOG_JOIN = entBool("log-join", 1));
    registerObject(LOG_PART = entBool("log-part", 1));
    registerObject(LOG_QUIT = entBool("log-quit", 1));
    registerObject(LOG_CTCP = entBool("log-ctcp", 1));
    registerObject(LOG_NICK_CHANGE = entBool("log-nick-change", 1));
    registerObject(LOG_MSG = entBool("log-msg", 1));
    registerObject(LOG_NOTICE = entBool("log-notice", 1));
    registerObject(LOG_NICKLIST_ON_JOIN = entBool("log-nicklist-on-join", 1));
    registerObject(LOG_TOPIC = entBool("log-topic", 1));
}

/** starts logging.
 * @return true on success, otherwise false
 */

bool Plog::record::start()
{
    char buffer[MAX_LEN], *dir_name;
    bool new_file=false;

    struct stat statbuf;

    if(fh)
        return false; // or true? :)

    realFilename=getRealFilename();

    // create directories

    dir_name=dirname(realFilename);

    if(dir_name && *dir_name)
    {
        mkdirhier(dir_name);

        if(dir_name)
            free(dir_name);
    }

    if(stat(realFilename, &statbuf)==-1)
        new_file=true;

    fh=fopen(realFilename, "a");
    
    if(fh==NULL)
    {
        if(userlist.first) // avoid segfault
            net.send(HAS_N, PLOG_PARTYLINE_PREFIX, " Cannot open ", realFilename, ": ", strerror(errno), NULL);

        return false;
    }

    last=NOW;

    if(new_file)
        fprintf(fh, "### This log has been created by plog module for psotnic. You can get it at http://www.psotnic.com ###\n");

    strftime(buffer, MAX_LEN, globalSet->OPEN_STRING, localtime(&NOW));
    fprintf(fh, "%s\n", buffer);
    fflush(fh);
    return true;
}

/** stops logging.
 */

void Plog::record::stop()
{
    char buffer[MAX_LEN];

    if(!fh)
        return;

    strftime(buffer, MAX_LEN, globalSet->CLOSE_STRING, localtime(&NOW));
    fprintf(fh, "%s\n", buffer);
    fclose(fh);
    fh=NULL;
}

/** appends a line to the logfile.
 * logfile will be opened if it is not open already.
 *
 * @param text format string like printf()
 * @param ... arguments
 */

void Plog::record::log(const char *text, ...)
{
    va_list list;
    char buffer[MAX_LEN], time_str[MAX_LEN];
    struct tm *tm;
    int hour, day;

    if(!ptr->LOG)
        return;

    if(!fh)
    {
        // start logging
        if(!start())
            return;
    }

    tm=localtime(&NOW);
    hour=tm->tm_hour;
    day=tm->tm_mday;

    tm=localtime(&last);
    day-=tm->tm_mday;

    // check every hour if the path variable has changed. If yes, close the log and open it in the new directory
    if (tm->tm_hour!=hour)
        rotateCheck();

    if(day!=0)
    {
        strftime(buffer, MAX_LEN, globalSet->DAY_CHANGED_STRING, localtime(&NOW));
        fprintf(fh, "%s\n", buffer);
    }

    va_start(list, text);
    vsnprintf(buffer, MAX_LEN-1, text, list);
    va_end(list);

    strftime(time_str, MAX_LEN, globalSet->TIMESTAMP_FORMAT, localtime(&NOW));

    fprintf(fh, "%s %s\n", time_str, buffer);
    fflush(fh);
    last=NOW;
}

/** closes and opens logfile if the path has changed.
 * Example: path variable is "logs/$0-%Y-%m-%d", then we must
 * create a new logfile every day.
 * This will be executed every hour.
 */

void Plog::record::rotateCheck()
{
    char *new_fname;

    if (!fh || !realFilename)
        return;

    new_fname=getRealFilename();

    if(strcmp(new_fname, realFilename))
    {
        stop();
        start();
    }
}

/** returns the expanded file name.
 * First formatting by strftime() is done.
 * Then $0 will be replaced by file_name variable.
 *
 * @param file_name usually plog_*_rec->target
 * @return Returns the expanded file name
 */

char *Plog::record::getRealFilename()
{
    char buffer[MAX_LEN], *foo;

    strftime(buffer, MAX_LEN, globalSet->LOG_PATH, localtime(&NOW));

    std::string str=buffer;
    std::string::size_type pos;

    if((pos=str.find("$0"))!=std::string::npos)
        str.replace(pos, strlen("$0"), target);

    foo=(char*)malloc(strlen(str.c_str())+1);
    strcpy(foo, str.c_str());

    return foo;
}

/** adds a channel.
 */

Plog::chanRecord *Plog::addChannel(const char *target)
{
    chanRecord *node;

    if(findChannel(target))
        return NULL;

    node=new chanRecord(target);
    node->globalSet=this->globalSet; //! pointer to globalSet of the upper class
    chanlist.addLast(node);
    return node;
}

/** deletes a channel.
 */

bool Plog::delChannel(const char *target)
{
    chanRecord *logrec=findChannel(target);

    if(logrec)
    {
        logrec->stop();
        chanlist.remove(logrec);
        return true;
    }

    return false;
}

/** finds a channel.
 */

Plog::chanRecord *Plog::findChannel(const char *target)
{
    ptrlist<chanRecord>::iterator i;

    for(i=chanlist.begin(); i; i++)
        if(!strcasecmp(i->target, target))
            return i;

    return NULL;
}

/** like findChannel() but returns only the channel if logging is enabled.
 */

Plog::chanRecord *Plog::findLoggingChannel(const char *target)
{
    chanRecord *logrec=findChannel(target);

    if(logrec && logrec->set->LOG)
        return logrec;

    return NULL;
}

/** loads the config file.
 */

void Plog::loadConf()
{
    FILE *fh;
    char arg[10][MAX_LEN], buffer[MAX_LEN];
    int line=0;
    options::event *e;
    chanRecord *logrec;

    if(!(fh=fopen(PLOG_CONF, "r")))
        return;

    while(fgets(buffer, MAX_LEN, fh))
    {
        e=NULL;
        buffer[strlen(buffer)-1]='\0';
        line++;

        str2words(arg[0], buffer, 10, MAX_LEN);

        if(!*arg[0] || arg[0][0]=='#')
            continue;

        if(!strcmp(arg[0], "set"))
            e=globalSet->setVariable(arg[1], rtrim(srewind(buffer, 2)));

        else if(!strcmp(arg[0], "clientset"))
            e=client->set->setVariable(arg[1], rtrim(srewind(buffer, 2)));

        else if(!strcmp(arg[0], "addchan"))
            addChannel(arg[1]);

        else if(!strcmp(arg[0], "chanset"))
        {
            if((logrec=findChannel(arg[1])))
                e=logrec->set->setVariable(arg[2], rtrim(srewind(buffer, 3)));
        }

        if(e && !e->ok)
            printf("[-] %s:%d: %s\n", PLOG_CONF, line, (const char *) e->reason);
    }

    fclose(fh);
}

/** saves the config file.
 */

void Plog::saveConf()
{
    FILE *fh;
    ptrlist<ent>::iterator i;
    ptrlist<chanRecord>::iterator logrec;

    if(!(fh=fopen(PLOG_CONF, "w")))
    {
        net.send(HAS_N, PLOG_PARTYLINE_PREFIX, " Cannot open ", PLOG_CONF, " for writing: ", strerror(errno), NULL);
        nextSave=NOW+60; // try again later
        return;
    }

    for(i=globalSet->list.begin(); i; i++)
    {
        if(!i->isDefault() && i->isPrintable())
            fprintf(fh, "set %s\n", i->print());
    }

    for(i=client->set->list.begin(); i; i++)
    {
        if(!i->isDefault() && i->isPrintable())
            fprintf(fh, "clientset %s\n", i->print());
    }

    for(logrec=chanlist.begin(); logrec; logrec++)
    {
        fprintf(fh, "addchan %s\n", logrec->target);

        for(i=logrec->set->list.begin(); i; i++)
        {
            if(!i->isDefault() && i->isPrintable())
                fprintf(fh, "chanset %s %s\n", logrec->target, i->print());
        }
    }

    fclose(fh);
    nextSave=0;
    net.send(HAS_N, PLOG_PARTYLINE_PREFIX, " Autosaving plog config.", NULL);
}

void Plog::autoSaveConf()
{
    if(nextSave && nextSave<=NOW)
    {
        saveConf();
        nextSave=0;
    }
}

/** sets a time when to save config file.
 */

void Plog::setSave()
{
    nextSave=NOW+SAVEDELAY;
}

