#include "includes/psotnic.h"
#include <string.h>
#include <string>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cctype>

/*

  Country Codes

  This module adds a !country command that will tell you what country a 2 letter code represents, or if
  you use .code then what country a TLD is owned by.

  Usage:
	!country <ISO code>   --  Will reply with the name of the country for that code as defined by ISO 3166
	!country .<tld>       --  Will reply with the name of the country for that TLD
  i.e.:
        <user> !country uk
        <bot> Country code 'uk' is 'United Kingdom'.
        <user> !country .uk
        <bot> TLD '.uk' is owned by 'United Kingdom'.
  
  Note: 
       there is a MySQL version of this module that adds support for searching on country name and wildcard searches
       if you have access to MySQL, use that one instead ;)

*/

void hook_privmsg(const char *from, const char *to, const char *msg) {
  // Check if we match any of our keywords
  if (match("!country *",msg)) {
    /*
      Change this to true if you want this command to be public, i.e. open to ALL people on a channel
       this isnt recommended, as it gives a means for someone flooding your bot off (I havent written
       flood protection into this function as for me its restricted to known users only)
    */
    bool pub = false;
    bool tld = false; // boolean holding if we want a tld or a code
    bool alt = false; // boolean holding if we found an alternative match
    bool hit = false; // boolean holding if we hit a valid country code or not
    string cc,code,desc,altcode; // some strings ;)
    char TLDcc[MAX_LEN] = "/usr/local/share/psotnics/country"; // filename containing the codes
    char arg[2][MAX_LEN]; // arguments
    char line[256]; // the character line holder
    chan *ch = findChannel(to);// channel we are acting in
    std::fstream fs; // filestream
    // check if we have a channel
    if(ch) {
      // get the user who is asking for the country
      chanuser *u = findUser(from, ch);
      // check if the user is a known one (to stop lamers flooding us off with requests)
      if((u && u->flags & (IS_VOICE | IS_OP)) || pub) {
        // break up the line
        str2words(arg[0], msg, 2, MAX_LEN, 0);
        // get the country
        cc = arg[1];
        // check if we are looking for a tld or code, and setup altcode in each case
        if (cc.find(".",0) == 0) { 
          altcode = cc;
          altcode.erase(0,1);
          tld = true; 
        } else { 
          altcode = "." + cc;
          tld = false;
        }
        // make the code lowercase
        transform (cc.begin(),cc.end(), cc.begin(), tolower);         
        // open the country code file
        fs.open(TLDcc, fstream::in);
        // check if the open file worked
        if (fs.bad()) { 
          // file didnt open properly, so tell the channel and return
          privmsg(ch->name,"Error, country file not found."); 
          return; 
        }
        // loop through the file
        do {
          // get the line, check the line worked
          fs.getline(line,256);
          code = line;
          if (fs.eof() || fs.fail()) { break; }  
          // get the next line and check if it worked
          fs.getline(line,256);
          if (fs.eof() || fs.fail()) { break; }  
          // check if we hit our code
          if (code == cc) {
            // we hit our code, set hit to true and setup our description holder
            hit = true;
            // set our description
            desc = line;
            // break out of our loop
            break;
          }
          // check if we hit an alt
          if (code == altcode) { 
            // we hit an alt code, set alt to true
            alt = true;
            // set our desc
            desc = line;
          }
        // loop until we hit the end of file or fail
        } while (!fs.eof() && !fs.fail());
        // close the file
        fs.close();
        // check what we are searching for
        if (tld) {
          // check if we hit our tld.  If we did, tell the channel the description, else tell them we missed
          if (hit) { desc = "TLD '" + cc + "' is '" + desc + "'."; }
          else if (alt) { desc = "TLD '" + cc + "' was not found, but country code '" + altcode + "' is '" + desc + "'"; }
          else { desc = "TLD '" + cc + "' was not found."; }
        } else {
          // check if we hit our country code.  If we did, tell the channel the description, else tell them we missed
          if (hit) { desc = "Country code '" + cc + "' is '" + desc + "'."; }
          else if (alt) { desc = "Country code '" + cc + "' was not found, but TLD '" + altcode + "' is '" + desc + "'"; }
          else { desc = "Country code '" + cc + "' was not found."; }
        }
        // send the message to the channel
        privmsg(ch->name,desc.c_str());
      } // end of check for if command issued by valid user
    } // end of check for if we found a valid channel
  } // end of check for if we matched our text
} // end of function

extern "C" module *init() {
    module *m = new module("!country in channels", "Stuart Scott <stu@wilf.co.uk>", "0.1.0");
    m->hooks->privmsg = hook_privmsg;
    return m;
}

extern "C" void destroy() { }

