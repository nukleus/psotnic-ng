/********************************************************
* Psotnic peak module                                                                  *
* Copyright (c) 2005 matrix <admin@areaunix.org>               *
* Created: 14-05-2005 by matrix@IRCNet                                 *
* Usage: !peak in channel to show the peak                     *
*                Automatic when new peak in channel                    *
********************************************************/

#include "../prots.h"
#include "../global-var.h"
#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <cctype>

using namespace std;

void hook_join(chanuser *u, chan *ch, const char *mask, int netjoin) {
       ifstream filein;
       ofstream fileout;
       string line,data,newfile,say,chan;
       string peak = itoa(0);
       int check = 0;
       int max = ch->users.entries();
       chan = string(ch->name);
       transform(chan.begin(),chan.end(),chan.begin(),(int(*)(int))tolower);
       data = chan;
       data += ":";
       data += itoa(max);
       data += ";";
       say = "New channel peak in ";
       say += chan;
       say += " : ";
       say += itoa(max);

       filein.open("peak.data");
       getline(filein,line);
               newfile = line;
               if (line.find(chan,0) != string::npos)  {
                       peak = 
line.substr(line.find(":",line.find(chan,0))+1,line.find(";",line.find(chan,0))-1-line.find(":",line.find(chan,0)));
                       check = 1;
               }
       filein.close();

       if (check == 0) {
                       fileout.open("peak.data",ios::app);
                       fileout << data;
                       fileout.close();
       }

       if (itoa(max) > peak && peak != itoa(0)) {
               
newfile.replace(newfile.find(chan,0),newfile.find(";",newfile.find(chan,0))-newfile.find(chan,0)+1,data);
               ME.privmsg(ch->name,say.c_str(),NULL);
               fileout.open("peak.data",ios::trunc);
               fileout << newfile;
               fileout.close();
       }
}

void hook_privmsg(const char *from, const char *to, const char *msg) {
       ifstream filein;
       string peak,line,say,chan;
       chan = string(to);
       transform(chan.begin(),chan.end(),chan.begin(),(int(*)(int))tolower);
   if(match("!peak", msg)) {
               filein.open("peak.data");
               getline(filein,line);
               if (line.find(chan,0) != string::npos)  {
                       peak = 
line.substr(line.find(":",line.find(chan,0))+1,line.find(";",line.find(chan,0))-1-line.find(":",line.find(chan,0)));
               }
               filein.close();
               say = chan;
               say += " peak is : ";
               say += peak;
               ME.privmsg(to,say.c_str(),NULL);
        }
}

extern "C" module *init()
{
   module *m = new module("Peak detector", "matrix <admin@areaunix.org>", "0.1.0");
   m->hooks->join = hook_join;
       m->hooks->privmsg = hook_privmsg;
   return m;
}

extern "C" void destroy()
{
}


