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

/*
 * This file is based upon socks5.c by Josh A. Beam
 * Copyright (C) 2002 Josh A. Beam
 * All rights reserved.
 */


#include "prots.h"
#include "global-var.h"

char *username = NULL;
char *password = NULL;

/* algorythm:
	00 - nothing to do.
	01 - syn sent.
	02 - connected to proxy (send type of login).
	03 - read 1st byte of welcome string.
	04 - read 2nd byte of welcome string (optionaly: send username & pass).
	05 - read 1st byte of auth reply.
	06 - read 2nd byte of auth reply (send remote host & ip).
	07 - read 1st byte of connect reply.
	08 - read 2nd byte of connect reply.
	09 - read dummy byte.
	10 - read type reply.

	if(type == 0x01) read 6 dummy bytes.
	if(type == 0x03) read 1 byte + value of that byte + 2 bytes.

	return fd.
*/

int asyn_socks5::work(char byte)
{
	switch(step)
	{
		case 1:
		{
			//printf("step %d\n", step);
			++step;
			//fall trough
		}
		case 2:
		{
			//printf("step %d\n", step);
			buf[0] = 0x05;
    		buf[1] = 0x01;
    		if(username && password)
        		buf[2] = 0x02;
    		else
				buf[2] = 0x00;

    		send(fd, buf, 3, 0);
			++step;
			break;
		}
		case 3:
		{
			//printf("step %d\n", step);
			if(byte != 0x05)
			{
				net.send(HAS_N, "[-] Bad SOCKS5 server response (invalid welcome string?)", NULL);
				step = 0;
				return -3;
			}
			if(username && password)
				++step;
			else step = 7;
			break;
		}
		case 4:
		{
			//printf("step %d\n", step);
			if(byte != buf[2])
			{
				net.send(HAS_N, "[-] Bad SOCKS5 server response (invalid welcome string?)", NULL);
				step = 0;
				return -4;
			}
			++step;
			//fall trough
		}
		case 5:
		{
			//printf("step %d\n", step);
			unsigned char tmplen;

        	buf[0] = 0x01;
        	len = (strlen(username) > 255) ? 255 : strlen(username);
        	buf[1] = len;
        	memcpy(buf + 2, username, len);

        	tmplen = (strlen(password) > 255) ? 255 : strlen(password);
        	buf[2 + len] = tmplen;
        	memcpy(buf + 3 + len, password, tmplen);

        	send(fd, buf, (3 + len + tmplen), 0);
			++step;
			break;
		}
		case 6:
		{
			//printf("step %d\n", step);
			if(byte != 0x01)
			{
				net.send(HAS_N, "[-] SOCKS5 authentication failed", NULL);
				step = 0;
				return -6;
			}
			++step;
			break;
		}
		case 7:
		{
			//printf("step %d\n", step);
			if(byte != 0x00)
			{
				net.send(HAS_N, "[-] SOCKS5 authentication failed", NULL);
				step = 0;
				return -7;
			}
			++step;
			//fall trough
		}
		case 8:
		{
			net.send(HAS_N, "[+] Connected to SOCKS5 server ", proxyip, ":", itoa(proxyport), NULL);
			//printf("step %d\n", step);
			buf[0] = 0x05;
    		buf[1] = 0x01;
    		buf[2] = 0x00;
    		buf[3] = 0x03;
    		len = (strlen(remotehost) > 255) ? 255 : strlen(remotehost);
    		buf[4] = (len & 0xff);
    		memcpy(buf + 5, remotehost, len);
    		buf[5 + len] = (remoteport >> 8);
    		buf[6 + len] = (remoteport & 0xff);
    		send(fd, buf, (7 + len), 0);
			++step;
			break;
		}
		case 9:
		{
			//printf("step %d\n", step);
			if(byte != 0x05)
			{
				net.send(HAS_N, "[-] Bad SOCKS5 server response (invalid remote addr?)", NULL);
				step = 0;
				return -9;
			}
			++step;
			break;
		}
		case 10:
		{
			//printf("step %d\n", step);
			if(byte != 0x00)
			{
				net.send(HAS_N, "[-] Bad SOCKS5 server response (invalid remote addr?)", NULL);
				step = 0;
				return -10;
			}
			++step;
			break;
		}
		case 11:
		{
			//printf("step %d\n", step);
			++step;
			break;
		}
		case 12:
		{
			//printf("step %d\n", step);
			if(byte == 0x01)
				toRead = 6;
			else if(byte == 0x03)
				toRead = 2 + byte;
			else
			{
				net.send(HAS_N, "[-] Bad SOCKS5 server response (remote connect failed?)", NULL);
				step = 0;
				return 12;
			}
			++step;
			break;
		}
		case 13:
		{
			//printf("step %d\n", step);
			if(!--toRead)
			{
				net.send(HAS_N, "[+] Connected to remote host via SOCKS5 proxy", NULL);
				step = 0;
				return fd;
			}
			break;
		}
		default:
			return -1;
	}
	return 0;
}

asyn_socks5::asyn_socks5()
{
	memset(this, 0, sizeof(asyn_socks5));
}

int asyn_socks5::connect()
{
	//printf("proxy:  %s:%d\n", proxyip, proxyport);
	//printf("remote: %s:%d\n", remotehost, remoteport);
	disconnect();
	fd = doConnect(proxyip, proxyport, "", -1);
	if(fd > 0)
	{
		step = 1;
		return fd;
	}
	else return -1;
}

void asyn_socks5::setup(const char *pip, unsigned short pport, const char *rhost, unsigned short rport)
{
	strncpy(proxyip, pip, sizeof(proxyip));
	strncpy(remotehost, rhost, sizeof(remotehost));
	proxyport = pport;
	remoteport = rport;
}

void asyn_socks5::disconnect()
{
	if(fd)
	{
		killSocket(fd);
		fd = 0;
	}
	step = 0;
}

int asyn_socks5::use()
{
	if(strlen(proxyip) && proxyport && strlen(remotehost) && remoteport)
		return 1;
	else return 0;
}
