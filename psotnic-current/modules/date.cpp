/**
 * Date
 *
 * Displays and Calculates dates
 *
 * Commands: !date !gdate !day
 *
 * Copyright (c) ???? CGod <c@sii.ath.cx>
 */

#include "../prots.h"
#include "../global-var.h"
#include "../module.h"

namespace Date
{
  const char CMD_PREFIX = '!';
  const int  DATE_SEP   = '/'; 

  const double YDAYS    = 365.2425;

  class Date : public Module
  {
    public:
    Date(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);

    virtual bool onLoad(string &msg);
    virtual void onPrivmsg(const char *from, const char *to, const char *msg);
    int getday(int y,int m,int d);
  };

  enum Cmds
  {
    CMD_DATE=0,
    CMD_GDATE,
    CMD_DAY,

    N_CMDS
  };

  const char *const CMDS[N_CMDS] =
  {
    "date",
    "gdate",
    "day",
  };

  const char *const DAYS[7]=
  {
    "Sun",
    "Mon",
    "Tue",
    "Wed",
    "Thu",
    "Fri",
    "Sat"
  };

  const int NDAYS[12]={31,28,31,30,31,30,31,31,30,31,30,31};

}

Date::Date::Date(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir) : Module(handle, file, md5sum, loadDate, dataDir)
{
}

bool Date::Date::onLoad(string &msg)
{
  return true;
}

int Date::Date::getday(int y,int m,int d)
{
  int days[12];

  m--;d--; //correct day and month to interval beginning with 0

  if (m<0 || d<0) return -1;

  memcpy(days,NDAYS,sizeof(days));

  bool leap=((abs(y)%4)==0);
  if (leap) days[1]++; //february has 29 days

  if (d>=days[m]) return -1;

  int nl=abs(y+3)/4; //number of leap days (at start of this year)

  int ndays=y*365;

  if (y!=0) ndays+=nl;

  for (int i=0;i<m;i++)
    ndays+=days[i];

  ndays+=d+5;
  ndays=abs(ndays);
  ndays%=7;

  return ndays;
}

void Date::Date::onPrivmsg(const char *from, const char *to, const char *msg)
{
  chan *ch = ME.findChannel(to);

  if (!ch) return; //message is not sent to channel

  chanuser *u = ch->getUser(from);

  if (!u) return; //user is not on channel

  if (msg[0]!=CMD_PREFIX) return; //message hasn't command prefix (e.g. '!')

  time_t t;
  static char buf[256];
  struct tm timedat;
  int    j,a,b,c,sdy;
  char   *endp;
  double sd,it; //star date, internet time

  for (int i=0;i<N_CMDS;i++)
    if (!strncmp(&msg[1],CMDS[i],strlen(CMDS[i])))
      switch (i)
      {
        case CMD_DATE: if (msg[1+strlen(CMDS[CMD_DATE])]!=0) break; //not exactly !date
	               time(&t);
	               memcpy(&timedat,localtime(&t),sizeof(struct tm));
		       timedat.tm_year+=1900;
		       timedat.tm_mon++;
		       sdy=timedat.tm_year-2323;
		       sd=((double)timedat.tm_yday+(double)(timedat.tm_year%4)/4.0)/365.2425*1000.0;
		       sd+=(double)(t%86400)/86400.0;
		       it=(double)((t+3600)%86400)/86.4;
		       sprintf(buf,"%s %02d%c%02d%c%02d %d:%02d:%02d (SD: %d%3.2lf) (IT: @%03.0lf)",
		         DAYS[timedat.tm_wday],
		         timedat.tm_year,DATE_SEP,
			 timedat.tm_mon,DATE_SEP,
			 timedat.tm_mday,
			 timedat.tm_hour,timedat.tm_min,timedat.tm_sec,
			 sdy,sd,it);
		       ME.privmsg(ch->name,buf,NULL);
	               break;
        case CMD_GDATE: if (msg[1+strlen(CMDS[CMD_GDATE])]!=0) break; //not exactly !gdate
	                time(&t);
		        memcpy(&timedat,gmtime(&t),sizeof(struct tm));
                        timedat.tm_year+=1900;
		        timedat.tm_mon++;
		        sprintf(buf,"%s %02d%c%02d%c%02d %d:%02d:%02d GMT",
		          DAYS[timedat.tm_wday],
		          timedat.tm_year,DATE_SEP,
			  timedat.tm_mon,DATE_SEP,
			  timedat.tm_mday,
			  timedat.tm_hour,timedat.tm_min,timedat.tm_sec);
                        ME.privmsg(ch->name,buf,NULL);
		        break;
        case CMD_DAY : if (msg[1+strlen(CMDS[CMD_DAY])]!=' ') break;

		       time(&t);
	               memcpy(&timedat,localtime(&t),sizeof(struct tm));

	               j=2+strlen(CMDS[CMD_DAY]);

		       if (j>=strlen(msg)) break;

		       a=strtol(&msg[j],&endp,10);

		       if (endp==&msg[j] || (*endp!=DATE_SEP && *endp!=0)) break;

		       if (*endp==0)
		         {
			   if (a>31)
			     {
			       sprintf(buf,"\x02%s:\x02 Invalid date",u->nick);
                               ME.privmsg(ch->name,buf,NULL);
			       break;
			     }

			   sdy=getday(timedat.tm_year+1900,timedat.tm_mon+1,a);
			 }
		       else
		         {
		           j+=endp-&msg[j]+1;
		           b=strtol(&msg[j],&endp,10);

		           if (endp==&msg[j] || (*endp!=DATE_SEP && *endp!=0)) break;

		           if (*endp==0)
		             {
                               if (a>12 || b>31)
                 	         {
		                   sprintf(buf,"\x02%s:\x02 Invalid date",u->nick);
                                   ME.privmsg(ch->name,buf,NULL);
				   break;
				 }

			       sdy=getday(timedat.tm_year+1900,a,b);
			     }
			   else
			     {
		               j+=endp-&msg[j]+1;
		               c=strtol(&msg[j],&endp,10);

		               if (*endp!=0) break;

                               if (b>12 || c>31)
			         {
		                   sprintf(buf,"\x02%s:\x02 Invalid date",u->nick);
                                   ME.privmsg(ch->name,buf,NULL);
				   break;
				 }

			       sdy=getday(a,b,c);
                             }
			}

		       if (sdy==-1)
			 {
		           sprintf(buf,"\x02%s:\x02 Invalid date",u->nick);
                           ME.privmsg(ch->name,buf,NULL);
			   break;
			 }

		       sprintf(buf,"%s",DAYS[sdy]);
		       ME.privmsg(ch->name,buf,NULL);
	               break;
      }
}

MOD_LOAD( Date::Date );
MOD_DESC( "date", "Displays and calculates dates, !date, !gdate, !day" );
MOD_AUTHOR( "CGod", "cgod@siigot.sytes.net" );
MOD_VERSION( "0.1" );

