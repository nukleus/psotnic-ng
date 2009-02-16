/*
  Peak - module for checking channel peak
  
  Programmed by cgod (cgod@siigot.sytes.net)
  portability fix: Stefan Valouch <stefanvalouch@googlemail.com>
  
  Merits:
    * based on AntiWords engine
    * saving of peak dates and nicks (who makes the peak)
    * auto-save of data to files 20 secs. after new peak
    * module control based on channel flags
        (CMD_MINFLAG to show peak and CMD_OPFLAG (global) to add/remove channels)
    * join-flood protection
  
  files:
    peak2.cpp - this module
    
    peaks     - file with saved channel peaks
    pchans    - file with channels on whitch peak module operates

  commands:
    !peak [chan] 	- write peak of current channel or channel chan
    !preset <chan>      - reset peak data for channel chan
    !pchadd <chan>	- add channel to peak-monitor list
    !pchrem <chan>	- remove channel from peak-monitor list
    !pchlist		- list channels on which peak module operates

  peaks file structure:
    one peak data (channel nick number time) per line
  pchans file structure:
    one channel name pre line
    Be aware of empty lines!
    
    * leave pchans file empty to operate on all channels
    
  auto-save: this module automatically saves data to data files 20 secs
             after last change 
*/

#include "../prots.h"
#include "../global-var.h"

#include <regex.h>
#include <stdarg.h>
#include <time.h>

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

    bool empty() const   { return words.empty(); }
    
    void clear()         { words.clear();changed=true; }
    
    const std::string &get_fname() const { return fname; }

    virtual bool reload()=0;
    virtual bool save()=0;    
    
    virtual bool print(const chan *)     {}
    virtual bool print(const chanuser *) {}
  protected:
    WList       words;
};

namespace Peak
{
  typedef std::string Chan;
}

//list of channels on which module is active
class Chans:public List <Peak::Chan>
{
  public:
    enum { NO_MATCH = -1 };

    Chans(const char *chfile):List <Peak::Chan> (chfile) {}

    bool reload();
    bool save();
    
    bool print(const chanuser *);

    static void lower(Peak::Chan &);
  protected:
    static const char *const DELIM;
};

const char *const Chans::DELIM = ", ";

//Peak list item - channel name, time, nick
class PeakItem
{
  public:
    PeakItem(const char *ch):pchan(ch),ptime(0),val(0) {} //for find()
    PeakItem(const char *ch,const char *n):pchan(ch),nick(n),ptime(NOW),val(0) {}
    PeakItem(const char *ch,const char *n,unsigned v):pchan(ch),nick(n),ptime(NOW),val(v) {}
    PeakItem(const char *ch,const char *n,unsigned v,time_t t):pchan(ch),nick(n),ptime(t),val(v) {}

    PeakItem(const PeakItem &i) { pchan=i.pchan;nick=i.nick;ptime=i.ptime;val=i.val; }
    
    bool operator ==(const PeakItem &i)
      { return !strcasecmp(pchan.c_str(),i.pchan.c_str()); }
    bool operator ==(const char *str)
      { return !strcasecmp(pchan.c_str(),str); }
    
    void operator =(const PeakItem &i)
      { nick=i.nick;val=i.val;ptime=i.ptime; }
    
    unsigned gettime() const    { return ptime; }
    unsigned getval() const     { return val; }
    const char *getnick() const { return nick.c_str(); }
    const char *getchan() const { return pchan.c_str(); }
    
    bool print(const chan *);
  protected:
    std::string pchan;
    std::string nick;
    time_t      ptime;
    unsigned    val;
};

class PeakList:public List <PeakItem>
{
  public:
    PeakList(const char *pfile):List <PeakItem> (pfile) {}
    
    bool reload();
    bool save();
    
    void mark_ch() { changed=true; }
};

namespace Peak
{
  static const char NAME[]      = "Peak";
  static const char AUTHOR[]    = "cgod <cgod@siigot.sytes.net>";
  static const char VERSION[]   = "1.0";
  
  static const char PEAKS[]     = "peaks";
  static const char PCHANS[]    = "pchans";
  
  static const int  CMD_CHAR    = '!';
  static const int  CMD_SEP     = ' ';
  const int         CMD_MINFLAG = IS_OP|IS_VOICE; //minimal local flag needed
  const int         CMD_OPFLAG  = HAS_O;          //who may add/remove channels (global flag)
  const time_t      ASAVE_WAIT  = 20;             //20 seconds
  const time_t      FLOOD_SECS  = 5;

  enum Cmds
    {
      CMD_PHELP=0,CMD_PABOUT,  //help
      CMD_PEAK,CMD_PRESET,
      CMD_PCHADD,CMD_PCHREM,CMD_PCHLIST, //channels

      N_CMDS
     };
     
  static const char *const CMDS[N_CMDS] =
  {
    "phelp",    //!phelp         - print help on query
    "pabout",   //!pabout        - print about info
    "peak",     //!peak	[chan]	 - print peak of channel
    "preset",   //!preset <chan> - reset peak data
    "pchadd",   //!pchadd <chan> - add channel to list
    "pchrem",   //!pchrem <chan> - remove channel from list
    "pchlist",  //!pchlist       - list channels
  };
  
  struct HelpLine
  {
    Cmds  cmd;
    const char  *help;
  };
  
  static const HelpLine HELP[N_CMDS] =
  {
    {CMD_PHELP,    "       - print help on query"},
    {CMD_PABOUT,   "       - print module name, version, author"},
    {CMD_PEAK,	   "[chan] - print channel peak"},
    {CMD_PRESET,   "<chan> - reset peak data"},
    {CMD_PCHADD,   "<chan> - add channel to list"},
    {CMD_PCHREM,   "<chan> - remove channel from list"},
    {CMD_PCHLIST,  "       - list channels"},
  };

  Chans	    *chans;
  PeakList  *peaklist;
  
  void parsecmd(const chan *,const chanuser *,const char *,int);
  void print(const chan *,const char *,...);
  void print(const chanuser *,const char *str,...);
  void check_peak(chan *ch,chanuser *u);
  int  global_flags(chanuser *);
  template <class T> void load(List <T> &);
  void print_help(const chanuser *);
  void print_about(const chan *);
}

void Chans::lower(Peak::Chan &str)
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

bool Chans::reload()
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

bool Chans::save()
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

bool Chans::print(const chanuser *u)
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

bool PeakList::reload()
{
  static char pchan[256];
  static char nick[256];
  unsigned    t;
  unsigned    val2;

  words.clear();
  
  std::fstream fs(fname.c_str(),std::ios_base::in);

  if (fs.bad()) return true; //file does not exist - no blacklist

  do {
    fs.get(pchan,256,' ');
    
    if (fs.eof()) break;
    if (fs.fail()) { fs.close();return false; }

    fs.get(); //extracts space

    if (fs.eof()) break;
    if (fs.fail()) { fs.close();return false; }

    fs.get(nick,256,' ');
    
    if (fs.eof()) break;
    if (fs.fail()) { fs.close();return false; }

    fs >> val2;

    if (fs.fail()) { fs.close();return false; }

    fs >> t;

    if (fs.fail()) { fs.close();return false; }
    
    fs.get();  //extracts CR
    
    words.push_back(PeakItem(pchan,nick,val2,(time_t)t));

  } while (!fs.eof() && !fs.fail());

  fs.close();
 
  saved();

  return true;
}

bool PeakList::save()
{
  std::fstream fs(fname.c_str(),std::ios_base::out|std::ios_base::trunc);

  if (fs.bad()) return false;
  
  WList::const_iterator it;
  
  for (it=words.begin();it!=words.end();it++)
    {
      fs << it->getchan() << ' ' << it->getnick() << ' '<< it->getval() << ' ' << it->gettime() << std::endl;
      if (fs.bad()) { fs.close();return false; }
    }

  fs.close();

  saved();

  return true;
}


void Peak::print(const chan *ch,const char *str,...)
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


void Peak::print(const chanuser *u,const char *str,...)
{
  static char buf[256];
  va_list args;
  
  va_start(args,str);
  vsnprintf(buf,256,str,args);
  va_end(args);
  
  ME.privmsg(u->nick,buf,NULL);
}

void Peak::print_help(const chanuser *u)
{
  print(u,"%s module, version %s, programmed by %s\n\n",NAME,VERSION,AUTHOR);
  for (int i=0;i<N_CMDS;i++)
    print(u,"%c%-10s%s",CMD_CHAR,CMDS[HELP[i].cmd],HELP[i].help);
}

void Peak::print_about(const chan *ch)
{
  static char buf[256];
  
  snprintf(buf,256,"\x02%s\x02 module, version %s, programmed by %s",NAME,VERSION,AUTHOR);
  ME.privmsg(ch->name,buf,NULL);
}

void Peak::parsecmd(const chan *ch,const chanuser *u,const char *line,int gflag)
{
  if (*line!=CMD_CHAR) return;
  
  std::string ln(line+1);
  
  Chans::lower(ln);
  
  int pos=ln.find_first_of(CMD_SEP);
  
  std::string cmd,param;
  
  if (pos==std::string::npos)
    cmd=ln;
  else
    {    
      cmd.assign(ln,0,pos);
    
      int ppos=ln.find_first_not_of(CMD_SEP,pos);

      param.assign(ln,ppos,ln.size()-ppos); //extracts parameter(s)
    }

  PeakItem    *pi;
  time_t      pt;
  std::string st;
  
  for (int i=0;i<N_CMDS;i++)
    if (cmd==CMDS[i])
      {
        switch (i)
	{
	  case CMD_PHELP:
	        print_help(u);
		break;
	  case CMD_PABOUT:
	        print_about(ch);
	        break;
	  case CMD_PEAK:
	        if (param.empty())
		  param=(const char *)ch->name;
		Chans::lower(param);
		if (chans->empty() || ((!chans->empty()) && ((chans->find(param))!=NULL)))
		  {
		    pi=peaklist->find(PeakItem(param.c_str()));
		    
		    if (pi==NULL)
		      print(ch,"no peak detected yet.");
		    else
		    {
		      pt=pi->gettime();
		      
		      st=ctime(&pt);
		      st.erase(st.size()-1); //erase \n at the end
		      
		      print(ch,"\x02%d\x02 users (set on \x02%s\x02 by \x02%s\x02)",pi->getval(),st.c_str(),pi->getnick());
		    }
		  }
		else
		  print(ch,"module does not operate on this channel.");
		  
	        break;
	  case CMD_PRESET:
	        if ((gflag&CMD_OPFLAG)!=CMD_OPFLAG) break;

		if (param.empty()) break;

		pi=peaklist->find(PeakItem(param.c_str()));
		    
		if (pi==NULL)
		   print(ch,"no peak for \"%s\".",param.c_str());
		else
		  {
	            peaklist->rem(*pi);
		    print(ch,"peak data for \"%s\" removed.",param.c_str());
		  }
	        break;
	  case CMD_PCHADD:
	        if ((gflag&CMD_OPFLAG)!=CMD_OPFLAG) break;
	        chans->add(param);
		print(ch,"added channel \"%s\".",param.c_str());
		break;
	  case CMD_PCHREM:
	        if ((gflag&CMD_OPFLAG)!=CMD_OPFLAG) break;
	        if (chans->rem(param))
    		  print(ch,"removed channel \"%s\" from list.",param.c_str());
		else
		  print(ch,"chan \"%s\" not in list",param.c_str());
		break;
	  case CMD_PCHLIST:
	        if ((gflag&CMD_OPFLAG)!=CMD_OPFLAG) break;
	        if (!chans->print(u))
		  print(ch,"operating on all channels");
		break;
	}
	return;
      }
}

void Peak::check_peak(chan *ch,chanuser *u)
{
  bool     newp=false;
  PeakItem *pi=peaklist->find(PeakItem(ch->name));
  bool     flood=false;	
  
  if (pi==NULL)
    {
      peaklist->add(PeakItem(ch->name,u->nick,ch->users.entries()));
      pi=peaklist->find(PeakItem(ch->name));
      newp=true;
    }

  if (newp || ch->users.entries()>pi->getval())
    {
      if (NOW-pi->gettime()<FLOOD_SECS)
        flood=true;
	
      *pi=PeakItem(ch->name,u->nick,ch->users.entries(),NOW);
      peaklist->mark_ch();
      
      if (!flood)
        print(ch, "\x02%d\x02 users.",pi->getval());
    }
}

int Peak::global_flags(chanuser *u)
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


void hook_join(chanuser *u,chan *ch,const char *,int)
{
  using namespace Peak;
  
  Chan pch(ch->name);
  
  if (chans->empty() || ((!chans->empty()) && (chans->find(pch)!=NULL)))
    check_peak(ch,u);
}

template <class T> void Peak::load(List <T> &l)
{
  if (!l.reload())
    printf("[-] %s: Error loading file %s\n",NAME,l.get_fname().c_str());
}

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    using namespace Peak;

    static char buf[MAX_LEN];
    
    chan *ch = ME.findChannel(to);

    if(!ch) return;
    
    chanuser *u = ch->getUser(from);
	
    if (!u) return;
    
    if (u->flags & CMD_MINFLAG)
      parsecmd(ch,u,msg,global_flags(u));
}

void hook_timer()
{
  using namespace Peak;
  
  static time_t last_save=0;
  
  if (!last_save)
    last_save=NOW;
  else
    if (NOW-last_save>=ASAVE_WAIT)
      {
        if (peaklist->mchanged()) peaklist->save();
        if (chans->mchanged()) chans->save();
      
        last_save=NOW;
      }
}

extern "C" module *init()
{
  using namespace Peak;

  module *m = new module(NAME,AUTHOR,VERSION);
    
  chans=new Chans(PCHANS);
  peaklist=new PeakList(PEAKS);

  load(*chans);
  load(*peaklist);

  m->hooks->privmsg = hook_privmsg;
  m->hooks->timer = hook_timer;
  m->hooks->join = hook_join;
    
  return m;
}
	    
extern "C" void destroy()
{
  using namespace Peak;
  
  if (chans!=NULL)     { delete chans;chans=NULL;  }
  if (peaklist!=NULL) { delete peaklist;peaklist=NULL; }
}

