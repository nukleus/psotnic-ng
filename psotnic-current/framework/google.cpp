/********************************************************
* Psotnic google module                                *
* Copyright (c) 2005 matrix <admin@areaunix.org>       *
* Created: 07-05-2005 by matrix@IRCNet                 *
* Usage: !google word in chan                          *
********************************************************/

#include "includes/psotnic.h"
#include <iostream>
#include <string>

#define DEST_HOST   "google.com"
#define DEST_PORT 80
#define MAXDATASIZE 1000

using namespace std;

void hook_privmsg(const char *from, const char *to, const char *msg)
{
    if(match("!google*", msg))
    {
	int fd;
	int check = 0;
        string url = "";
        string page = "";
        string question = string(msg).erase(0,8); //delete '!google '
        while (question.find(" ") != string::npos) question[question.find(" ")] = '+'; 
	//replace spaces with +
	string query = "GET /search?q="; //creating url
	query += question;
        query += " HTTP/1.0\n\n";

        //create socket
        char buf[MAXDATASIZE];
        int bytes_sent, bytes_recv;
        struct hostent *he;
        struct sockaddr_in dest_addr;
        he=gethostbyname(DEST_HOST);
        fd = socket(AF_INET, SOCK_STREAM, 0);
        dest_addr.sin_family = AF_INET;
        dest_addr.sin_port = htons(DEST_PORT);
        dest_addr.sin_addr = *((struct in_addr *)he->h_addr);
        memset(&(dest_addr.sin_zero), '\0', 8 );

        connect(fd, (struct sockaddr *)&dest_addr, sizeof(struct sockaddr)); //connect
        bytes_sent = send(fd, query.c_str(), strlen(query.c_str()), 0); //send request
        while ((bytes_recv = recv(fd, buf, MAXDATASIZE-1, 0)) > 0) { //receive answer
    	    buf[bytes_recv] = '\0';
    	    page += buf;
	}
	close(fd); //close file descriptor
        while (page.find("href=") != string::npos) {    //find all the urls
    	    page.erase(0,page.find("href=") + 5);
            url.assign(page,0,page.find_first_of(">"));
    	    if (check != 1 && (url.find_first_of("\"") != 0) && 
		(url.find_first_of("/") != 0) && (url.find("translate")) == string::npos) { //check if is a 
		    //google url and if one url already sent to channel
                    privmsg(to,url.c_str());
                    check = 1;
            }
        }
    }
}

extern "C" module *init()
{
   module *m = new module("!google public command", "matrix <admin@areaunix.org>", "0.1.0");
   m->hooks->privmsg = hook_privmsg;
   return m;
}

extern "C" void destroy()
{
}
