#ifndef HAVE_STATIC
	#define HAVE_MODULES
#endif

#define _NO_LAME_ERRNO			1

#ifndef HIDE_STDLIB_RAND
#define HIDE_STDLIB_RAND 1

#define rand hide_this_function
#define srand hide_this_function2

#include <stdlib.h>
#undef rand
#undef srand
int rand();
void srand(int a=0, int b=0, int c=0);

#endif

#include "config.h"
#include <stdarg.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/utsname.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include <utime.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>
//#include <arpa/nameser.h>
#include <dirent.h>
#define comment pwd_h_comment
#include <pwd.h>
#undef comment
#include <sys/types.h>
//#include <sys/dir.h>

#include <iostream>
#include <string>
#include <map>

using std::cin;
using std::cout;
using std::endl;
using std::string;

#ifdef HAVE_ADNS_PTHREAD
    #include <resolv.h>
#endif

#ifdef HAVE_ANTIPTRACE
    #include <sys/ptrace.h>
#endif

#ifdef HAVE_SSL
	#include <openssl/ssl.h>
	#include <openssl/err.h>
#endif

#ifdef HAVE_ADNS_PTHREAD
	#include <pthread.h>
#endif
#include <typeinfo>
#include <assert.h>
#ifdef HAVE_MODULES
    #include <dlfcn.h>
#endif

#ifdef HAVE_TCL
    #include <tcl.h>
#endif

#ifndef PSOTNIC_PROTS_H
#define PSOTNIC_PROTS_H 1

#include "svn-rev.h"
#include "pstring.h"
//#include "class-ent.h"
//#include "iterator.h"
#include "isaac.h"
#include "numeric_def.h"
#include "match.h"
#include "blowfish.h"
#include "md5.h"
#include "defines.h"
#include "structs.h"
#include "ptrlist.h"
#include "hashlist.h"
#include "fastptrlist.h"
#include "grass.h"
#include "classes.h"
#include "common.h"
#include "firestring.h"
#include "firedns.h"

typedef void (*sighandler_t)(int);
typedef void (*sig_t)(int);

/*! Formatted output.
 * \{
 * */
void printError( const char *format, ... );
void printItem( const char *format, ... );
void printMessage( const char *format, ... );
void printPrompt( const char *format, ... );
//! \}

/*! Read stuff from users.
 * \{
 * */
bool readUserInput( const char *prompt, pstring<>  &var,    const char *defaultValue="" );
void readUserInput( const char *prompt, entBool    &entity, bool force=false);
void readUserInput( const char *prompt, entInt     &entity, bool force=false);
//entTime
//entPerc
void readUserInput( const char *prompt, entHost   &entity);
void readUserInput( const char *prompt, entString &entity);
//entWord
void readUserInput( const char *prompt, entMD5Hash &entity );
//entHPPH
void readUserInput( const char *prompt, entHub     &entity );
void readUserInput( const char *prompt, entServer  &entity );
int readUserMC( const char *prompt, const char *choices[], size_t len, unsigned int defChoice );
bool readUserYesNo( const char *prompt, bool defaultValue );
//! \}

void createInitialConfig();

/* parse functions */
void parse_irc(char *data);
void parse_owner(inetconn *c, char *data);
void parse_bot(inetconn *c, char *data);
void parse_hub(char *data);
void parse_ctcp(char *mask, char *data, char *to);
int parse_botnet(inetconn *c, char *data);

/* net functions  */
int doConnect(const char *server, int port, const char *vhost, int noblock);
int doConnect(unsigned int server, int port, unsigned int vhost, int noblock);
#ifdef HAVE_IPV6
int doConnect6(const char *server, int port, const char *vhost, int noblock);
const char *inet6char(struct in6_addr* addr);
#endif

int acceptConnection(int fd, bool ssl);

int startListening(const char *ip, int port);
int getpeerport(int fd);
const char *getipstr(int fd, int proto, int (*fun)(int s, struct sockaddr *name, socklen_t *namelen));
unsigned int getip4(int fd, int (*fun)(int s, struct sockaddr *name, socklen_t *namelen));
int getport(int fd, int (*fun)(int s, struct sockaddr *name, socklen_t *namelen));
char *inet2char(unsigned int inetip);

void killSocket(int fd);
char read_byte(int fd);
int getIpVersion(int fd);
unsigned int network_bitcmp(unsigned char *d1, unsigned char *d2, int bits);

int inet_pton4(const char *src, unsigned char *dst);
int inet_pton6(const char *src, unsigned char *dst);

/* random stuff */
void divide(int *ret, int value, int parts, int part_size);
int getRandomNumbers(int top, int *ret, int num);
int getRandomItems(chanuser **ret, ptrlist<chanuser>::iterator start, int interval, int num, int ex=0);

/* string functions */
void nickCreator(char *nick);
bool extendhost(const char *host, char *buf, unsigned int len);
char *expand(const char *str, char *buf, int len, const char *args);
unsigned int hash32(const char *word);
int str2words(char *word, const char *str, int x, int y, int ircstrip=0);
void str2args(char *word, const char *str, int x, int y);
char *srewind(const char *str, int word);
char *memmem(void *vsp, size_t len1, void *vpp, size_t len2);
char *push(char *ptr, const char *lst, ...);
char *va_push(char *ptr, va_list ap, const char *lst, int size);
int va_getlen(va_list ap, const char *lst);
bool isNullString(const char *str, int len);
char *itoa(int val);
const char *getFileName(const char *path);
int count(const char *arr[]);
unsigned char *quoteHex(const char *str, unsigned char *hex);
char *quoteHexStr(const unsigned char *hex, char *str, int len=16);
bool isRealStr(const char *str);
char *timestr(const char *format, time_t &t);
char itoa_4bit(int x);
char* itoa_8bit(int n, char *buf);
char *getFullVersionString();
void hexDump(const char *str, int len);
int countWords(const char *str);
char *rtrim(char *str);

void mem_strncpy(char *&dest, const char *src, int n);
void mem_strcpy(char *&dest, const char *src);
void mem_strcat(char *&dest, const char *src);
char *nindex(const char *str, int count, char sep);

/* match */
int match(const char *mask, const char *str);
int wildMatch(const char *mask1, const char *mask2);
int matchIp(const char *bannedIp, const char *ip);
int matchBan(const char *ban, const char *nick, const char *ident, const char *host, const bool wild=0, const char *ip=NULL, const char *uid=NULL);
int matchBanMask(const char *ban, const char *mask, const bool wild=0, const char *ip=NULL, const char *uid=NULL);
int matchBan(const char *ban, const chanuser *u, const bool wild=0, const char *ip=NULL, const char *uid=NULL);

/* rest */
void precache();
void precache_expand();
void signalHandling();
void safeExit();
void propaganda();
void badparameters(const char *str);
void loadConfig(const char *file);
void sendLogo(inetconn *c);
long int nanotime();
void validate();
void parse_cmdline(int argc, char *argv[]);
void lurk();
int userLevel(chanuser *u);
int isValidIp(const char *str);
int imUp();
int listcmd(char what, const char *from, const char *arg1=NULL, const char *arg2=NULL, inetconn *c=NULL);
char *h_strerror(int error);
char *fetchVersion(char *buf, int client);
char *int2units(char *buf, int len, int val, unit_table *ut);
int units2int(const char *str, unit_table *ut, int &out);
bool isPrefix(char c);
int domaincmp(const char *s1, const char *s2, int n);
char *getPartOfDomain(const char *s, int n);
void addToCron(int i, char *argv[], int argc);
bool ipcmp(const char *s1, const char *s2, char sep, int count);
void error(const char *type, const char *str);
int enableCoreDumps();
void botnetcmd(const char *from, const char *cmd);
int rmdirext(const char *dir);
void dumpIrcBacktrace();

/* adns */
void *__adns_work(void *);

/* modules */
#ifdef HAVE_MODULES
int loadModule(const char *name);
#endif

bool _isnumber(const char *str);

void gen_ul_seed(unsigned char *);
void gen_cfg_seed(unsigned char *);

/* dlsym cast 'bug' warkaround */
DLSYM_FUNCTION dlsym_cast(void *handle, const char *symbol);
DLSYM_FUNCTION obj2fun(DLSYM_OBJECT o);
DLSYM_OBJECT fun2obj(DLSYM_FUNCTION f);

#endif
