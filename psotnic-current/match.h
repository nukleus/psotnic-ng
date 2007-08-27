/************************************************************************
 *   IRC - Internet Relay Chat, common/match_ext.h
 *   Copyright (C) 1997 Alain Nissen
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

/*  This file contains external definitions for global variables and functions
    defined in match.c.
 */
#ifndef IRCD_MATCH_H
#define IRCD_MATCH_H 1

extern unsigned char tolowertab[];
extern unsigned char touppertab[];
//extern unsigned char char_atribs[];

int ircd_match (const char *mask, const char *name);
char *ircd_collapse (char *pattern);
int ircd_strcmp (const char *s1, const char *s2);
int ircd_strncmp (const char *str1, const char *str2, int n);
int ircd_tolower(const u_char c);
int ircd_toupper(const u_char c);
#endif
