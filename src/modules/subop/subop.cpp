

#include "prots.h"
#include "global-var.h"
#include "module.h"
#include <cstring>
#include <string>

/*

  SubOp

  This is a kind-of-port of the Eggdrop subop script.  It will allow +e'd people to kick + ban people as well as setting the topic

  Usage:
	Users should be voiced and +e'd on the bot before anything works

	!kick <user> <reason>     -- Will kick the user with the reason
	!kban <user> <reason>     -- Will ban for 20 minutes
        !quick <user>             -- Will ban for 10 seconds
	!topic <topic>		  -- Will set the topic to '<nick>: <topic>'

*/

class SubOp : public Module
{
  public:
  SubOp(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir);

  virtual bool onLoad(string &msg);
  virtual void onPrivmsg(const char *from, const char *to, const char *msg);
};

SubOp::SubOp(void *handle, const char *file, const char *md5sum, time_t loadDate, const char *dataDir) : Module(handle, file, md5sum, loadDate, dataDir)
{
}

bool SubOp::onLoad(string &msg)
{
  return true;
}

void SubOp::onPrivmsg(const char *from, const char *to, const char *msg)
{
  // Check if we match any of our keywords
  if (match("!kick *",msg) || match("!kban *",msg) || match("!quick *",msg) || match("!topic *",msg)) {
    char arg[50][MAX_LEN];      // arguments
    char user[MAX_LEN] = "";   //  the user to perform the action on
    char rest[MAX_LEN] = "";    // the rest of the line (minus the user)
    char whole[MAX_LEN] = "";  //  the whole line
    char nick[15] = "";         // local nickname
    char *pch;                 //  position holder
    chan *ch = ME.findChannel(to); // channel we are acting in
    // check if we have a channel
    if(ch) {
      //do i have op ?
      if(ch->me->flags & IS_OP) {
        // get the user who is performing the action
        chanuser *u = ch->getUser(from);
        // check if the user is valid, has the e flag and is voiced or oped in the channel currently
        if((u) && (u->flags & HAS_E) && (u->flags & IS_VOICE || u->flags & IS_OP)) {
          // break up the line
          str2words(arg[0], msg, 50, MAX_LEN, 0);
          // get the user
          strcpy(user,arg[1]);
          // loop through the line to concat it into one string again (minus the user)
          for (int i=2;i < 50;i++) { if (strlen(arg[i]) > 0) { strcat(rest,strcat(arg[i]," ")); } }
          // get the 'whole' line (the user to act on, a space and the rest of the line)
          strcat(whole,user);
          strcat(whole," ");
          strcat(whole,rest);
          // get just the nick from the nick!ident@host string
          pch=strchr(from,'!');
          strncat(nick,from,pch-from);
          // check if we are setting the topic
          if(match("!topic *",msg)) {
            // stick 'nick:' on the front of the topic string
            strcat(nick,":");
            strcat(nick,whole);
            // set the topic
            net.irc.send("TOPIC ", (const char *) ch->name, " :", nick, NULL);
          } else {
            // we arent setting the topic, so we are acting on another user, get that user
            chanuser *o = ch->getUser(user);
            // check if the person is trying to kick either myself or a permanent owner
            if (o && ((o == ch->me) || (o->flags & HAS_X)) && !(u->flags & HAS_X)) { 
              // kick the user for being naughty
              ch->kick(u,"Don't try it, fucker.");
            // check if we are trying to kick someone we shouldnt..
            } else if (o && (o != u) && (!(o->flags & (HAS_E | HAS_O | HAS_H | HAS_S | HAS_L)) || ((u->flags & HAS_X) && !(o->flags & HAS_X)))) {
              // check if we are trying to kickban
              if(match("!kban *",msg)) {
                // create the kick message
                strcat(nick,":");
                strcat(nick,rest);
                // kickban the user for 1200 seconds (20 mins) with the created reason
                ch->knockout(o,nick,1200);
              // check if we are kicking
              } else if(match("!kick *",msg)) {
                // kick the user with the reason
                ch->kick(o,rest);
              // check if we are quickbanning a user
              } else if(match("!quick *",msg)) {
                // create the kick message
                strcat(nick,":");
                strcat(nick,"Quickban.");
                // kickban the user for 10 seconds
                ch->knockout(o,nick,10);
              } // end of checks for kick/ban type
            } // end of check for kicking an invalid user
          } // end of check for setting topic or kick/banning
        } // end of check for a valid user sending the command
      } // end of check for is I have ops
    } // end of check for if we found a valid channel
  } // end of check for if we matched our text
} // end of function

MOD_LOAD( SubOp );
MOD_DESC( "SubOp", "Eggdrop-like SubOp in channels" );
MOD_AUTHOR( "Stuart Scott", "stu@wilf.co.uk" );
MOD_VERSION( "0.1.0" );

