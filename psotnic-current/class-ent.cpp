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
#include "defines.h"
#include "global-var.h"

/*
 * ent
 */
bool ent::operator<(const ent &e) const
{
	return strcmp(name, e.name) < 0 ? true : false;
}

/*! Compares two entities for name equality.
 * \note This only compares the names, not the values.
 * \param e The entity to compare with.
 * \return true if the names match, false otherwise.
 */
bool ent::operator==(const ent &e) const
{
	return !strcmp(name, e.name);
}

const char *ent::print(const int n) const
{
	static char buf[256];
	const char *v = getValue();
	snprintf(buf, 256, "%s %*s", name, n - strlen(name) + 3 + strlen(v), v);
	return buf;
}

/*! Returns the name of the entity.
 * \return The name of the entity.
 */
const char *ent::getName() const
{
	return name;
}

options::event *ent::set(const char *arg, const bool justTest)
{
	return setValue(name, arg, justTest);
}

/*! Checks if the entity is printable in a reasonable form.
 * \return true if the entity could be printed, false otherwise.
 */
bool ent::isPrintable() const
{
	return dontPrintIfDefault ? !isDefault() : true;
}

/*
 * entBool
 */
options::event ent::_event;

/*! Integer case operator.
 * \return The integer expression of the current value.
 */
entBool::operator int() const
{
	return value;
}

/*! Returns the current value as a string. The value is either "ON" or "OFF".
 * \return The string expression of the current value.
 */
const char *entBool::getValue() const
{
	return value ? "ON" : "OFF";
}

options::event *entBool::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		if(!arg2)
		{
		    _event.setError(this, "argument doesn't exists for entry %s", name);
		    return &_event;
		}

		bool ok;
		bool n = str2int(arg2, ok);

		if(ok)
		{
			if(!justTest)
				value = n;

			_event.setOk(this, "%s has been turned %s", name, getValue());
			return &_event;
		}
		else
		{
			_event.setError(this, "argument is not a boolean value");
			return &_event;
		}
	}
	else if((*arg1 == '-' || *arg1 == '+') && !strcmp(arg1+1, name))
	{
		if(!justTest)
			value = (*arg1 == '+');

		_event.setOk(this, "%s has been turned %s", name, value ? "ON" : "OFF");
		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

/*! Converts the string \a str to a boolean expression. Values like "yes" or "on" are considered
 * true, while "no", "off" and all other values are considered false.
 * \param str The string to converts.
 * \param ok Set to true if the conversation was successfull.
 * \return The boolean expression of the string.
 */
bool entBool::str2int(const char *str, bool &ok) const
{
	if(!strcmp(str, "yes") || !strcmp(str, "ON") ||
		   !strcmp(str, "enable") || !strcmp(str, "1"))
	{
		ok = true;
		return true;
	}

	if(!strcmp(str, "no") || !strcmp(str, "off") ||
		   !strcmp(str, "disable") || !strcmp(str, "0"))
	{
		ok = true;
		return false;
	}

	ok = false;
	return false;
}

/*! Resets the current value by assigning the default value to it. */
void entBool::reset()
{
	value = defaultValue;
}

/*! Checks if the current value is the default value.
 * \return true if the current value is the default value, false otherwise.
 */
bool entBool::isDefault() const
{
	return value == defaultValue;
}

/*
 * entInt
 */

options::event *entInt::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		bool ok;
		int n = str2int(arg2, ok);

		if(!ok)
		{
			_event.setError(this, "argument is not an integer type");
			return &_event;
		}
		if(n >= min && n <= max)
		{
			if(!justTest)
				value = n;

			_event.setOk(this, "%s has been set to %s", name, getValue());
			return &_event;
		}
		else
		{
			_event.setError(this, "argument does not belong to range <%s, %s>", getMin(), getMax());
			return &_event;
		}
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

/*! Converts the current value to a string.
 * \return The resulting string.
 */
const char *entInt::getValue() const
{
	static char buf[20];
	sprintf(buf, "%d", value);
	return buf;
}

/*! Converts the string \a str to an integer.
 * \param str The string to converts.
 * \param ok Always true.
 * \return The integer expression of the string.
 * \see atoi
 */
int entInt::str2int(const char *str, bool &ok) const
{
	ok = true;
	return atoi(str);
}

/*! Returns the minimum allowed number as a string.
 * \return The resulting string.
 */
const char *entInt::getMin() const
{
	static char buf[20];
	sprintf(buf, "%d", min);
	return buf;
}

/*! Returns the maximum allowed number as a string.
 * \return The resulting string.
 */
const char *entInt::getMax() const
{
	static char buf[20];
	if(max != MAX_INT)
		sprintf(buf, "%d", max);
	else
		strcpy(buf, "INFINITY");

	return buf;
}

/*! Integer cast operator.
 * \return The current value of the entity.
 */
entInt::operator int() const
{
	return value;
}

/*! Comparison operator for comparing the entities value to an other integer \a n.
 * \return true if both are equal, false otherwise.
 */
bool entInt::operator==(int n) const
{
	return value == n;
}

/*
 * class entTime
 */

/*! Converts a given string \a str to an integer in unix timestamp format.
 * \param str The string to converts.
 * \param ok Set to true if the conversion was successfull, false otherwise.
 * \return The resulting integer or 0 if conversion failed.
 */
int entTime::str2int(const char *str, bool &ok) const
{
	int ret = 0;

	if(units2int(str, ut_time, ret) != 1)
	{
		ok = false;
		return 0;
	}

	ok = true;
	return ret;
}

/*! Returns the currently stored time as a string.
 * \return The resulting string.
 */
const char *entTime::getValue() const
{
	static char buf[80];
	int2units(buf, 80, value, ut_time);

	return buf;
}

/*! Converts the minimum accepted time to a string.
 * \return The resulting string.
 */
const char *entTime::getMin() const
{
	static char buf[80];
	int2units(buf, 80, min, ut_time);

	return buf;
}

/*! Convers the maximum accepte time to a string.
 * \return The resulting string.
 */
const char *entTime::getMax() const
{
	static char buf[80];

	if(max != MAX_INT)
		int2units(buf, 80, max, ut_time);
	else
		strcpy(buf, "INFINITY");

	return buf;
}

/*
 * class entPerc
 */

int entPerc::str2int(const char *str, bool &ok) const
{
	int ret = 0;

	if(units2int(str, ut_perc, ret) != 1)
	{
		ok = false;
		return 0;
	}

	ok = true;
	return ret;
}

const char *entPerc::getValue() const
{
	static char buf[80];
	int2units(buf, 80, value, ut_perc);

	return buf;
}

const char *entPerc::getMax() const
{
	static char buf[80];
	int2units(buf, 80, min, ut_perc);

	return buf;
}

const char *entPerc::getMin() const
{
	static char buf[80];
	int2units(buf, 80, max, ut_perc);

	return buf;
}

/*
 * class entHost
 */

entHost::operator const char*() const
{
	return (const char *) connectionString;
}

const char *entHost::getValue() const
{
	return connectionString;
}

void entHost::reset()
{
	ip = "0.0.0.0";
	connectionString = "0.0.0.0";
}

bool entHost::isDefault() const
{
	return !strcmp(connectionString, "0.0.0.0");
}

int entHost::getConnectionStringType(const char *str) const
{
	int type = 0;
	const char *ptr = str;

	if(!strncmp(ptr, "ssl:", 4))
	{
		type |= use_ssl;
		ptr += 4;
	}

	switch(isValidIp(ptr))
	{
		case 4:
			type |= ipv4;
			break;
		case 6:
			type |= ipv6;
			break;
		default:
			type |= domain;
	}

	return type;
}


options::event *entHost::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		int type = getConnectionStringType(arg2);
		int check = (type & typesAllowed) ^ type;

		if(check != 0)
		{
			//TODO: add more verbous message here
			_event.setError(this, "%s is not valid: unsupported format", arg2);
			return &_event;
		}

		const char *_arg = (type & use_ssl) ? arg2+4 : arg2;

		if(type & (ipv4 | ipv6))
		{
			if(!justTest)
			{
				ip = _arg;
				connectionString = arg2;
			}
			_event.setOk(this, "%s has been set to %s", name, getValue());
			return &_event;
		}
		else if(type & domain)
		{
			char buf[MAX_LEN];
			bool ok = false;
#ifdef HAVE_IPV6
			if((typesAllowed & ipv6) && inet::gethostbyname(_arg, buf, AF_INET6))
				ok = true;
			else
#endif
			if(!ok && (typesAllowed & ipv4) && inet::gethostbyname(_arg, buf, AF_INET))
				ok = true;

			if(ok)
			{
				if(!justTest)
				{
					ip = buf;
					connectionString = arg2;
				}

				_event.setOk(this, "%s has been set to %s", name, getValue());
			}
			else if (errno)
				_event.setError(this, "Unknown host: %s (%s)", _arg, hstrerror(errno));
			else
				_event.setError(this, "Unknown host: %s", _arg);
			return &_event;
		}
		else
		{
			_event.setError(this, "FIXME: Unsupported type");
			return &_event;
		}
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

entHost::operator unsigned int() const
{
	return inet_addr(ip);
}

/**
 * class entString
 */
const char *entString::getValue() const
{
	return string;
}

options::event *entString::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		int n = strlen(arg2);

		if(n > max)
		{
			_event.setError(this, "argument is longer than %d characters", max);
			return &_event;
		}
		else if(n < min)
		{
			_event.setError(this, "argument is shorter than %d characters", min);
			return &_event;
		}
		else
		{
			if(!justTest)
				string = arg2;

			_event.setOk(this, "%s has been set to %s", name, getValue());
			return &_event;
		}
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

/*! Resets the entities value to its default. */
void entString::reset()
{
	string = defaultString;
}

bool entString::isDefault() const
{
	return !strcmp(defaultString, string);
}

/*! String cast operator.
 * \return The current value.
 */
entString::operator const char*() const
{
	return string;
}

/*! Returns the current string length.
 * \return The current string length.
 */
int entString::getLen() const
{
	return string.len();
}

/**
 * class entWord
 */
options::event *entWord::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		if(countWords(arg2) > 1)
		{
			_event.setError(this, "argument is not a one word");
			return &_event;
		}

		int n = strlen(arg2);

		if(n > max)
		{
			_event.setError(this, "argument is longer than %d characters", max);
			return &_event;
		}
		else if(n < min)
		{
			_event.setError(this, "argument is shorter than %d characters", min);
			return &_event;
		}
		else
		{
			if(!justTest)
				string = arg2;

			_event.setOk(this, "%s has been set to %s", name, getValue());
			return &_event;
		}
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

/*
 * class entHPPH
 */

options::event *entHPPH::_setValue(const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		options::event *e;

		if(_host)
		{
			e = _host->set(arg2, 1);
			if(!e || !e->ok)
				return e;
		}
		if(_port)
		{
			e = _port->set(arg3, 1);
			if(!e || !e->ok)
				return e;
		}
		if(_pass)
		{
			e = _pass->set(arg4, 1);
			if(!e || !e->ok)
				return e;
		}
		if(_handle)
		{
			e = _handle->set(arg5, 1);
			if(!e || !e->ok)
				return e;
		}

		if(!justTest)
		{
			if(_host)
				_host->set(arg2);
			if(_port)
				_port->set(arg3);
			if(_pass)
				_pass->set(arg4);
			if(_handle)
				_handle->set(arg5);
		}

		_event.setOk(this, "%s has been set to %s", name, getValue());

		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

/*! Converts the entity to a string of the form "host port pass handle".
 * \return The string representation of the entity.
 */
const char *entHPPH::getValue() const
{
	static char buf[1024];

	if(_host)
		strcpy(buf, _host->getValue());
	if(_port)
	{
		strcat(buf, " ");
		strcat(buf, _port->getValue());
	}
	if(_pass && *(_pass->getValue())!='\0')
	{
		strcat(buf, " ");
		strcat(buf, _pass->getValue());
	}
	if(_handle && *(_handle->getValue())!='\0')
	{
		strcat(buf, " ");
		strcat(buf, _handle->getValue());
	}

	return buf;
}

options::event *entHPPH::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	char arg[4][256];
	str2words(arg[0], arg2 ? arg2 : "", 4, 256);

	return _setValue(arg1, arg[0], arg[1], arg[2], arg[3], justTest);
}

/*! Resets the stored entities to their default values. */
void entHPPH::reset()
{
	if(_host)
		_host->reset();
	if(_port)
		_port->reset();
	if(_pass)
		_pass->reset();
	if(_handle)
		_handle->reset();
}

/*! Checks if all entities are at their default values.
 * \return true if all entities are at their defaults, false otherwise.
 */
bool entHPPH::isDefault() const
{
	if(_host && !_host->isDefault())
		return false;
	if(_port && !_port->isDefault())
		return false;
	if(_pass && !_pass->isDefault())
		return false;
	if(_handle && !_handle->isDefault())
		return false;

	return true;
}

entHPPH::~entHPPH()
{
	if(_host)
		delete _host;
	if(_port)
		delete _port;
	if(_pass)
		delete _pass;
	if(_handle)
		delete _handle;
}

entHPPH &entHPPH::operator=(const entHPPH &e)
{
	name = e.name;
	dontPrintIfDefault = e.dontPrintIfDefault;
	readOnly = e.readOnly;

	if(_host)
		delete _host;
	if(_port)
		delete _port;
	if(_pass)
		delete _pass;
	if(_handle)
		delete _handle;

	if(e._host)
	{
		_host = new entHost();
		*_host = *e._host;
	}
	else
		_host = NULL;

	if(e._port)
	{
		_port = new entInt();
		*_port = *e._port;
	}
	else
		_port = NULL;

	if(e._pass)
	{
		if(typeid(*e._pass) == typeid(entWord))
			_pass = new entWord();
		else if(typeid(*e._pass) == typeid(entMD5Hash))
			_pass = new entMD5Hash();

		*_pass = *e._pass;
	}
	else
		_pass = NULL;

	if(e._handle)
	{
		_handle = new entWord();
		*_handle = *e._handle;
	}
	else
		_handle = NULL;

	return *this;
}

/**
 * class entMD5Hash
 */
options::event *entMD5Hash::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}
		//TODO: add [0-9a-f] check
		//its a hash
		int n = strlen(arg2);
		if(n == 32)
		{
			if(!justTest)
			{
				string =  arg2;
				quoteHex(arg2, hash);
			}
		}
		else
		{
			if(!justTest)
			{
				char buf[33];
				MD5Hash(hash, arg2, n);
				quoteHexStr(hash, buf, 16);
				string = buf;
			}
		}

		_event.setOk(this, "%s has been set to %s", arg1, getValue());
		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

entMD5Hash::operator const unsigned char*() const
{
	return hash;
}

const unsigned char *entMD5Hash::getHash() const
{
	return hash;
}

void entMD5Hash::reset()
{
	memset(hash, 0, 16);
	string = "";
}

bool entMD5Hash::isDefault() const
{
	for(int i=0; i<16; ++i)
		if(hash[i])
			return 0;

	return 1;
}

/**
 * class entMult
 */
options::event *entMult::setValue(const char *arg1, const char *arg2, bool justTest)
{
	if(!strcmp(arg1, name) || (*arg1 == '+' && !strcmp(arg1+1, name)))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		ptrlist<ent>::iterator i = list.begin();
		while(i)
		{
			if(i->isDefault())
			{
				options::event *e = i->set(arg2, justTest);
				if(!e || !e->ok)
					return e;

				_event.setOk(i, "adding new entry: %s %s", i->name, i->getValue());
				return &_event;
			}
			i++;
		}
		_event.setError(this, "%s has reached maximum number of entries, please remove some entries in order to add new ones", name);
		return &_event;
	}
	else if(*arg1 == '-' && arg2 && *arg2 && !strcmp(arg1+1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		ptrlist<ent>::iterator i = list.begin();
		while(i)
		{
			if(!i->isDefault() && !strcmp(i->getValue(), arg2))
			{
				_event.setOk(i, "removing entry: %s %s", i->name, i->getValue());
				i->reset();
				return &_event;
			}
			i++;
		}
		_event.setError(this, "no such entry");
		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

void entMult::reset()
{
	ptrlist<ent>::iterator i = list.begin();
	while(i)
	{
		i->reset();
		i++;
	}
}

void entMult::add(ent *e)
{
	list.addLast(e);
}

/**
 * entServer
 */
options::event *entServer::set(const char *ip, const char *port, const char *pass, const bool justTest)
{
	return _setValue(name, ip, port, pass, ip, justTest);
}

/**
 * entLoadModules
 */
//extern void registerAll(int (*_register)(const char *name, DLSYM_FUNCTION address));

bool entLoadModules::isDefault() const
{
	return *m_file=='\0';
}

void entLoadModules::reset()
{
	entLoadModules::unload(m_file);
	m_file="";
	m_md5sum="";
}

entLoadModules &entLoadModules::operator=(const entLoadModules &e)
{
	name=e.name;
	m_md5=e.m_md5;
	dontPrintIfDefault=e.dontPrintIfDefault;
	readOnly=e.readOnly;

	return *this;
}

const char *entLoadModules::getValue() const
{
	static char str[MAX_LEN];

	if(*m_md5sum)
		snprintf(str, MAX_LEN, "%s %s", (const char*)m_file, (const char*)m_md5sum);
	else
		snprintf(str, MAX_LEN, "%s", (const char*)m_file);

	return str;
}

options::event *entLoadModules::setValue(const char *arg1, const char *arg2, bool justTest)
{
	char arg[2][256];
	str2words(arg[0], arg2 ? arg2 : "", 2, 256);

	return _setValue(arg1, arg[0], arg[1], justTest);
}

options::event *entLoadModules::_setValue(const char *arg1, const char *arg2, const char *arg3, bool justTest)
{
	if(!strcmp(arg1, name))
	{
#ifdef HAVE_MODULES
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		if(findModule(arg2))
		{
			_event.setError(this, "module is already loaded");
			return &_event;
		}

		int fd;
		unsigned char digest[16];
		char digestHex[33];

		if(m_md5)
		{
			struct stat statbuf;

			// avoid crashes on systems that allow to open() a directory -- patrick

			if(stat(arg2, &statbuf) == 0)
			{
				if(!S_ISREG(statbuf.st_mode))
				{
					_event.setError(this, "cannot open %s: no regular file", arg2);
					return &_event;
				}
			}

			else
			{
				_event.setError(this, "cannot stat %s: %s", arg2, strerror(errno));
				return &_event;
			}

			fd = open(arg2, O_RDONLY);

			if(fd == -1)
			{
				_event.setError(this, "cannot open %s: %s", arg2, strerror(errno));
				close(fd);
				return &_event;
			}
			MD5HashFile(digest, fd);
			close(fd);

			quoteHexStr(digest, digestHex, 16);

			if(*arg3)
			{
				if(strcmp(digestHex, arg3))
				{
					_event.setError(this, "cannot load %s: md5 signature missmatch", arg2);
					return &_event;
				}
			}
		}
		else
			digestHex[0] = '\0';

		// load the module
		void *handle = dlopen(arg2, RTLD_NOW);

		if(!handle)
		{
			_event.setError(this, "error while loading %s: %s", arg2, dlerror());
			return &_event;
		}

		sfp version = (sfp) dlsym(handle, "version");
		if(!version)
		{
			_event.setError(this, "error while loading module %s: %s", arg2, dlerror());
			dlclose(handle);
			return &_event;
		}

		sfp description = (sfp) dlsym(handle, "description");
		if(!description)
		{
			_event.setError(this, "error while loading module %s: %s", arg2, dlerror());
			dlclose(handle);
			return &_event;
		}

		sfp author = (sfp) dlsym(handle, "author");
		if(!author)
		{
			_event.setError(this, "error while loading module %s: %s", arg2, dlerror());
			dlclose(handle);
			return &_event;
		}

		sfp email = (sfp) dlsym( handle, "email" );
		if(!email)
		{
			_event.setError(this, "error while loading %s: %s", arg2, dlerror());
			dlclose(handle);
 			return &_event;
		}

		sfp name = (sfp) dlsym(handle, "name");
		if(!name)
		{
			_event.setError(this, "error while loading %s: %s", arg2, dlerror());
			dlclose(handle);
			return &_event;
		}

		sfp compileTime = (sfp) dlsym(handle, "compileTime");
		if(!compileTime)
		{
			_event.setError(this, "error while loading %s: %s", arg2, dlerror());
			dlclose(handle);
			return &_event;
		}

		sfp compileDate = (sfp) dlsym(handle, "compileDate");
		if(!compileDate)
		{
			_event.setError(this, "error while loading %s: %s", arg2, dlerror());
			dlclose(handle);
			return &_event;
		}

		mfp load = (mfp) dlsym(handle, "load");
		if(!load)
		{
			_event.setError(this, "error while loading %s: %s", arg2, dlerror());
			dlclose(handle);
			return &_event;
		}

		vfp unload = (vfp) dlsym(handle, "unload");//dlsym_cast?
		if(!unload)
		{
			_event.setError(this, "error while loading %s: %s", arg2, dlerror());
			dlclose(handle);
			return &_event;
		}


		/* FIRE */
		Module *module = load(handle, arg2, digestHex, time(NULL), "./");

		if(!justTest)
		{
			modules.addLast(module);
			module->setAuthor(author());
			module->setCompileTime(compileTime(), compileDate());
			module->setDescription(description());
			module->setEmail(email());
			module->setName(name());
			module->setVersion(version());
			m_file = arg2;
			m_md5sum = digestHex;
			m_loadDate = time(NULL);
			//m_handle=handle;


			if(m_md5)
				m_md5sum=digestHex;

				string loadMessage;
				bool loadCheck = module->onLoad( /* args, */ loadMessage );
				if(!loadCheck)
				{
					_event.setError(this, "Module %s refused to load: %s", arg2, loadMessage.c_str());
					this->unload(arg2);
					return &_event;
				}
		}
		else
		{
			dlclose(handle);
		}

		_event.setOk(this, "loaded module %s", (const char *)name());
		return &_event;

#else
	_event.setError(this, "This version of bot does not support modules");
	return &_event;
#endif
	}
	return NULL;
}

ptrlist<Module>::iterator entLoadModules::findModule(const char *str)
{
	ptrlist<Module>::iterator i = modules.begin();

	while(i)
	{
		if(!strcmp(i->file(), str))
			return i;

		i++;
	}

	return i;
}


bool entLoadModules::unload(const char *str)
{
#ifdef HAVE_MODULES
	ptrlist<Module>::iterator i = findModule(str);

	if(i)
	{
		void *handle=i->handle();
		modules.removeLink(i, false);
		dlclose(handle);
		return true;
	}
#endif
	return false;
}


/*
bool entLoadModules::rehash(const char *str)
{
	if(
*/

/**
 * class entChattr
 */

int entChattr::checkArg(const char *args)
{
	int i, j, k, c;
	size_t s;
	int star;
	char *_l = NULL;
	char *_k = NULL;

	const char *modes = CHATTR_MODES;

	static char duplicate[32];
	static char arg[3][CHAN_LEN];

	memset(gpFlags, 0, sizeof(gpFlags));
	memset(gmFlags, 0, sizeof(gmFlags));
	memset(gKey, 0, sizeof(gKey));
	gLimit = 0;

	memset(duplicate, 0, sizeof(duplicate));

	str2words(arg[0], args, 3, CHAN_LEN);

	for(k = 0, c = 0, i = 0, j = strlen(arg[0]), s = sizeof(duplicate); i < j && (unsigned) i < s-1; i++)
	{
		if(arg[0][i] == '-' || arg[0][i] == '+')
			continue;
		if(arg[0][i] == '*')
		{
			if(!k)
			{
				k = 1;
				continue;
			}
			return -7;
		}
		if(strchr(modes, arg[0][i]) == NULL)
			return i;
		else // duplicated modes check
		{
			if(strchr(duplicate, arg[0][i]) != NULL)
				return -1;
			duplicate[c++] = arg[0][i];
		}
	}

	for(star = -1, i = 0, c = 1, j = strlen(arg[0]), s = 1; i < j; i++)
	{
		switch(arg[0][i])
		{
			case '-':
				c = 0;
				break;
			case '+':
				c = 1;
				break;
			case '*':
				star = c;
				break;
			default:
				setFlag(c, arg[0][i]);
				if(c && (arg[0][i] == 'l' || arg[0][i] == 'k') && s < 3)
				{
					switch(arg[0][i])
					{
						case 'l':
							if(!strlen(arg[s])) return -3;
							if(!_isnumber(arg[s])) return -4;
							gLimit = atol(arg[s++]);
							break;
						case 'k':
							k = strlen(arg[s]);
							if(!k) return -5;
							if(k > CHAN_LEN) return -6;

							strcpy(gKey, arg[s++]);
							break;
					}
				}
		}
	}

	if(star != -1)
	{
		for(i = 0, s = 1,j = strlen(modes); i < j; i++)
		{
			if(strchr(duplicate, modes[i]) == NULL)
			{
				setFlag(star, modes[i]);
				if(star && (modes[i] == 'l' || modes[i] == 'k') && s < 3)
				{
					switch(modes[i])
					{
						case 'l':
							if(!strlen(arg[s])) return -3;
							if(!_isnumber(arg[s])) return -4;
							gLimit = atol(arg[s++]);
							break;
						case 'k':
							k = strlen(arg[s]);
							if(!k) return -5;
							if(k > CHAN_LEN) return -6;

							strcpy(gKey, arg[s++]);
							break;
					}
				}
			}
		}
	}

	if(*gpFlags)
	{
		// +s and +p cannot exist bothly
		if(hasFlag(0, 'p', 1) && hasFlag(0, 's', 1))
			return -2;
		// darkman req. swaping limit with key when key isnt before limit
		if(hasFlag(0, 'l', 1) && hasFlag(0, 'k', 1))
		{
			_l = strchr(gpFlags, 'l');
			_k = strchr(gpFlags, 'k');
			if(_k != NULL && _l != NULL)
			{
				if(_l - _k > 0) // we need to swap those chars
				{
					i = (int) *_k;
					*_k = *_l;
					*_l = (char) i;
				}
			}
		}
	}
	return -8;
}

void entChattr::setFlags()
{
	strncpy(pFlags, gpFlags, sizeof(pFlags)-1);
	strncpy(mFlags, gmFlags, sizeof(mFlags)-1);
	strcpy(Key, gKey);
	Limit = gLimit;
}

bool entChattr::hasFlag(int minusFlag, const char flag, int Gen) const
{
	if(minusFlag)
		return strchr(Gen ? gmFlags : mFlags, flag)==NULL ? false : true;
        else
		return strchr(Gen ? gpFlags : pFlags, flag)==NULL ? false : true;

	return false;
}

void entChattr::setFlag(int plusFlag, const char flag)
{
	int len;

	if(plusFlag)
	{
		if(strchr(CHATTR_MODES, flag))
		{
			len=strlen(gpFlags);
			gpFlags[len]=flag;
			gpFlags[len+1]='\0';
		}

		else
			memset(gpFlags, 0, sizeof(gpFlags));
	}

	else
	{
		if(strchr(CHATTR_MODES, flag))
		{
			len=strlen(gmFlags);
			gmFlags[len]=flag;
			gmFlags[len+1]='\0';
		}

		else
			memset(gmFlags, 0, sizeof(gmFlags));
	}
}

const char *entChattr::getKey() const
{
	return Key;
}

long int entChattr::getLimit() const
{
	return Limit;
}

const char *entChattr::getValue() const
{
	static char modes[128];
	int _l, _k;
	char *l = NULL;
	char *k = NULL;

	memset(modes, 0, sizeof(modes));

	if(*pFlags || *mFlags)
	{
		if(*pFlags)
		{
			snprintf(modes, sizeof(modes), "+%s", pFlags);
			k = strchr(pFlags, 'k');
			l = strchr(pFlags, 'l');
		}
		if(*mFlags)
		{
			strncat(modes, "-", sizeof(modes)-strlen(modes)-1);
			strncat(modes, mFlags, sizeof(modes)-strlen(modes)-1);
		}

		// FIXME: we should use ltoa() instead of itoa()
		if(k != NULL || l != NULL)
		{
			strncat(modes, " ", sizeof(modes)-strlen(modes)-1);
			if(k != NULL && l != NULL)
			{
				_l = strlen(l);
				_k = strlen(k);

				if((_k - _l) > 0)
				{
					strncat(modes, Key, sizeof(modes)-strlen(modes)-1);
					strncat(modes, " ", sizeof(modes)-strlen(modes)-1);
					strncat(modes, itoa(Limit), sizeof(modes)-strlen(modes)-1);
				}
				else
				{
					strncat(modes, itoa(Limit), sizeof(modes)-strlen(modes)-1);
					strncat(modes, " ", sizeof(modes)-strlen(modes)-1);
					strncat(modes, Key, sizeof(modes)-strlen(modes)-1);
				}
			}
			else
			{
				if(k != NULL)
					strncat(modes, Key, sizeof(modes)-strlen(modes)-1);
				if(l != NULL)
					strncat(modes, itoa(Limit), sizeof(modes)-strlen(modes)-1);
			}
		}
	}
	else
		strncpy(modes, "-", sizeof(modes)-1);

	return modes;
}

options::event *entChattr::setValue(const char *arg1, const char *arg2, const bool justTest)
{
	if(!strcmp(arg1, name))
	{
		if(isReadOnly())
		{
			_event.setError(this, "entry %s is read-only", name);
			return &_event;
		}

		int i = checkArg(arg2);
		switch(i)
		{
			case -8: // all is ok
				break;
			case -7:
				_event.setError(this, "argument cannot contain wildcard char more than once");
				return &_event;
			case -6:
				_event.setError(this, "argument for key is too long");
				return &_event;
			case -5:
				_event.setError(this, "argument for key has no length");
				return &_event;
			case -4:
				_event.setError(this, "argument for limit is not a number");
				return &_event;
			case -3:
				_event.setError(this, "argument for limit has no lenght");
				return &_event;
			case -2: //
				_event.setError(this, "conflict in argument: +s cannot exist with +p");
				return &_event;
			case -1:
				_event.setError(this, "argument contain dulicated modes");
				return &_event;
			default:
				_event.setError(this, "argument contain incorrect char at position %d", i+1);
				return &_event;
		}


		setFlags();

		if(!justTest)
		{
			// FIXME: should we set it twice?
			setFlags();
		}

		_event.setOk(this, "%s has been set to %s", name, getValue());

		return &_event;
	}
	else
	{
		_event.setError(this);
		return NULL;
	}
}

void entChattr::reset()
{
	strcpy(pFlags, dpFlags);
	strcpy(mFlags, dmFlags);
	strcpy(Key, dKey);
	Limit = dLimit;
}

bool entChattr::isDefault() const
{
	if(!strcmp(dmFlags, mFlags) && !strcmp(dpFlags, pFlags) && !strcmp(dKey, Key) && dLimit == Limit)
		return 1;
	return 0;
}
