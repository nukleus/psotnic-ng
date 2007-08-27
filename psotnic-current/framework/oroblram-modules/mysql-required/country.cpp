#include "includes/psotnic.h"
#include <string>
#include <algorithm>
#include <mysql.h>

/*

  Country

  This module adds a !country command that lists the country info 

  Usage:
	!country <1 char ISO code>      --  Will list the country that matches this 2 character ISO code
        !country .<TLD>                 --  Will list the country that matches the given top-level domain code
        !country <Country name>         --  Will list the country that matches the given string (can use wildcards)

*/

static MYSQL psotdb;                              //  psotnic db object
static const char* DB_NAME = "psotnic";          //   psotnic db name
static const string DB_TABLE = "country";         //  seen table name
static const char* DB_USER = "psotnic";          //   db user
static const char* DB_PASS = "psotnic";           //  db user's password
static const char* DB_HOST = "localhost";        //   db host
static const string FLD_ISO = "iso";              //  iso field
static const string FLD_TLD = "tld";             //   tld field
static const string FLD_NAME = "name";            //  country name field
static const int MAX_ISO = 2;                    //   the maximum size of the iso
static const int MAX_TLD = 10;                    //  the maximum size of the tld
static const int DB_PORT = 3306;                 //   db port
static const int MAX_RETURNS = 10;		  //  maximum number of records to return in 1 go
static const string ERROR = "COUNTRY_DB_ERROR";  //   error string
static const string TOOMANY = "TOOMANY_RETURNED"; //  error string for when too many are returned

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
  // make it lowercase
  transform (str.begin(),str.end(), str.begin(), tolower);
  // escape our string
  mysql_real_escape_string(&psotdb,hold,str.c_str(),str.length());
  // return
  return string(hold);
}

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
    } else if (update) { rows = mysql_errno(&psotdb); } // end of check for storing results
  } else if (update) { rows = mysql_errno(&psotdb); } // end of check for query
  return res;
}

/*
  getString
   this function returns the passed in string if it exists, or "<None>" if it doesnt
   --
   str : the string to check
   --
   returns : str if not "", else "<None>"
*/
string getString(const char *str) { if (str == NULL) { return "<None>"; } else { return string(str); } } 

/*
  getCountryDetails
   this function collects the country details from the passed in query
   --
   query :  the query to use
   iso   :  the ISO to set
   tld   :  the TLD to set
   name  :  the country name to set
   --
   returns : the formated info
*/
string getCountryDetails(string query,bool many) {
  string ret = ""; // the return string
  my_ulonglong rows; // count of rows
  MYSQL_RES *res; // our resultset
  MYSQL_ROW row; //  our rows
  string join; // joining string

  // execute our query
  res = runQuery(query,rows,false);

  // check how many rows we got
  if ((rows > 1) && (!many)) {
    // we got more than one row, something went wrong with our query ;)
    ret = ERROR;
  } else if (rows > MAX_RETURNS) { 
    // we got too many
    ret = TOOMANY;
  } else if (rows < 1) {
    // we didnt get any rows, so word not matched..
    ret = "";
  } else {
    // set the name to error
    ret = ERROR;
    // try and get data 
    while ((row = mysql_fetch_row(res))) {
      // check to reset ret
      if (join == "") { ret = ""; }
      // everything worked, get the data to return
      ret = ret + join + getString(row[2]) + "(" + getString(row[0]) + ",." + getString(row[1]) + ")";
      join = ",";
    } 
  }

  // return
  return ret;
}

/*
  getCountryFromISO
   this function returns the full description for a country from an ISO
   --
   iso   :  the ISO
   --
   returns  :  formated string reply
*/
string getCountryFromISO(string ISO) {
  string iso = ISO; // the ISO to search for
  bool ret = false; // if we found our word
  string query; // the query

  // escape it
  iso = escapeString(iso,MAX_ISO);

  // setup the query
  query = "SELECT " + FLD_ISO + "," + FLD_TLD + "," + FLD_NAME + " FROM " + DB_TABLE + " WHERE " + 
          FLD_ISO + "='" + iso + "'";

  // return
  return getCountryDetails(query,false);
}

/*
  getCountryFromTLD
   this function returns the full description for a country from an TLD
   --
   tld   :  the TLD
   --
   returns  :  formated string reply
*/
string getCountryFromTLD(string TLD) {
  string tld = TLD; // the TLD to search for
  bool ret = false; // if we found our word
  string query; // the query

  // escape it
  tld = escapeString(tld,MAX_TLD);

  // setup the query
  query = "SELECT " + FLD_ISO + "," + FLD_TLD + "," + FLD_NAME + " FROM " + DB_TABLE + " WHERE " +
          FLD_TLD + "='" + tld + "'";

  // return
  return getCountryDetails(query,false);
}

/*
  getCountryFromName
   this function returns the full description for a country from a name (wildcards allowed)
   --
   name  :  the country
   --
   returns  :  formated string reply
*/
string getCountryFromName(string Name) {
  string name = Name;
  bool ret = false; // if we found our word
  string sign = " = "; // the sign to use for searching on name
  string query; // the query

  // convert *'s to %'s (*'s are usual wildcards, but mysql uses %'s)
  while (name.find("*") != string::npos) { name.replace(name.find("*"),1,"%"); }

  // find out if we need to do a like or an =
  if (name.find("%") != string::npos) { sign = " like "; }

  // escape it
  name = escapeString(name,50);

  // setup the query
  query = "SELECT " + FLD_ISO + "," + FLD_TLD + "," + FLD_NAME + " FROM " + DB_TABLE + " WHERE lower(" +
          FLD_NAME + ")" + sign + "lower('" + name + "')";

  // return
  return getCountryDetails(query,true);
}

void hook_privmsg(const char *from, const char *to, const char *msg) {
  // Check if we match any of our keywords
  if (match("!country *",msg)) {
    /*
      Change this to true if you want !country to be public, i.e. open to ALL people on a channel
       this isnt recommended, as it gives a means for someone flooding your bot off (I havent written
       flood protection into this function as for me its restricted to known users only)
    */
    bool pub = false;
    string channame,output,arg; // some strings
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
          // get the argument
          arg = string(msg).substr(9); 
          // check what we are searching for
          if ((arg.find(".") == 0) && (arg.length() <= (MAX_TLD + 1))) { 
            // TLD search
            arg = arg.substr(1);
            output = getCountryFromTLD(arg);
            // check if we matched nothing, and if we did then search for an ISO instead IF WE HAVE A 2 CHAR TLD
            if ((output == "") && (arg.length() <= MAX_ISO)) { output = getCountryFromISO(arg); }
          } else if (arg.length() == MAX_ISO) { 
            // ISO search
            output = getCountryFromISO(arg);
            // check if we matched nothing, and if we did then search for a TLD instead
            if (output == "") { output = getCountryFromTLD(arg); } 
          } else {
            // country name search
            output = getCountryFromName(arg);
          }
          // check what we got
          if (output == ERROR) { 
            // we had a standard error
            output = "There was an error.";
          } else if (output == TOOMANY) {
            // we found too many countries
            output = "Too many countries matched your search, please try to be more specific.";
          } else if (output == "") { 
            // we didnt find any countries
            output = "Your search didn't match any countries.";
          } else {
            // we found 1/some countries
            output = "Your search matched the following: " + output; 
          }
          // send the message to the channel
          privmsg(ch->name,output.c_str());
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

