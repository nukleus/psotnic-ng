#include "includes/psotnic.h"
#include <string>
#include <algorithm>
#include <mysql.h>

/*

  Explain

  This module adds the ability to explain words that have been predefined by other users

  Usage:
	!explain <word>        		--  Will 'explain' the word
        !expadd <word> <exp>   		--  Will add the explanation of word as exp
        !expdel <word>         		--  Will remove the explanation of word
        !explains/!explist/!explain	--  Will list all the words with explanations

*/

static MYSQL psotdb;                              //  psotnic db object
static const char* DB_NAME = "psotnic";          //   psotnic db name
static const string DB_TABLE = "explains";        //  seen table name
static const char* DB_USER = "psotnic";          //   db user
static const char* DB_PASS = "psotnic";           //  db user's password
static const char* DB_HOST = "localhost";        //   db host
static const string FLD_WORD = "word";            //  word field
static const string FLD_WHO = "who";             //   who field
static const string FLD_CHAN = "chan";            //  channel field
static const string FLD_ADDED = "added";         //   added field
static const string FLD_DETAIL = "detail";        //  detail field
static const int DB_PORT = 3306;                 //   db port
static const int MAX_WORD_LENGTH = 30;            //  maximum word length in db
static const int MAX_WHO_LENGTH = 30;            //   maximum user length in db
static const int MAX_CHAN_LENGTH = 30;            //  maximum channel length in db
static const int MAX_DESC = 400;		 //   maximum description length
static const string ERROR = "EXPLAINS_DB_ERROR";  //  error string

/*
  escapeString
   this functions escapes the passed in string ready for mysql usage
   --
   str     :   the string to escape
   max     :   the maximum length of our string
   --
   returns :   the escaped string
*/
string escapeString(string str,int max) {
  char hold[(max * 2) + 2]; // our holder for an escaped string..
  // check our length (hoho)
  if (str.length() > max) { str = str.substr(0,max); }
  // escape our string
  mysql_real_escape_string(&psotdb,hold,str.c_str(),str.length());
  // return
  return string(hold);
}


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
  runQuery
   this function runs the query and returns a resultset
   --
   query    :  the query to execute
   rows     :  the number of rows retreived | if its an update, then the error number
   update   :  if its an update statement or a select
   --
   returns  :  the resultset (if any) from the query
*/
MYSQL_RES* runQuery(string query,my_ulonglong & rows,bool update) {
  MYSQL_RES *res;
  rows = 0;
  if ((mysql_real_query(&psotdb,query.c_str(),query.length()) == 0) && (mysql_errno(&psotdb) == 0)) {
    // see if we can get the results of our query
    if ((res = mysql_store_result(&psotdb))) {
      // return how many rows we hit
      rows =  mysql_num_rows(res);
      if ((rows == 0) && update) { rows = mysql_errno(&psotdb); }
      // check if we hit anything this time
    } // end of check for storing results
  } // end of check for query
  return res;
}

/*
  convertTimestamp
   this function returns a converted timestamp from 'YYYY-MM-DD HH:MM:SS' to 'DD/MM/YYYY'
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
  // return our nicely formatted string
  return stamp;
}


/*
  delDesc
   this function removes the passed in word from the db
   --
   word  : the word to remove
   chan  : the channel to remove it from
   --
   returns  :  0 if it worked, error number if it didnt
*/
int delDesc(string Word,string Chan) {
  string word = Word;
  string chan = Chan;
  string query;
  my_ulonglong rows;             // count of rows

  // make the word and chan lowercase
  transform (word.begin(),word.end(), word.begin(), tolower);
  transform (chan.begin(),chan.end(), chan.begin(), tolower);

  // escape them both
  word = escapeString(word,MAX_WORD_LENGTH);
  chan = escapeString(chan,MAX_CHAN_LENGTH);

  // build the query
  query = "DELETE FROM " + DB_TABLE + " WHERE " + FLD_WORD + "='" + word + "' AND " + FLD_CHAN + "='" + chan + "'";

  // execute our query
  runQuery(query,rows,true);

  // return
  return rows;
}

/*
  setDesc
   this function adds/overwrites the passed in word with its passed in explanation, as well as who is explaining it
   --
   word  :  the word to explain
   chan  :  the channel to explain it in
   desc  :  the explanation
   who   :  the person doing the explaining
   --
   returns  :  0 if it worked or error number if it didnt
*/
int setDesc(string Word,string Chan,string desc,string who) {
  string word = Word;
  string chan = Chan;
  string query; // the query
  my_ulonglong rows;             // count of rows

  // make the word and chan lowercase
  transform (word.begin(),word.end(), word.begin(), tolower);
  transform (chan.begin(),chan.end(), chan.begin(), tolower);

  // escape them all
  word = escapeString(word,MAX_WORD_LENGTH);
  chan = escapeString(chan,MAX_CHAN_LENGTH);
  desc = escapeString(desc,MAX_DESC);
  who = escapeString(who,MAX_WHO_LENGTH);

  // setup the query
  query = "REPLACE INTO " + DB_TABLE + " SET " + 
          FLD_WORD + "='" + word + "'," + 
          FLD_CHAN + "='" + chan + "'," + 
          FLD_WHO + "='" + who + "'," + 
          FLD_DETAIL + "='" + desc + "'";
  // execute our query
  runQuery(query,rows,true);

  // return
  return rows;
}

/*
  getDesc
   this function returns the description for the passed in word (if found)
   --
   word  :  the word to search for a description for
   chan  :  the channel to search in
   desc  :  the description found
   who   :  who explained the word
   when  :  when the word was explained
   --
   returns  :  true if found, false if not
*/
bool getDesc(string Word,string Chan,string &desc,string &who,string &when) {
  string word = Word; // the word to search for
  string chan = Chan; // the channel to search in
  bool ret = false; // if we found our word
  string query; // the query
  my_ulonglong rows;             // count of rows

  MYSQL_RES *res; // our resultset
  MYSQL_ROW row; //  our rows

  // make the word and chan lowercase
  transform (word.begin(),word.end(), word.begin(), tolower);
  transform (chan.begin(),chan.end(), chan.begin(), tolower);

  // escape them both
  word = escapeString(word,MAX_WORD_LENGTH);
  chan = escapeString(chan,MAX_CHAN_LENGTH);

  // setup the query
  query = "SELECT " + FLD_DETAIL + "," + FLD_WHO + "," + FLD_ADDED + " FROM " + DB_TABLE + " WHERE " + 
          FLD_WORD + "='" + word + "' AND " + FLD_CHAN + "='" + chan + "'";

  // execute our query
  res = runQuery(query,rows,false);

  // check how many rows we got
  if (rows > 1) {
    // we got more than one row, something went wrong with our query ;)
    desc = ERROR;
  } else if (rows < 1) {
    // we didnt get any rows, so word not matched..
    desc = "";
  } else {
    // we matched 1 row, try and get it (else return error)
    if ((row = mysql_fetch_row(res))) { 
      // everything worked, get the data to return
      desc = row[0];
      who = row[1];
      when = row[2];   
      ret = true;
    } else { desc = ERROR; }
  }

  // return
  return ret;
}

/*
  allDesc
   this function returns a comma seperated list of all the words in the db
   --
   chan  :  the channel to search in
   --
   returns  :  "" if nothing found, ERROR if an error occured or a comma seperated list of the words found
*/
string allDesc(string Chan) {
  string chan = Chan; // the channel to search in
  string ret = ERROR; // our return
  string query,join; // the query and join
  my_ulonglong rows;             // count of rows
  MYSQL_RES *res; // our resultset
  MYSQL_ROW row; //  our rows

  // make the chan lowercase
  transform (chan.begin(),chan.end(), chan.begin(), tolower);

  // escape the chan
  chan = escapeString(chan,MAX_CHAN_LENGTH);

  // setup the query
  query = "SELECT " + FLD_WORD + " FROM " + DB_TABLE + " WHERE " + FLD_CHAN + "='" + chan + "' ORDER BY " + FLD_WORD;

  // execute our query
  res = runQuery(query,rows,false);

  // check how many rows we got
  if (rows < 1) {
    // we didnt get any rows, so we found no words to explain
    ret = "";
  } else {
    // we found words
    ret = "";
    // loop through the words
    while ((row = mysql_fetch_row(res))) {
      // append current word to list
      ret = ret + join + row[0];
      join = ",";
    } 
  }

  // return
  return ret;
}

void hook_privmsg(const char *from, const char *to, const char *msg) {
  // Check if we match any of our keywords
  if (match("!exp*",msg)) {
    /*
      Change this to true if you want !explain to be public, i.e. open to ALL people on a channel
       this isnt recommended, as it gives a means for someone flooding your bot off (I havent written
       flood protection into this function as for me its restricted to known users only)
    */
    bool pub = false;
    string channame,desc,word,who,when; // some strings
    char arg[75][MAX_LEN]; // arguments
    int pos; // position holder
    int back = 0; // holder for returns
    chan *ch = findChannel(to);// channel we are acting in
    // check if we have a channel
    if(ch) {
      // get the name
      channame = string(ch->name);
      // get the user who is asking for the country
      chanuser *u = findUser(from, ch);
      // check if the user is a known one (to stop lamers flooding us off with requests)
      if((u && u->flags & (IS_VOICE | IS_OP)) || pub) {
        // init the db
        mysql_init(&psotdb);
        // connect
        if(mysql_real_connect(&psotdb, DB_HOST, DB_USER, DB_PASS, DB_NAME, DB_PORT, NULL, 0)){
          // see what we have been asked to do
          if (match("!explain *",msg)) {
            // break up the line
              str2words(arg[0], msg, 2, MAX_LEN, 0);
            // get the word
            word = arg[1];
            // get the description
            if (getDesc(word,channame,desc,who,when)) { 
              desc = word + ": " + desc + " <" + who + " - " + convertTimestamp(when) + ">";
            } else if (desc == ERROR) {
              desc = "Error, something went wrong with the database.";
            } else {
              desc = word + ": unexplainable."; 
            }
            // send the message to the channel
            privmsg(ch->name,desc.c_str());
          } else if ((match("!explains*",msg)) || (match("!explain*",msg)) || (match("!explist*",msg))) {
            // get the words
            desc = allDesc(channame);
            // check what we got
            if (desc == ERROR) {
              desc = "Error, explain file is missing/bad. (read)";
            } else if (desc == "") {
              desc = "I can't explain anything at the moment.";
            } else {
              desc = "I can currently explain: " + desc;
            }
            // send the message to the channel
            privmsg(ch->name,desc.c_str());
          } else if ((match("!expadd *",msg)) && (u->flags & HAS_O)) {
            word = msg;
            word = word.substr(8);
            pos = word.find(" ",0);
            // check if we have a word and a description
            if (pos < 1) { 
              privmsg(ch->name,"That doesn't look like a proper word, try again.");
            } else {
              desc = word.substr(pos+1); 
              word = word.substr(0,pos);
              if (desc.size() > MAX_DESC) {
                privmsg(ch->name,"Too long, try to be a little more succinct.");
              } else {
                // try to add it
                back = setDesc(word,channame,desc,u->nick);
                if (back == 0) { 
                  desc = "Explanation updated.";
                } else {
                  desc = "Error, something went wrong with the db(" + string(itoa(back)) + ")";
                }
                // tell the user what happened
                privmsg(ch->name,desc.c_str());
              } // end of check for desc size
            } // end of check for position of space
          } else if ((match("!expdel *",msg)) && (u->flags & HAS_O)) {
            // break up the line
            str2words(arg[0], msg, 2, MAX_LEN, 0);
            // get the word
            word = arg[1];
            // try to delete it
            back = delDesc(word,channame);
            // check what happened
            if (back == 0) { 
              desc = "Word deleted.";
            } else {
              desc = "Word not found or couldn't be deleted(" + string(itoa(back)) + ")";
            } 
            // send the message
            privmsg(ch->name,desc.c_str());
          } // end of match
          // close the mysql connection
          mysql_close(&psotdb);
        } else {
          privmsg(ch->name,"DB Failed to connect.");
        } // end of check for db connection
      } // end of check if we have a valid user (or are pub)
    } // end of check for if we found a valid channel
  } // end of check for if we matched our text
} // end of function

extern "C" module *init() {
    module *m = new module("!country in channels", "Stuart Scott <stu@wilf.co.uk>", "0.1.0");
    m->hooks->privmsg = hook_privmsg;
    return m;
}

extern "C" void destroy() { }

