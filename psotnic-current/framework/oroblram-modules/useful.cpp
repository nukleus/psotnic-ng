#include "includes/psotnic.h"
#include <string>

/*

  Useful

  This module adds a few useful but simple things that didn't warrant their own module

  Usage:
	!ping     -- will reply with !pong in channel
        !optopic  -- will set the topic to the base plus your words
        !list     -- will kick you ;)
        !lang     -- says text in lang

*/

void hook_privmsg(const char *from, const char *to, const char *msg) {
  // Check if we match any of our keywords
  if ((match("!ping",msg)) || (match("!list",msg)) || (match("!lang",msg)) || (match("!optopic *",msg))) {
    /*
      Change this to true if you want !ping to be public
    */
    bool pub = false;
    string begtop = "Welcome to " + string(to) + ". Speak English, no downloads or asl. ";
    char *lang = "Please speak English in here, as the topic (usually) clearly says.";
    string topic;
    chan *ch = findChannel(to);// channel we are acting in
    // check if we have a channel
    if(ch) {
      // get the user who is asking for the country
      chanuser *u = findUser(from, ch);
      // check what they want
      if (((u && u->flags & (IS_VOICE | IS_OP)) || pub) && (match("!ping",msg))) {
        privmsg(ch->name,"!pong");
      } else if (u && u->flags & IS_OP && match("!optopic *",msg)) {
        topic = string(msg).substr(9);
        setTopic(ch,string(begtop + topic).c_str()); 
      } else if (u && (!(u->flags & (IS_OP | IS_VOICE | HAS_O | HAS_V | HAS_E))) && (match("!list",msg))) {
        knockout(ch, u, "Read the topic next time, we don't do downloads.  Come back in 10 seconds or so if you must.", 10); 
      } else if (match("!lang",msg)) { 
        privmsg(ch->name,lang);
      } // end of check if we have a valid user (or are pub)
    } // end of check for if we found a valid channel
  } // end of check for if we matched our text
} // end of function

extern "C" module *init() {
    module *m = new module("!stuff in channels ;)", "Stuart Scott <stu@wilf.co.uk>", "0.1.0");
    m->hooks->privmsg = hook_privmsg;
    return m;
}

extern "C" void destroy() { }

