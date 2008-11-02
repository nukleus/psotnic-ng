/*
  AntiWords - module for kick/ban user who use some words
  
  Programmed by cgod (c@sii.ath.cx)
  portability fix: Stefan Valouch <stefanvalouch@googlemail.com>
  
  files:
    words.cpp - this module
    
    words   - file with words (should be located in bin directory)
    wchans  - file with list of chans on which module operates
    wignore - file with ignored words
    wblist  - file with blacklist

  commands:
    !whelp              - print help on query
    !wadd <word>	- add word to database
    !wrem <word>	- remove word
    !wlist		- list all words
    !wsave 		- save words to file
    !wreload 	 	- reload words from file
    !wreset [user] 	- reset blacklist or user status
    !wblist   	 	- show blacklist
    !wchadd <chan>      - add chan to list
    !wchrem <chan>      - remove chan from list
    !wchlist            - list channels
    !wchsave            - save chanlist to file
    !wchreload          - reload channels-list from file
    !wiadd <word>	- add word to ignore list
    !wirem <word>       - remove word from ignore list
    !wilist             - show ignore list
    !wisave             - save ignore list
    !wireload           - reload ignore-list from file

  words (chans) file structure:
    one word/channel name per line
    Be aware of empty lines!
    
    * if you want to operate on all channels, leave chans file empty
    
  auto-save: this module automaticcaly save all data to data files 20 secs
             after last change 
*/

#include "../prots.h"
#include "../global-var.h"

#include <regex.h>
#include <stdarg.h>
#include <list>
#include <string>
#include <fstream>
#include <algorithm>

class DataFile
{
  public:
    DataFile(const char *fn):fname(fn) { changed=false; }
    
    bool mchanged() const { return changed; }
  protected:
    std::string fname;
    bool        changed;
    
    void saved()          { changed=false; }
};

//list
template <class T> class List:public DataFile
{
  public:
    typedef std::list <T> WList;

    List(const char *fn):DataFile(fn) {} //argument is filename
    
    void add(const T &w) { words.push_back(w);words.unique();changed=true; }
    bool rem(const T &);
    
    T *  find(const T&i)
      { 
        typename WList::iterator it=std::find(words.begin(),words.end(),i);
	if (it==words.end()) return NULL; else return &(*it);
      }

    void clear()         { words.clear();changed=true; }
    
    const std::string &get_fname() const { return fname; }

    virtual bool reload()=0;
    virtual bool save()=0;    
    
    virtual bool print(const chan *)     {}
    virtual bool print(const chanuser *) {}
  protected:
    WList       words;
};

namespace AntiWords
{
  typedef std::string Word;
}

class Words:public List <AntiWords::Word>
{
  public:
    enum { NO_MATCH = -1 };

    Words(const char *chfile):List <AntiWords::Word> (chfile) {}

    bool reload();
    bool save();
    
    bool print(const chanuser *);

    virtual int match(const AntiWords::Word &,int = 0,int * = NULL);
    
    static void lower(AntiWords::Word &);
  protected:
    static const char *const DELIM;
  
};

const char *const Words::DELIM = ", ";

//list of channels on which module is active
class Chans:public Words
{
  public:
    Chans(const char *chfile):Words(chfile) {}
    
    int match(const AntiWords::Word &ch,int = 0,int * = NULL)
      { if (words.empty()) return 0; else return Words::match(ch); }
};

//blacklist item - currently nick and number of violations
class BLItem
{
  public:
    BLItem(const char *str):n(0),nick(str) {}
    BLItem(const char *str,unsigned num):n(num),nick(str)   {}
    
    bool operator ==(const BLItem &i)
      { return !strcasecmp(nick.c_str(),i.nick.c_str()); }
    bool operator ==(const char *str) { return !strcasecmp(nick.c_str(),str); }
    
    unsigned operator =(unsigned u) { n=u;return u; }
    
    unsigned num() const         { return n; }
    const char *getnick() const { return nick.c_str(); }
  protected:
    std::string nick;
    unsigned    n;
};


class BlackList:public List <BLItem>
{
  public:
    BlackList(const char *bfile):List <BLItem> (bfile) {}
    
    bool reload();
    bool save();
    
    bool print(const chan *);
  protected:
    static const char *const DELIM;
};

const char *const BlackList::DELIM = ", ";

namespace AntiWords
{
  static const char NAME[]      = "AntiWords";
  static const char AUTHOR[]    = "cgod <c@sii.ath.cx>";
  static const char VERSION[]   = "1.0";
  
  static const char WORDS[]     = "words";
  static const char WCHANS[]    = "wchans";
  static const char IGNORE[]    = "wignore";
  static const char BLIST[]     = "wblist";
  
  static const char DELIM[]     = ", ";
  static const int  CMD_CHAR    = '!';
  static const int  CMD_SEP     = ' ';
  const int         CMD_MINFLAG = HAS_O; //minimal global flag needed
  const time_t      ASAVE_WAIT  = 20;    //20 seconds

  enum Cmds
    {
      CMD_WHELP=0,CMD_WABOUT,  //help
      CMD_WADD,CMD_WREM,CMD_WLIST,CMD_WSAVE,CMD_WRELOAD, //words
      CMD_WRESET,CMD_WBLIST, //blacklist
      CMD_WCHADD,CMD_WCHREM,CMD_WCHLIST,CMD_WCHSAVE,CMD_WCHRELOAD, //channels
      CMD_WIADD,CMD_WIREM,CMD_WILIST,CMD_WISAVE,CMD_WIRELOAD, //ignore-list

      N_CMDS
     };
     
  static const char *const CMDS[N_CMDS] =
  {
    "whelp",    //!whelp         - print help on query
    "wabout",   //!wabout        - print about info
    "wadd",	//!wadd <word>	 - add word to database
    "wrem",     //!wrem <word>	 - remove word
    "wlist",    //!wlist	 - list all words
    "wsave",    //!wsave 	 - save words to file
    "wreload",  //!wreload 	 - reload words from file
    "wreset",   //!wreset [user] - reset blacklist or user status
    "wblist",   //!wblist        - show blacklist
    "wchadd",   //!wchadd <chan> - add channel to list
    "wchrem",   //!wchrem <chan> - remove channel from list
    "wchlist",  //!wchlist       - list channels
    "wchsave",  //!wchsave       - save chanlist to file
    "wchreload",//!wchreload     - reload chanlist from file
    "wiadd",    //!wiadd <word>  - add word to ignore list
    "wirem",    //!wirem <word>  - remove word from ignore list
    "wilist",   //!wilist        - show ignore-words list
    "wisave",   //!wisawe        - save ignored words to file
    "wireload"  //!wireload      - reload ignored words form file
  };
  
  struct HelpLine
  {
    Cmds  cmd;
    const char  *help;
  };
  
  static const HelpLine HELP[N_CMDS] =
  {
    {CMD_WHELP,    "       - print help on query"},
    {CMD_WABOUT,   "       - print module name, version, author"},
    {CMD_WADD,     "<word> - add word to database"},
    {CMD_WREM,     "<word> - remove word"},
    {CMD_WLIST,    "       - list all words"},
    {CMD_WSAVE,    "       - save words to file"},
    {CMD_WRELOAD,  "       - reload words from file"},
    {CMD_WRESET,   "[user] - reset blacklist or user status"},
    {CMD_WBLIST,   "       - show blacklist"},
    {CMD_WCHADD,   "<chan> - add channel to list"},
    {CMD_WCHREM,   "<chan> - remove channel from list"},
    {CMD_WCHLIST,  "       - list channels"},
    {CMD_WCHSAVE,  "       - save chanlist to file"},
    {CMD_WCHRELOAD,"       - reload chanlist from file"},
    {CMD_WIADD,    "<word> - add word to ignore list"},
    {CMD_WIREM,    "<word> - remove word from ignore list"},
    {CMD_WILIST,   "       - show ignore-words list"},
    {CMD_WISAVE,   "       - save ignored words to file"},
    {CMD_WIRELOAD, "       - reload ignored words form file"}
  };

  Words     *words,*ignore;
  Chans	    *chans;
  BlackList *blacklist;
  
  void parsecmd(const chan *,const chanuser *,const char *);
  void print(const chan *,const char *,...);
  void print(const chanuser *,const char *str,...);
  void check_word(const Word &,chan *,chanuser *);
  int  global_flags(chanuser *);
  template <class T> void load(List <T> &);
  void print_help(const chanuser *);
  void print_about(const chan *);
}

void Words::lower(AntiWords::Word &str)
{
  int         i=0;
  
  while (str[i]!=0) { str[i]=tolower(str[i]);i++; }
}

template <class T> bool List <T> ::rem(const T &w)
{
  typename WList::iterator it=std::find(words.begin(),words.end(),w);
  
  if (it==words.end()) return false;
  
  words.erase(it);
  
  changed=true;
  
  return true;
}

bool Words::reload()
{
  char line[256]={0};

  std::fstream fs(fname.c_str(),std::ios_base::in);

  if (fs.bad()) return true;

  words.clear();
  do {
    fs.getline(line,256);

    if (fs.eof()) break;
    if (fs.fail()) { fs.close();return false; }

    std::string str;

    str.assign(line);
    lower(str);

    words.push_back(str);

  } while (!fs.eof() && !fs.fail());

  fs.close();

  saved();
  
  return true;
}

bool Words::save()
{
  std::fstream fs(fname.c_str(),std::ios_base::out|std::ios_base::trunc);

  if (fs.bad()) return false;
  
  WList::const_iterator it;
  
  for (it=words.begin();it!=words.end();it++)
    {
      fs << it->c_str() << std::endl;
      if (fs.bad()) { fs.close();return false; }
    }

  fs.close();

  saved();
  
  return true;
}

int Words::match(const AntiWords::Word &str,int spos,int *end)
{
  if (spos>=str.size()) return NO_MATCH;

  std::string s(str,spos);

  lower(s);

  WList::const_iterator it;

  for (it=words.begin();it!=words.end();it++)
    {
      if (it->size()>s.size()) continue;
      
      int i=s.find(it->c_str());

      if (i!=std::string::npos)
        {
	  if (end!=NULL) *end=i+it->size()-1;
          return i;
	}
    }

  return NO_MATCH;
}

bool Words::print(const chanuser *u)
{
  WList::const_iterator it=words.begin();
  std::string           str;
  
  static const size_t SIZE = 220u;
  
  if (it!=words.end()) str+=*it; else return false;
    
  it++;
  for (;it!=words.end();it++)
    {
      str+=DELIM;
      if (str.size()+it->size()>SIZE)
        {
          ME.privmsg(u->nick,str.c_str(),NULL);
	  str.clear();
        }
      str+=*it;
    }
  ME.privmsg(u->nick,str.c_str(),NULL);
  return true;
}

bool BlackList::reload()
{
  static char nick[256];
  
  unsigned    n;

  words.clear();
  
  std::fstream fs(fname.c_str(),std::ios_base::in);

  if (fs.bad()) return true; //file does not exist - no blacklist

  do {
    fs.get(nick,256,' ');
    
    if (fs.eof()) break;
    if (fs.fail()) { fs.close();return false; }
    
    fs >> n;
    
    if (fs.fail()) { fs.close();return false; }
    
    fs.get();  //extracts CR
    
    words.push_back(BLItem(nick,n));

  } while (!fs.eof() && !fs.fail());

  fs.close();
 
  saved();

  return true;
}

bool BlackList::save()
{
  std::fstream fs(fname.c_str(),std::ios_base::out|std::ios_base::trunc);

  if (fs.bad()) return false;
  
  WList::const_iterator it;
  
  for (it=words.begin();it!=words.end();it++)
    {
      fs << it->getnick() << ' ' << it->num() << std::endl;
      if (fs.bad()) { fs.close();return false; }
    }

  fs.close();

  saved();

  return true;
}


bool BlackList::print(const chan *ch)
{
  static char times[20];

  WList::const_iterator it=words.begin();
  std::string           str;
  
  static const size_t SIZE = 220u;
  
  if (it!=words.end())
    {
      str+=it->getnick();
      snprintf(times,20," (%dx)",it->num());
      str+=times;
    }
  else return false;
    
  it++;
  for (;it!=words.end();it++)
    {
      str+=DELIM;
      
      std::string buf;
      
      buf+=it->getnick();
      snprintf(times,20," (%dx)",it->num());
      buf+=times;
      
      if (str.size()+buf.size()>SIZE)
        {
          ME.privmsg(ch->name,str.c_str(),NULL);
	  str.clear();
        }
	
      str+=buf;
    }
  ME.privmsg(ch->name,str.c_str(),NULL);
  return true;
}

void AntiWords::print(const chan *ch,const char *str,...)
{
  static char buf[256];
  static char msg[256];
  
  va_list args;
  
  va_start(args,str);
  vsnprintf(buf,255,str,args);
  va_end(args);
  
  snprintf(msg,255,"\x02%s:\x02 %s",NAME,buf);
  
  ME.privmsg(ch->name,msg,NULL);
}


void AntiWords::print(const chanuser *u,const char *str,...)
{
  static char buf[256];
  va_list args;
  
  va_start(args,str);
  vsnprintf(buf,256,str,args);
  va_end(args);
  
  ME.privmsg(u->nick,buf,NULL);
}

void AntiWords::print_help(const chanuser *u)
{
  print(u,"%s script, version %s, programmed by %s\n\n",NAME,VERSION,AUTHOR);
  for (int i=0;i<N_CMDS;i++)
    print(u,"%c%-10s%s",CMD_CHAR,CMDS[HELP[i].cmd],HELP[i].help);
}

void AntiWords::print_about(const chan *ch)
{
  static char buf[256];
  
  snprintf(buf,256,"\x02%s\x02 module, version %s, programmed by %s",NAME,VERSION,AUTHOR);
  ME.privmsg(ch->name,buf,NULL);
}

void AntiWords::parsecmd(const chan *ch,const chanuser *u,const char *line)
{
  if (*line!=CMD_CHAR) return;
  
  std::string ln(line+1);
  
  Words::lower(ln);
  
  int pos=ln.find_first_of(CMD_SEP);
  
  std::string cmd,param;
  
  if (pos==std::string::npos)
    cmd=ln;
  else
    {    
      cmd.assign(ln,0,pos);
    
      int ppos=ln.find_first_not_of(CMD_SEP,pos);

      param.assign(ln,ppos,ln.size()-ppos); //extraxts parameter(s)
    }

  for (int i=0;i<N_CMDS;i++)
    if (cmd==CMDS[i])
      {
        switch (i)
	{
	  case CMD_WHELP:
	        print_help(u);
		break;
	  case CMD_WABOUT:
	        print_about(ch);
	        break;
	  case CMD_WADD:
 	        words->add(param);
		print(ch,"added word \"%s\" to list.",param.c_str());
		break;
	  case CMD_WREM:
                if (words->rem(param))
    		  print(ch,"removed word \"%s\" from list.",param.c_str());
		else
		  print(ch,"word \"%s\" not in list",param.c_str());
		break;
	  case CMD_WLIST:
	        if (!words->print(u))
		  print(ch,"words list empty");
		break;
	  case CMD_WSAVE:
	        if (words->save())
		  print(ch,"words saved.");
		else
		  print(ch,"error saving words");
		break;
	  case CMD_WRELOAD:
	        if (words->reload())
		  print(ch,"words reloaded.");
		else
		  print(ch,"error reloading words");  
		break;
	  case CMD_WRESET:
	        if (param.empty())
		  {
		    blacklist->clear();
		    print(ch,"blacklist erased.");
		  }
		else
		  {
		    if (!blacklist->rem(BLItem(param.c_str())))
		      print(ch,"user \"%s\" not in blacklist.",param.c_str());
		    else
		      print(ch,"user \"%s\" erased from blacklist.",param.c_str());
		  }
		break;
	  case CMD_WBLIST:
	        if (!blacklist->print(ch))
		  print(ch,"blacklist is empty");
		break;  
	  case CMD_WCHADD:
	        chans->add(param);
		print(ch,"added channel \"%s\".",param.c_str());
		break;
	  case CMD_WCHREM:
	        if (chans->rem(param))
    		  print(ch,"removed channel \"%s\" from list.",param.c_str());
		else
		  print(ch,"chan \"%s\" not in list",param.c_str());
		break;
	  case CMD_WCHLIST:
	        if (!chans->print(u))
		  print(ch,"operating on all channels");
		break;
	  case CMD_WCHSAVE:
	        if (chans->save())
                  print(ch,"chanlist saved.");
		else
		  print(ch,"error saving chanlist");
		break;
	  case CMD_WCHRELOAD:
	        if (chans->reload())
		  print(ch,"chans list reloaded.");
		else
		  print(ch,"error reloading chans list");
		break;
	  case CMD_WIADD:
 	        ignore->add(param);
		print(ch,"added word \"%s\" to ignore list.",param.c_str());
		break;
	  case CMD_WIREM:
                if (ignore->rem(param))
    		  print(ch,"removed word \"%s\" from ignore list.",param.c_str());
		else
		  print(ch,"word \"%s\" not in ignore list",param.c_str());
		break;
	  case CMD_WILIST:
	        if (!ignore->print(u))
		  print(ch,"ignore-words list empty");
		break;
	  case CMD_WISAVE:
	        if (ignore->save())
		  print(ch,"ignore-words list  saved.");
		else
		  print(ch,"error saving ignore-words list");
		break;
	  case CMD_WIRELOAD:
	        if (words->reload())
		  print(ch,"ignore-words list reloaded.");
		else
		  print(ch,"error reloading ignore-words list");
		break;
	}
	return;
      }
}

void AntiWords::check_word(const Word &msg,chan *ch,chanuser *u)
{
  static char buf[MAX_LEN];

  Word word(msg);
  
  for (;;)
  {
    int pend;
    
    int pos=ignore->match(word,0,&pend);
    if (pos==Words::NO_MATCH) break;
    word.erase(pos,pend-pos+1);
    if (word.empty()) return;
  }

  if (words->match(word)==Words::NO_MATCH) return;
  
  BLItem *bi=blacklist->find(u->nick);
	
  if (bi==NULL)
    {
      blacklist->add(BLItem(u->nick));
      bi=blacklist->find(u->nick);
    }

  int t;
  switch (bi->num())
  {
    case 0 : u->setReason("Watch what are you typing!");
             ch->toKick.sortAdd(u);
             ch->kick(u,"Watch what are you typing!");
             break;
    case 1 : u->setReason("Watch what are you typing! (one more time and ban)");
             ch->toKick.sortAdd(u);
             ch->kick(u,"Watch what are you typing! (one more time and ban)");
             break;
    case 2 : ch->knockout(u,
              "You have been warned to avoid some words - expires in 5 mins",
              5*60);
             break;
    default: t=(bi->num()-1)*5;
             snprintf(buf,MAX_LEN,"Get out of here! (ban for %d mins)",t);
             ch->knockout(u,buf,t*60);
             break;
  }
  (*bi)=bi->num()+1;
}

int AntiWords::global_flags(chanuser *u)
{
  std::string mask;
  HANDLE      *h=userlist.first;
  int         maxflag=0;
  
  mask=u->nick;mask+="!";mask+=u->ident;mask+="@";mask+=u->host;
  while (h)
  {
    for (int i=0;i<MAX_HOSTS;i++)
      if (h->host[i]!=NULL && match(h->host[i],mask.c_str()))
        if (h->flags[GLOBAL]>maxflag)
	  maxflag|=h->flags[GLOBAL];
    h=h->next;
  }
  return maxflag;
}

template <class T> void AntiWords::load(List <T> &l)
{
  if (!l.reload())
    printf("[-] %s: Error loading file %s\n",NAME,l.get_fname().c_str());
}

void hook_notice(const char *from,const char *to, const char *msg)
{
  using namespace AntiWords;

  chan *ch = ME.findChannel(to);
    
  if(!ch) return;

  if (chans->match((const char *)ch->name)==Words::NO_MATCH) return;

  chanuser *u = ch->getUser(from);
	
  if (u->flags & (HAS_V | HAS_O | IS_OP | IS_VOICE)) return;

  AntiWords::check_word(msg,ch,u);
}

void hook_ctcp(const char *from, const char *to, const char *msg)
{
  using namespace AntiWords;
  
  chan *ch = ME.findChannel(to);
    
  if(!ch) return;

  if (chans->match((const char *)ch->name)==Words::NO_MATCH) return;

  chanuser *u = ch->getUser(from);
	
  if (u->flags & (HAS_V | HAS_O | IS_OP | IS_VOICE)) return;

  if (!match("ACTION *", msg)) return;
	
  AntiWords::check_word(msg,ch,u);
}

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    using namespace AntiWords;

    static char buf[MAX_LEN];
    
    chan *ch = ME.findChannel(to);

    if(!ch) return;
    
    chanuser *u = ch->getUser(from);
	
    if (!u) return;
    
    if (((global_flags(u)&CMD_MINFLAG)==CMD_MINFLAG) && u->flags & (HAS_O|IS_OP))
      parsecmd(ch,u,msg);

    if (u->flags & (HAS_V | HAS_O | IS_OP | IS_VOICE)) return;
    
    if (chans->match((const char *)ch->name)==Words::NO_MATCH) return;
    
    AntiWords::check_word(msg,ch,u);
}

void hook_timer()
{
//  using namespace AntiWords;
  
  static time_t last_save=0;
  
  if (!last_save) { last_save=NOW;return; }
  
  if (NOW-last_save>=AntiWords::ASAVE_WAIT)
    {
      if (AntiWords::words->mchanged()) AntiWords::words->save();
      if (AntiWords::blacklist->mchanged()) AntiWords::blacklist->save();
      if (AntiWords::ignore->mchanged()) AntiWords::ignore->save();
      if (AntiWords::chans->mchanged()) AntiWords::chans->save();
      
      last_save=NOW;
    }
}

extern "C" module *init()
{
//  using namespace AntiWords;

  module *m = new module(AntiWords::NAME,AntiWords::AUTHOR,AntiWords::VERSION);
    
  AntiWords::words=new Words(AntiWords::WORDS);
  AntiWords::ignore=new Words(AntiWords::IGNORE);
  AntiWords::chans=new Chans(AntiWords::WCHANS);
  AntiWords::blacklist=new BlackList(AntiWords::BLIST);

  AntiWords::load(*AntiWords::words);
  AntiWords::load(*AntiWords::ignore);
  AntiWords::load(*AntiWords::chans);
  AntiWords::load(*AntiWords::blacklist);

  m->hooks->privmsg = hook_privmsg;
  m->hooks->ctcp = hook_ctcp;
  m->hooks->timer = hook_timer;
  m->hooks->notice = hook_notice;
    
  return m;
}
	    
extern "C" void destroy()
{
//  using namespace AntiWords;
  
  if (AntiWords::words!=NULL)     { delete AntiWords::words; AntiWords::words=NULL; }
  if (AntiWords::ignore!=NULL)    { delete AntiWords::ignore; AntiWords::ignore=NULL; }
  if (AntiWords::chans!=NULL)     { delete AntiWords::chans; AntiWords::chans=NULL;  }
  if (AntiWords::blacklist!=NULL) { delete AntiWords::blacklist; AntiWords::blacklist=NULL; }
}
