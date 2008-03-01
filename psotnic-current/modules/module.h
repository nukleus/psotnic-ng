void mem_strcpy	(char *&dest, const char *src);
int ircd_strcmp (const char *str1, const char *str2);
int ircd_strncmp (const char *str1, const char *str2, int n);
char* itoa (int num);
chanuser* findUser (const char *nick, chan *channel);
void sendToOwner (const char *owner, const char *msg);
void sendToOwners (const char *msg);
int getPenalty ();
void privmsg (const char *to, const char *msg);
void notice (const char *to, const char *msg);
char* handle ();
char* nick ();
void cycle (chan *channel, const char *reason, int rejoinDelay);
char* server ();
char* ircIp ();
int saveUserlist (const char *file, const char *pass);
void flags2str (int flags, const char *str);
void setTopic (chan *ch, const char *topic);
bool isSticky (const char *ban, chan *ch);

// descr: those three functions attempt to kick given user(s).
//        kick is only performed if penalty is low enough,
//        otherwise user(s) will _NOT_ be kicked.
int kick4 (chan *channel, chanuser **multHandle, int num);
int kick6 (chan *channel, chanuser **multHandle, int num);
int kick (chan *channel, chanuser *p, const char *reason);

// descr: this function retruns 1 if `str' matches the `mask'
//        otherwise 0 is returned.
int match (const char *mask, const char *str);

// descr: function rewinds a `str' by given number of `word's
//        (double spaces / tabs are ingored)
//
// example:  char *a, buf[] = "     one two     three four"
//           a = srewind(buf, 2)  // a == "three four";
//           a = srewind(buf, 10) // a == NULL;
char* srewind (const char *str, int word);

// descr: this function divides a string `str' in to `x'
//        words each of maximum lenght `y', when ircstrip
//        is set to 1 colons will be striped from the begining
//        of each word
//
// example: char arg[10][MAX_LEN], "one two three four five";
//          str2words(arg[0], data, 10, MAX_LEN);
//              //arg[0] = "one", arg[2] = "three", ..., arg[5] = "", and so on;
void str2words (const char *word, const char *str, int x, int y, int ircstrip);

// descr: returns a pointer to chanlist named `name' or NULL if
// such chanlist is not present in the userlist
CHANLIST* findChanlist (const char *name);

// descr: returns a pointer to channel named `name' or NULL if
// bot is not joined to such channel
chan* findChannel (const char *name);

// descr: sets the reason of removal for given chanuser, if no
//        reason has been set bot will use default kick reason
void setReason (chanuser *u, const char *reason);

// descr: those functions add mode/kick to queue, in case of addMode
//        parameter prio must be set to PRIO_LOW or PRIO_HIGH
//        (low priority or high priority queue)
void addMode (chan *channel, const char mode[2], const char *arg, int prio, int delay);
void addKick (chan *channel, chanuser *p, const char *reason);

// descr: this function manualy flushes given queue on given channel,
//        funtion returns number of flushed modes or 0 when penalty
//        is too high (> 10 for PRIO_HIGH and != 0 for PRIO_LOW)
int flushModeQueue (chan *channel, int prio);


// descr: this function registers custom constructor and destructor for
//        given `Class', function returns 1 if `Class' supports custom
//        constructors and destructors, otherwise 0 is returned, please
//        check framework.h for classes supporting custom constructors
//        destructors
//
// example: see repeat.cpp

int initCustomData (const char *Class, FUNCTION constructor, FUNCTION destructor);


// descr: temporary bans and kicks chanuser from a channel
void knockout (chan *ch, chanuser *u, const char *reason, int delay);

// desc: returns NULL on failure or data if any data was fetched from given link
//
// example: char *info = httpget(http://some.site.com/file.php?info=8234234");
//          //process data
//          free(info);
//

char* httpget (const char *link);


// desc: find first connection from given handle
inetconn* findConnByHandle (HANDLE *h);
inetconn* findConnByName (const char *name);

// desc: tells the bot that it should not do any further parsing
void stop ();

void reconnect (char *reason, int delay);
