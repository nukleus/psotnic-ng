/***************************************************************************
 *   Copyright (C) 2003-2005 by Grzegorz Rusin                             *
 *   grusin@gmail.com                                                      *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include "prots.h"
#include "global-var.h"

http::url::url(const char *u)
{
	memset(this, 0, sizeof(url));

	if(!u) return;
	if(strncmp("http://", u, 7)) return;
	mem_strcpy(link, u);
	u += 7;

	char *a = strchr(u, '/');

	if(a)
	{
		mem_strncpy(host, u, abs(a-u)+1);
		mem_strcpy(filepath, a);
		char *b = strrchr(filepath, '/');
		if(b && *b && strlen(b+1))
			mem_strcpy(file, b+1);
	}
	else
	{
		mem_strcpy(host, u);
		mem_strcpy(filepath, "/");
	}

	char buf[MAX_LEN];

	if(inet::gethostbyname(host, buf))
		mem_strcpy(ip, buf);

	port = 80;
}

http::url::~url()
{
	if(host) free(host);
	if(ip) free(ip);
	if(file) free(file);
	if(filepath) free(filepath);
	if(link) free(link);
}

int http::url::ok()
{
	if(ip) return 1;
	else return 0;
}

#ifdef HAVE_DEBUG
void http::url::print()
{
	printf("link:     %s\n", link);
	printf("host:     %s\n", host);
	printf("ip:       %s\n", ip);
	printf("filepath: %s\n", filepath);
	printf("file:     %s\n", file);
}
#endif

http::http()
{
	data = NULL;
}

http::~http()
{
	if(data) free(data);
}

int http::get(const char *link, int estSize)
{
	url u(link);
	return get(u, estSize);
}

int http::get(const char *link, const char *file)
{
	url u(link);
	return get(u, file);
}

int http::get(url &u, int estSize)
{
	int ret;

	if((ret = sendRequest(u)))
		return ret;

	if((ret = processHeader()) != -200)
		return ret;

	char buf[MAX_LEN];
	int len;

	Pchar d(estSize);

	while(1)
	{
		len = read(www.fd, buf, MAX_LEN);
		if(!len)
		{
			data = d.data;
			d.data = NULL;
			return d.len;
		}
		else if(len > 0)
			d.push(buf, len);
		else
		{
			www.close();
			return ERR_READ;
		}
	}
}

int http::get(url &u, const char *file)
{
	int fd;

	if((fd = open(file, O_CREAT | O_TRUNC | O_RDWR, S_IRWXU)) < 1)
		return ERR_DST;

	int ret;

	if((ret = sendRequest(u)))
		return ret;

	if((ret = processHeader()) != -200)
		return ret;

	int len, total=0;
	int n;
	char buf[MAX_LEN];

	while(1)
	{
		len = read(www.fd, buf, MAX_LEN);
		
		if(!len)
		{
			www.close();
			close(fd);
			return total;
		}
		else if(len > 0)
		{
			if((n = write(fd, buf, len)) != len)
			{
				www.close();
				close(fd);
				return ERR_DST_WRITE;
			}
			total += len;
		}
		else //len < 0
		{
			www.close();
			close(fd);
			return ERR_READ;
		}
	}
}

char *http::error(int n)
{
	static char err_404[] = "404: File not found";
	static char err_403[] = "403: Access Denied";
	static char err_conn[] = "Cannot establish connection";
	static char err_header[] = "Invalid HTTP header";
	static char err_eof[] = "EOF from client";
	static char err_read[] = "Read error";
	static char err_url[] = "Invalid URL";
	static char err_dst[] = "Cannot open destination file";
	static char err_unknown[] = "Unknown error";

	switch(n)
	{
		case ERR_EOF: return err_eof;
		case ERR_CONN: return err_conn;
		case ERR_URL: return err_url;
		case ERR_DST: return err_dst;
		case ERR_HEADER: return err_header;
		case ERR_READ: return err_read;
		case ERR_OK: return NULL;
		case -200: return NULL;
		case -404: return err_404;
		case -403: return err_403;

		default: return err_unknown;
	}
}

int http::sendRequest(url &u)
{
	if(!u.ok())
		return ERR_URL;

	www.fd = doConnect(u.ip, u.port, "", 0);

	if(www.fd > 0)
	{
		www.send("GET ", u.filepath, " HTTP/1.0", NULL);
		www.send("User-Agent: Mozilla/5.0 (compatible; ", getFullVersionString(), ")", NULL);
		www.send("Host: ", u.host, ":", itoa(u.port), NULL);
		www.send("\r\n");
		return 0;
	}
	return ERR_CONN;
}


int http::processHeader()
{
	char buf[MAX_LEN], tmp[MAX_LEN];
	int len, ret = 0, ok;

	while(1)
	{
		len = www.readln(buf, MAX_LEN, &ok);

		if(len == -1)
		{
			www.close();
			return ERR_READ;
		}

		if(ok)
		{
			if(len)
			{
				//printf("len: %d :: %s\n", len, buf);
				if(!ret)
				{
					if(len < MAX_LEN)
						sscanf(buf, "%s %d", tmp, &ret);
					else return ERR_HEADER;

					if(ret != 200) return -ret;
				}
			}
			else
				return -200;
		}
	}
}
