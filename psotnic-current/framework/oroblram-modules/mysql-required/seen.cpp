#include "includes/psotnic.h"
#include <string>
#include <algorithm>
#include <mysql.h>

/*

  Seen

  This module adds a !seen command.  You can set it to record everyone or just people on the bots, and respond to everyone
   or just people on the bots ;)

  Usage:
	!seen <nickname>        -- Will search for the nickname in its seen table
        !seen *!*@*		-- Will search for the hostmark in its seen table

*/
static MYSQL psotdb; 				 //  psotnic db object
static const char* DB_NAME = "psotnic";		//   psotnic db name
static const string DB_TABLE = "seen";		 //  seen table name
static const char* DB_USER = "psotnic";		//   db user
static const char* DB_PASS = "psotnic";	 	 //  db user's password
static const char* DB_HOST = "localhost";	//   db host
static const string FLD_NICK = "nick";		 //  nick field
static const string FLD_CHAN = "chan";		//   channel field
static const string FLD_HOST = "host";		 //  host field
static const string FLD_WHEN = "part";		//   when field (part in db due to 'when' being a reserved word in mysql)
static const int DB_PORT = 3306;		 //  db port
static const int MAX_NICK_LENGTH = 30;		//   maximum nick length in db
static const int MAX_CHAN_LENGTH = 30;		 //  maximum channel length in db
static const int MAX_HOST_LENGTH = 100;		//   maximum host length in db
static const int MAX_NICKS = 10;                 //  maximum number of nicks to return to a wildcard search
static const string QUIT_CHAN = "QUIT_CHAN";  	//   quit string
static const string ERROR = "SEEN_DB_ERROR"; 	 //  error string
static const string ERROR_TOO_MANY = "TOO_MANY";//   error string

/*
  getNick
   this function returns just the nickname from a full host string
   --
   host     :  the host to get the nick from
   --
   returns  :  the nickname part of the host string
*/
string getNick(string host) { return host.substr(0,host.find("!")); }

/*
  getRest
   this function returns everything after the nickname (minus the !) in a host string
   --
   host     :  the host to get the rest from
   --
   returns  :  everything after the nickname part of the host (minus the !)
*/
string getRest(string host) { return host.substr(host.find("!") + 1); }

/*
  newSeen
   this function handles when a user quits/parts
  --
  host   :  the hostmark of the parting/quitting user
  chan   :  the channel to log the seen to
  --
  returns   :  bugger all
*/
void newSeen (string Host,string Chan) {
  string host = Host; 	 	 // host string
  string chan = Chan;		//  channel string
  string query,nick;	 	 //  some strings
  MYSQL_RES *res; 	  	// our results
  MYSQL_ROW row; 		 //  our rows
  char hold[(MAX_HOST_LENGTH * 2) + (MAX_NICK_LENGTH * 2) + 2]; // our holder for an escaped host..
  // init the db
  mysql_init(&psotdb);
  // connect to the mysql db..
  if(mysql_real_connect(&psotdb, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)){
    // check if our host/chan are too long
    if ((host.length() < (MAX_HOST_LENGTH + MAX_NICK_LENGTH)) && (chan.length() < MAX_CHAN_LENGTH)) {
      // convert our host to lowercase
      transform (host.begin(),host.end(), host.begin(), tolower);
      // escape our host string
      mysql_real_escape_string(&psotdb,hold,host.c_str(),host.length());
      // store our host string
      host = hold;
      if (chan != QUIT_CHAN) { 
        transform (chan.begin(),chan.end(), chan.begin(), tolower);
        // escape our chan string
        mysql_real_escape_string(&psotdb,hold,chan.c_str(),chan.length());
        // store our chan string
        chan = hold;
      }
      // get our nickname from the host string
      nick = getNick(host);
      // strip out nickname from the host string
      host = getRest(host);
      // setup the query to replace/insert our record
      query = "REPLACE INTO " + DB_TABLE + " SET " + 
              FLD_NICK + " = '" + nick + "'," + 
              FLD_CHAN + " = '" + chan + "'," + 
              FLD_HOST + " = '" + host + "'";
      // execute the query
      mysql_real_query(&psotdb,query.c_str(),query.length());
      // check if we are inserting a quit or a channel part
      if (chan == QUIT_CHAN) { 
        // we have a quit, so setup the query to remove everything that isnt a quit for this nick
        query = "DELETE FROM " + DB_TABLE + " WHERE " + 
                FLD_CHAN + " != '" + QUIT_CHAN + "' AND " + 
                FLD_NICK + " = '" + nick + "'"; 
        // execute the query
        mysql_real_query(&psotdb,query.c_str(),query.length());
      }
      // we could do some checks here to make sure we did something, but then what would we do if it had failed? :P
    } // end of check if host is too long
  } // end of connection attempt
  // close the mysql connection
  mysql_close(&psotdb);
}

MYSQL_RES* runQuery(string query,my_ulonglong & rows) {
  MYSQL_RES *res;
  if ((mysql_real_query(&psotdb,query.c_str(),query.length()) == 0) && (mysql_errno(&psotdb) == 0)) {
    // see if we can get the results of our query
    if ((res = mysql_store_result(&psotdb))) { 
      // return how many rows we hit
      rows =  mysql_num_rows(res);
      // check if we hit anything this time
    } // end of check for storing results
  } // end of check for query
  return res;
}

/*
  getSeen
   this function returns the last seen date for the passed in nick/host pattern (if found)
   --
   host  :  the host to search for a last seen date for
   chan  :  the channel to check for seens in
   --
   returns  :  ERROR if the psotnic db isnt found/any other errors, "" if no last seen found or the last seen if there is one
*/
string getSeen(string Host,string Chan) {
  string ret = ERROR; 	 	 // our return string
  string host = Host;		//  our host string
  string chan = Chan; 		 // our chan string
  string nsign = "="; 		//  our connection sign (nick)
  string hsign = "="; 		 // our connection sign (host)
  string query,join,nick;	//  some strings
  MYSQL_RES *res; 		 // our resultset
  MYSQL_ROW row; 		//  our rows
  my_ulonglong rows; 		 // count of rows
  bool many = false; 		//  if we found many records or just the one
  char hold[(MAX_HOST_LENGTH * 2) + (MAX_NICK_LENGTH * 2) + 2]; // our holder for an escaped host..
  // initialise our connection
  mysql_init(&psotdb); 
  // try to connect to mysql db
  if(mysql_real_connect(&psotdb, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)){
    // lowercase our host+chan strings
    transform (host.begin(),host.end(), host.begin(), tolower);
    transform (chan.begin(),chan.end(), chan.begin(), tolower);
    // check if we have a full host string or just a nick
    if (host.find("!") == string::npos) {
      nick = host;
      host = "";
    } else { 
      nick = getNick(host);
      host = getRest(host); 
    }
    // convert *'s to %'s (*'s are usual wildcards, but mysql uses %'s)
    while (nick.find("*") != string::npos) { nick.replace(nick.find("*"),1,"%"); }
    while (host.find("*") != string::npos) { host.replace(host.find("*"),1,"%"); }
    // find out if we need to do a like or an =
    if (nick.find("%") != string::npos) { nsign = "like"; }
    if (host.find("%") != string::npos) { hsign = "like"; }
    // check if we have a long host/nick/channel or an ok one
    if ((host.length() < MAX_HOST_LENGTH) && (nick.length() < MAX_NICK_LENGTH) && (chan.length() < MAX_CHAN_LENGTH)) {
      // escape the nick string (incase of naughty people)
      mysql_real_escape_string(&psotdb,hold,nick.c_str(),nick.length());
      // store our lowercase host
      nick = hold;
      // escape the host string (incase of naughty people)
      mysql_real_escape_string(&psotdb,hold,host.c_str(),host.length());
      // store our lowercase host
      host = hold;
      // check if we have a host
      if (host != "") { host = "AND " + FLD_HOST + " " + hsign + " '" + host + "' "; }
      // build our query
      query = "SELECT " + FLD_WHEN + "," + FLD_NICK + "," + FLD_CHAN + " FROM " + DB_TABLE + " WHERE " + 
              FLD_NICK + " " + nsign + " '" + nick + "' AND " +
              FLD_CHAN + " = '" + chan + "' " + 
              host + 
              "ORDER BY " + FLD_NICK + "," + FLD_CHAN;
      // execute our query
      res = runQuery(query,rows);
      if (rows < 1) { 
        // we didnt hit anything, so we need to search for a general QUIT
        query = "SELECT " + FLD_WHEN + "," + FLD_NICK + "," + FLD_CHAN + " FROM " + DB_TABLE + " WHERE " + 
                FLD_NICK + " " + nsign + " '" + nick + "' AND " +
                FLD_CHAN + " = '" + QUIT_CHAN + "' " + 
                host + 
                "ORDER BY " + FLD_NICK + "," + FLD_CHAN;
        res = runQuery(query,rows);
        if (rows < 1) { 
          // we didnt hit anything, so we need to search for a general QUIT
          query = "SELECT " + FLD_WHEN + "," + FLD_NICK + "," + FLD_CHAN + " FROM " + DB_TABLE + " WHERE " +
                  FLD_NICK + " " + nsign + " '" + nick + "' " + 
                  host + 
                  "ORDER BY " + FLD_NICK + "," + FLD_CHAN;
          // this is our last chance, so we dont car if we hit or not at this point
          res = runQuery(query,rows);
        } // end of second check for hitting anything
      } // end of first check to see if we hit any rows
      // check if we hit ANY rows
      if (rows < 1) { ret = ""; }
      else if (rows > MAX_NICKS) { 
        // we hit too many nicks, ask them to be more specific
        ret = ERROR_TOO_MANY; 
      } else {
        // clear out return, ready for new data
        ret = "";
        // set our boolean for holding if we hit many or 1 row
        many = (rows > 1); 
        // loop through the returned rows
        while ((row = mysql_fetch_row(res))) { 
          // if we have many then get the nick and put it in our return string, else get the timestamp
          if (many) { ret = ret + join + row[1]; join = ","; }
          else { ret = row[1] + string("!") + row[0] + string("!") + row[2]; }
        } // end of loop through rows
      } // end of third check to see if we hit any rows
      // release the memory used for our results
      mysql_free_result(res);
    } // end of the check for if the host is too long
  } // end of the connection attempt
  // close the mysql connection
  mysql_close(&psotdb);
  // return
  return ret;
}

/*
  convertTimestamp
   this function returns a converted timestamp from 'YYYY-MM-DD HH:MM:SS' to 'DD/MM/YYYY at HH:MM:SS GMT'
   --
   st  :  the timestamp to convert
   --
   returns  :  the timestamp reformated for humans
*/
string convertTimestamp(string ts) { 
  string stamp,time; // some strings
  // get the date part of the timestamp
  stamp = ts.substr(0,10); 
  // switch around the fields in the date
  stamp = stamp.substr(8,2) + "/" + stamp.substr(5,2) + "/" + stamp.substr(0,4);
  // get the time part of the timestamp
  time = ts.substr(11);
  // return our nicely formatted string
  return stamp + " at " + time + " GMT";
}

void hook_privmsg(const char *from, const char *to, const char *msg) {
  // Check if we match any of our keywords
  if (match("!seen *",msg)) {
    /*
      Set this to true if you want !seen to be public, i.e. open to ALL people on a channel
       this isnt recommended, as it gives a means for someone flooding your bot off (I havent written
       flood protection into this function as for me its restricted to known users only)
    */
    bool pub = true;
    string channame,host,seen; // some strings ;)
    char arg[75][MAX_LEN]; // arguments
    int pos; // position holder
    chan *ch = findChannel(to);// channel we are acting in
    string chan; // channel holder
    // check if we have a channel
    if(ch) {
      // get the name
      channame = string(ch->name);
      // get the user who is asking for the country
      chanuser *u = findUser(from, ch);
      // check if the user is a known one (to stop lamers flooding us off with requests)
      if((u && u->flags & (IS_VOICE | IS_OP)) || pub) {
        // break up the line
        str2words(arg[0], msg, 2, MAX_LEN, 0);
        // get the word
        host = arg[1];
        // check if they are on channel now
        if (findUser(host.c_str(),ch)) { 
          // they are on channel..
          seen = host + " is here right now, you idiot.";
        } else {
          // get the seen
          seen = getSeen(host,channame);
          // check if we hit it
          if (seen == ERROR) {
            // we had an error, tell them
            seen = "Error, database connection went wrong.";
          } else if (seen == ERROR_TOO_MANY) {
            // we got too many nicks hit
            seen = "Too many users were matched, please be more specific.";
          } else if (seen == "") {
            // we didnt find a seen record for the user
            seen = "I don't remember when I last saw " + host + ", maybe I never have.";
          } else if (seen.find(",") != string::npos) {
            // we matched more than one person
            seen = "The following nicknames matched your search, please search again using one of these: " + seen; 
          } else if ((host.find("*") == string::npos) && (host.find("%") == string::npos)) { 
            // we have a single match where we didnt send wildcards
            // get the channel
            chan = getRest(getRest(seen));
            // check if it was a quit or a part
            if (chan == QUIT_CHAN) { 
              seen = "I last saw " + host + " quitting IRC on " + convertTimestamp(getNick(getRest(seen)));
            } else {
              seen = "I last saw " + host + " on " + convertTimestamp(getNick(getRest(seen))) + " leaving " + chan;
            } // end of check for quit or part
          } else {
            // we sent a wildcarded string and got 1 match back
            // get the channel
            chan = getRest(getRest(seen));
            // check if it was a quit or a part
            if (chan == QUIT_CHAN) {
              seen = "I last saw nick " + getNick(seen) + " (matching " + host + ") quitting IRC on " + 
                     convertTimestamp(getNick(getRest(seen)));   
            } else {
              seen = "I last saw nick " + getNick(seen) + " (matching " + host + ") on " + 
                     convertTimestamp(getNick(getRest(seen))) + " in " + chan;
            } // end of check for quit or part
          } // end of check if we hit
        }
        // send the message to the channel
        privmsg(ch->name,seen.c_str());
      } // end of check if we have a valid user (or are pub)
    } // end of check for if we found a valid channel
  } // end of check for if we matched our text
} // end of function

void hook_post_part(const char *mask, const char *channel) { newSeen(string(mask),string(channel)); }
void hook_kick(chan *ch, chanuser *kicked, chanuser *kicker) {
  // check if we have a valid kicked user, if so call a newSeen for them
  if (kicked && ch) { 
    newSeen(string(string(kicked->nick) + "!" + string(kicked->ident) + "@" + string(kicked->host)),string(ch->name)); 
  }
}

void hook_rawirc(const char *data) {
  string dat = data; // our data string
  char arg[2][MAX_LEN]; // our arguments
  // get our broken up words from the data
  str2words(arg[0], data, 2, MAX_LEN, 0); 
  // check if we had a quit, if so call a newSeen on it
  if (strcmp(arg[1],"QUIT") == 0) { 
    newSeen(string(arg[0]).substr(1),QUIT_CHAN); 
  }
}

extern "C" module *init() {
    module *m = new module("!seen in channels", "Stuart Scott <stu@wilf.co.uk>", "0.1.0");
    // setup our hooks
    m->hooks->privmsg = hook_privmsg;
    m->hooks->kick = hook_kick;
    m->hooks->post_part = hook_post_part;
    m->hooks->rawirc = hook_rawirc;
    return m;
}

extern "C" void destroy() { }

