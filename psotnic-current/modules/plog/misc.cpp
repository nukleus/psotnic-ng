#include "plog.h"

char *mkdirhier_next_sep(char *);

/** returns hostmask like "patrick (patrick@psotnic.com)" if possible.
 * Otherwise it just returns 'from'.
 *
 * @param from any string
 * @return string in format "patrick (patrick@psotnic.com)" or the string in "from"
 */

char *get_hostmask(const char *from)
{
    static char buffer[MAX_LEN];

    if(match("*!*@*", from))
    {
        chanuser u(from, NULL, 0, false);
        snprintf(buffer, MAX_LEN, "%s (%s@%s)", u.nick, u.ident, u.host);
    }
    else // server
        snprintf(buffer, MAX_LEN, "%s", from);
    return buffer;
}

/** removes color codes from a string.
 * ripped from X-Chat Copyright (C) 1998 Peter Zelezny
 *
 * @param text any string which may contain color codes
 * @param len len of text variable
 * @param strip_hidden if true, hidden chars will be stripped
 * @return the given string without color codes
 */

char *strip_color(const char *text, int len, int strip_hidden)
{
	int i = 0;
	int rcol = 0, bgcol = 0;
	int hidden = false;
	char *new_str;

	new_str = (char*)malloc (len + 2);

	while (len > 0)
	{
		if (rcol > 0 && (isdigit (*text) || (*text == ',' && isdigit (text[1]) && !bgcol)))
		{
			if (text[1] != ',') rcol--;
			if (*text == ',')
			{
				rcol = 2;
				bgcol = 1;
			}
		} else
		{
			rcol = bgcol = 0;
			switch (*text)
			{
			case '\003': // ATTR_COLOR
				rcol = 2;
				break;
			case '\007': // ATTR_BEEP
			case '\017': // ATTR_RESET
			case '\026': // ATTR_REVERSE
			case '\002': // ATTR_BOLD
			case '\037': // ATTR_UNDERLINE
			case '\035' : // ATTR_ITALICS
				break;
			case '\010' : ////ATTR_HIDDEN
				hidden = !hidden;
				break;
			default:
				if (!(hidden && strip_hidden))
					new_str[i++] = *text;
			}
		}
		text++;
		len--;
	}

	new_str[i] = 0;

	return new_str;
}

/** needed by mkdirhier().
 * from mkdirhier.c
 */

char *mkdirhier_next_sep(char *path)
{
    while(*path)
        if(*path=='/')
            return path;
        else
            path++;
    return NULL;
}

/** makes a directory hierarchy.
 * from mkdirhier.c
 * @param dir_name a directory hierarchy, e.g. "dir1/dir2/dir3/
 * @return true on success, otherwise false
 */

bool mkdirhier(char *dir_name)
{
    char *next, *prev;
    char buf[1024];
    struct stat sb;

    if(!dir_name)
       return false;

    prev=dir_name;

    while((next=mkdirhier_next_sep(prev)))
    {
        strncpy(buf, dir_name, next-dir_name);
        buf[next-dir_name]='\0';

        if(stat(buf, &sb))
            mkdir(buf, 0700);

        prev=next+1;
    }

    if(mkdir(dir_name, 0700)==-1)
        return false;

    return true;
}

/** returns directory name of a path.
 * from dirname.c for busybox
 * @return directory name
 */

char *dirname(char *filename2)
{
    char *filename, *s;

    if(!filename2)
        return NULL;

    filename=strdup(filename2);
    s=filename+strlen(filename)-1;

    while (s && *s == '/')
    {
        *s='\0';
        s=filename+strlen(filename)-1;
    }

    s=strrchr(filename, '/');

    if (s && *s)
        *s='\0';

    return filename;
}

