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

/************************************************************************
 *   IRC - Internet Relay Chat, common/match.c
 *   Copyright (C) 1990 Jarkko Oikarinen
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 1, or (at your option)
 *   any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, write to the Free Software
 *   Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include "prots.h"

unsigned char tolowertab[] =
    { 0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
      0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14,
      0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
      0x1e, 0x1f,
      ' ', '!', '"', '#', '$', '%', '&', 0x27, '(', ')',
      '*', '+', ',', '-', '.', '/',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      ':', ';', '<', '=', '>', '?',
      '@', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
      'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
      't', 'u', 'v', 'w', 'x', 'y', 'z', '[', '\\', ']', '^',
      '_',
      '`', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i',
      'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's',
      't', 'u', 'v', 'w', 'x', 'y', 'z', '{', '|', '}', '~',
      0x7f,
      0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
      0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
      0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
      0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
      0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
      0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
      0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
      0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
      0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
      0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
      0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
      0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
      0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
      0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
      0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };

unsigned char touppertab[] =
    { 0, 0x1, 0x2, 0x3, 0x4, 0x5, 0x6, 0x7, 0x8, 0x9, 0xa,
      0xb, 0xc, 0xd, 0xe, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14,
      0x15, 0x16, 0x17, 0x18, 0x19, 0x1a, 0x1b, 0x1c, 0x1d,
      0x1e, 0x1f,
      ' ', '!', '"', '#', '$', '%', '&', 0x27, '(', ')',
      '*', '+', ',', '-', '.', '/',
      '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
      ':', ';', '<', '=', '>', '?',
      '@', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
      'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
      'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '[', '\\', ']', '^',
      0x5f,
      '`', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I',
      'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S',
      'T', 'U', 'V', 'W', 'X', 'Y', 'Z', '{', '|', '}', '~',
      0x7f,
      0x80, 0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
      0x8a, 0x8b, 0x8c, 0x8d, 0x8e, 0x8f,
      0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
      0x9a, 0x9b, 0x9c, 0x9d, 0x9e, 0x9f,
      0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
      0xaa, 0xab, 0xac, 0xad, 0xae, 0xaf,
      0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
      0xba, 0xbb, 0xbc, 0xbd, 0xbe, 0xbf,
      0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8, 0xc9,
      0xca, 0xcb, 0xcc, 0xcd, 0xce, 0xcf,
      0xd0, 0xd1, 0xd2, 0xd3, 0xd4, 0xd5, 0xd6, 0xd7, 0xd8, 0xd9,
      0xda, 0xdb, 0xdc, 0xdd, 0xde, 0xdf,
      0xe0, 0xe1, 0xe2, 0xe3, 0xe4, 0xe5, 0xe6, 0xe7, 0xe8, 0xe9,
      0xea, 0xeb, 0xec, 0xed, 0xee, 0xef,
      0xf0, 0xf1, 0xf2, 0xf3, 0xf4, 0xf5, 0xf6, 0xf7, 0xf8, 0xf9,
      0xfa, 0xfb, 0xfc, 0xfd, 0xfe, 0xff };

#define	MAX_ITERATIONS	512

/*
**  Compare if a given string (name) matches the given
**  mask (which can contain wild cards: '*' - match any
**  number of chars, '?' - match any single character.
**
**	return	0, if match
**		1, if no match
*/

/*
** ircd_match()
** Iterative matching function, rather than recursive.
** Written by Douglas A Lewis (dalewis@acsu.buffalo.edu)
*/

int	ircd_match(const char *mask, const char *name)
{
    u_char	*m = (u_char *)mask, *n = (u_char *)name;
	char	*ma = const_cast<char*>(mask), *na = const_cast<char*>(name);
    int	wild = 0, q = 0, calls = 0;

    if (!*mask)
        return 1;

    if (mask[0]=='*' && mask[1]=='\0')
        return 0;

    while (1)
    {
#ifdef	MAX_ITERATIONS
        if (calls++ > MAX_ITERATIONS)
            break;
#endif

        if (*m == '*')
        {
            while (*m == '*')
                m++;
            wild = 1;
            ma = (char *)m;
            na = (char *)n;
        }

        if (!*m)
        {
            if (!*n)
                return 0;
            for (m--; (m > (u_char *)mask) && (*m == '?'); m--)
                ;
            if ((m > (u_char *)mask) && (*m == '*') &&
                    (m[-1] != '\\'))
                return 0;
            if (!wild)
                return 1;
            m = (u_char *)ma;
            n = (u_char *)++na;
        }
        else if (!*n)
            return 1;
        if ((*m == '\\') &&
                ((m[1] == '*') || (m[1] == '?') || (m[1] == '#')))
        {
            m++;
            q = 1;
        }
        else
            q = 0;

        if ((tolower(*m) == tolower(*n))
                || (*m == '?' && !q)
                || (*m == '#' && !q && isdigit(*n))
                || (*m == '%' && !q && (*n == '+' || *n == '-' || *n == '=')))
        {
            if (*m)
                m++;
            if (*n)
                n++;
        }
        else
        {
            if (!wild)
                return 1;
            m = (u_char *)ma;
            n = (u_char *)++na;
        }
    }

    return 1;
}


/*
** collapse a pattern string into minimal components.
** This particular version is "in place", so that it changes the pattern
** which is to be reduced to a "minimal" size.
*/
char *collapse(char *pattern)
{
    char *s = pattern, *s1, *t;

    /*
     * Collapse all \** into \*, \*[?]+\** into \*[?]+
     */
    for (; *s; s++)
        if (*s == '\\')
            if (!*(s + 1))
                break;
            else
                s++;
        else if (*s == '*')
        {
            if (*(t = s1 = s + 1) == '*')
                while (*t == '*')
                    t++;
            else if (*t == '?')
                for (t++, s1++; *t == '*' || *t == '?'; t++)
                    if (*t == '?')
                        *s1++ = *t;
            while ((*s1++ = *t++))
                ;
        }
    return pattern;
}

/*
**  Case insensitive comparison of two null terminated strings.
**
**	returns	 0, if s1 equal to s2
**		<0, if s1 lexicographically less than s2
**		>0, if s1 lexicographically greater than s2
*/
int ircd_strcmp(const char *s1, const char *s2)
{
    unsigned char *str1 = (unsigned char *)s1;
    unsigned char *str2 = (unsigned char *)s2;
    int	res;

    while ((res = toupper(*str1) - toupper(*str2)) == 0)
    {
        if (*str1 == '\0')
            return 0;
        str1++;
        str2++;
    }
    return (res);
}

int	ircd_strncmp(const char *str1, const char *str2, int n)
{
    unsigned char *s1 = (unsigned char *)str1;
    unsigned char *s2 = (unsigned char *)str2;
    int	res;

    while ((res = ircd_toupper(*s1) - ircd_toupper(*s2)) == 0)
    {
        s1++;
        s2++;
        n--;
        if (n == 0 || (*s1 == '\0' && *s2 == '\0'))
            return 0;
    }
    return (res);
}

int ircd_tolower(u_char c)
{
    return tolowertab[c];
}

int ircd_toupper(u_char c)
{
    return touppertab[c];
}

int match(const char *mask, const char *str)
{
	return !ircd_match(mask, str);
}

int wildMatch(const char *mask1, const char *mask2)
{
	return !ircd_match(mask1, mask2) || !ircd_match(mask2, mask1);
}

unsigned int network_bitcmp(unsigned char *d1, unsigned char *d2, int bits)
{
	int n;
	unsigned int ret;

	while(bits)
	{
		n = bits - 8;
		if(n >= 0)
		{
			if((ret = *d1 - *d2))
				return ret;

			bits = n;
		}
		else if (n < 0)
		{
			if((ret = ((*d1 ^ *d2) >> -n)))
				return ret;

			return 0;
		}

		++d1;
		++d2;
	}
	return 0;
}

int matchBan(const char *ban, const chanuser *u, const bool wild, const char *ip, const char *uid)
{
	if(ip)
		return matchBan(ban, u->nick, u->ident, u->host, wild, ip, uid);

	if(*u->ip4)
		if(matchBan(ban, u->nick, u->ident, u->host, wild, u->ip4, uid))
			return 1;
	if(*u->ip6)
		if(matchBan(ban, u->nick, u->ident, u->host, wild, u->ip6, uid))
			return 1;

	return matchBan(ban, u->nick, u->ident, u->host, wild, NULL, uid);
}

int matchBanMask(const char *ban, const char *mask, const bool wild, const char *ip, const char *uid)
{
	char *mEx = NULL;
	char *mAt = NULL;
	char *tmp = const_cast<char*>(mask);

	//check mask corectness
	while(*tmp)
	{
		if(*tmp == '!')
		{
			if(mEx)
				return 0;
			mEx = tmp;
		}
		else if(*tmp == '@')
		{
			if(mAt)
				return 0;
			mAt = tmp;
		}
		++tmp;
	}
	if(!mEx || !mAt)
		return 0;

	*mEx = '\0';
	*mAt = '\0';
	int ret = matchBan(ban, mask, mEx+1, mAt+1, wild, ip, uid);
	*mEx = '!';
	*mAt = '@';
	return ret;
}

int matchBan(const char *ban, const char *nick, const char *ident, const char *host, const bool wild, const char *ip, const char *uid)
{
	char *bEx = NULL;
	char *bAt = NULL;

	char *tmp = const_cast<char*>(ban);
	while(*tmp)
	{
		if(*tmp == '!')
		{
			if(bEx)
				return 0;
			bEx = tmp;
		}
		else if(*tmp == '@')
		{
			if(bAt)
				return 0;
			bAt = tmp;
		}
		++tmp;
	}
	if(!bEx || !bAt)
		return 0;

	int ret = 0;

	*bAt = '\0';
	*bEx = '\0';

	// sth-here!*@* should never match *!sth-here@sth-here
	if(!strcmp(bAt+1, "*") && !strcmp(bEx+1, "*") && !strcmp(nick, "*"))
		return 0;


	//compare ident
	if(match(bEx+1, ident) || (wild && match(ident, bEx+1)))
	{
		//compare nick

		if(match(ban, nick) || (uid && *uid && match(ban, uid)) || (wild &&
			(match(nick, ban) || (uid && *uid && match(uid, ban)))))
		{
			//compare host
			ret = (match(bAt+1, host) || matchIp (bAt+1, host) || (ip && *ip && matchIp(bAt+1, ip)) || (wild &&
					(match(host, bAt+1) || match(host, bAt+1) || (ip && *ip && matchIp(ip, bAt+1)))));
		}
	}

	*bAt = '@';
	*bEx = '!';

	return ret;
}


int matchIp(const char *bannedIp, const char *ip)
{
	char *slash = strrchr(bannedIp, '/');

	if(!slash)
		return match(bannedIp, ip);

	int cls = atoi(slash + 1);

	if(!cls)
		return 1;

	u_char banip[NS_IN6ADDRSZ], hostip[NS_IN6ADDRSZ];
	//unsigned int banip, hostip;

	*slash = '\0';
	if(!inet_pton4(bannedIp, banip))
	{
		/* maybe thats ipv6 cidr ban? */
		if(!inet_pton6(bannedIp, banip))
		{
			*slash = '/';
			return 0;
		}
		*slash = '/';

		if(!inet_pton6(ip, hostip))
		{
			return 0;
		}
		return !network_bitcmp(banip, hostip, cls);
	}
	*slash = '/';

	if(!inet_pton4(ip, hostip))
	{
		return 0;
	}

	return !network_bitcmp(banip, hostip, cls);
}

