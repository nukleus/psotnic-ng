#ifndef PKS_CLASS_ENT_H
#define PKS_CLASS_ENT_H 1

#include "pstring.h"
#include "ptrlist.h"
#include "common.h"
#include "Options.hpp"

class ent;
class inetconn;
class Module;

/*! Configuration entity. Psotnics configuration is based on entities derived from this base class.
 * All options that should be stored have to use entities. This base class does not save actual
 * configuration values, but instead, it saves the name of the entity as well as some organisatory
 * stuff. Storage has to be done by the derived classes.
 */
class ent
{
	public:
	const char *name;		//!< The entities name.
	static options::event _event;	//!< Event.
	bool dontPrintIfDefault;	//!< If true, prevents this entity from beeing printed out to the config file.
	bool readOnly;			//!< Entities marked as RO cannot be changed.

	ent(const char *n=NULL) : name(n), dontPrintIfDefault(false), readOnly(false) { };
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0) = 0;
	virtual options::event *set(const char *arg, const bool justTest=0);
	virtual const char *getValue() const = 0 ;
	virtual const char *getName() const;
	virtual const char *print(const int n=0) const;
	virtual void reset() = 0;
	virtual bool isDefault()  const = 0;
	virtual bool isPrintable() const;
	virtual bool isReadOnly() const { return readOnly; };
	virtual void setDontPrintIfDefault(bool value) { dontPrintIfDefault = value; };
	virtual void setReadOnly(bool value) { readOnly = value; };
	bool operator<(const ent &e) const;
	bool operator==(const ent &e) const;

	//virtual ~ent();
};

/*! Bool storage entity. This entity is used for storing boolean ("ON, "OFF") values. */
class entBool : public ent
{
	public:
	int value;		//!< The current value.
	int defaultValue;	//!< The default value.

	virtual options::event *setValue(const char *arg1, const char *arg2="", const bool justTest=0);
	operator int() const;
	entBool() : ent(NULL), value(false), defaultValue(false) { };
	entBool(const char *n, const bool def) : ent(n), value(def), defaultValue(def) { };

	bool str2int(const char *str, bool &ok) const;
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	bool operator=(int n)	{ value = (n == 1); return value; };
	virtual ~entBool() { };
};

/*! Integer storage. Additionally provides min and max values and will refuse to accept values that
 * are not within the defined boundaries [min, max].
 */
class entInt : public entBool
{
	public:
	int min;	//!< Minimum value.
	int max;	//!< Maximum value.

	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual int str2int(const char *str, bool &ok) const;
	entInt() : min(0), max(0) { name = NULL; };
	entInt(const char *n, const int minimum, const int maximum, const int def)
		: min(minimum), max(maximum) { name = n, value = defaultValue = def; };
	virtual const char *getValue() const;
	virtual const char *getMin() const;
	virtual const char *getMax() const;
	operator int() const;
	bool operator==(int n) const;
	virtual ~entInt() { };
};

/*! Time storage. Saves unix timestamps. */
class entTime : public entInt
{
	public:
	entTime() : entInt() { };
	entTime(const char *n, const int minimum, const int maximum, const int def) :
		entInt(n, minimum, maximum, def) { };

	virtual int str2int(const char *str, bool &ok) const;
	virtual const char *getValue() const;
	virtual const char *getMin() const;
	virtual const char *getMax() const;
	virtual ~entTime() { };
};

/*! Storage for percentage values. */
class entPerc : public entInt
{
	public:
	entPerc() : entInt() { };
	entPerc(const char *n, const int minimum, const int maximum, const int def) :
		entInt(n, minimum, maximum, def) { };

	virtual int str2int(const char *str, bool &ok) const;
	virtual const char *getValue() const;
	virtual const char *getMin() const;
	virtual const char *getMax() const;
	virtual ~entPerc() { };
};

class entHost : public ent
{
	protected:
	int typesAllowed;

	public:
	pstring<8> ip;
	pstring<16> connectionString;

	enum types
	{
		ipv4 = 0x01,		//!< IPv4.
		ipv6 = 0x02,		//!< IPv6.
		bindCheck = 0x04,	//!< Perform bind check.
		domain = 0x08,		//!< Domain.
		use_ssl = 0xf0		//!< Use ssl.
	};

	entHost(const char *n="", const int t=ipv4) : ent(n), typesAllowed(t), ip("0.0.0.0"), connectionString("0.0.0.0") { };
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	virtual operator const char*() const;
	virtual operator unsigned int() const;
	virtual ~entHost() { };
	bool isSSL() { return getConnectionStringType(connectionString) & use_ssl; };
	bool isIpv6() { return getConnectionStringType(connectionString) & ipv6; };
	int getConnectionStringType(const char *str=0) const;


};

/*! String storage entity. Defines minimum and maximum length and refuses to accept strings not
 * matching the boundaries [min, max].
 */
class entString : public ent
{
	public:
	int min;			//!< Minimum string length to accept.
	int max;			//!< Maximum string length to accept.
	pstring <16> string;		//!< The currently stored string.
	pstring <16> defaultString;	//!< The default string.


	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	virtual operator const char*() const;
	virtual int getLen() const;
	entString(const char *n="", const int minimum=0, const int maximum=MAX_INT, const char *def="") :
			ent(n), min(minimum), max(maximum), string(def), defaultString(def) { }
	virtual ~entString() { };
};

/*! Storage for words. Words are like strings, but do not contain blanks. */
class entWord : public entString
{
	public:
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
		entWord(const char *n="", const int minimum=0, const int maximum=MAX_INT, const char *def="")
	: entString(n, minimum, maximum, def) { };

	virtual ~entWord() { };
};

/*! Storage for MD5 hashes. */
class entMD5Hash : public entWord
{
	public:
	unsigned char hash[16];		//!< The hash to store.

	public:
	entMD5Hash(const char *n="") : entWord(n)	{ memset(hash, 0, 16); };
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual void reset();
	virtual bool isDefault() const;
	virtual const unsigned char *getHash() const;
	virtual operator const unsigned char*() const;
	virtual ~entMD5Hash() { };
};

class CONFIG;
/*! Host Port Password Handle storage. This entity is a combination of some others to store more
 * complex information.
 */
class entHPPH : public ent	//host port password handle
{
	protected:
	entHost *_host;		//!< Host information.
	entInt *_port;		//!< Port number.
	entWord *_pass;		//!< Connection password.
	entWord *_handle;	//!< Connection handle.

	virtual options::event *_setValue(const char *arg1, const char *arg2, const char *arg3, const char *arg4, const char *arg5, const bool justTest);

	public:
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	entHPPH(const char *n="", entHost *host=0, entInt *port=0, entWord *pass=0, entWord *handle=0) :
			ent(n), _host(host), _port(port), _pass(pass), _handle(handle) { };
	virtual const char *getValue() const;
	virtual ~entHPPH();
	virtual entHost &getHost()	{ return *_host; };
	virtual entInt &getPort()	{ return *_port; };
	virtual entWord &getPass()	{ return *_pass; };
	virtual entWord &getHandle()	{ return *_handle; };
	virtual entHPPH &operator=(const entHPPH &e);
	virtual void reset();
	virtual bool isDefault() const;

	friend class CONFIG;
	virtual bool isSSL() const	{ return _host->isSSL(); };
};

/*! Stores connection information for connecting to a hub. It differs from entHPPH from the fact
 * that it also stores the number of failed connections, which are not written to config file.
 */
class entHub : public entHPPH
{
	private:

	public:
	int failures;		//!< Number of failed connections.

	entHub(const char *n="", entHost *host=0, entInt *port=0, entMD5Hash *pass=0, entWord *handle=0) :
		entHPPH(n, host, port, pass, handle), failures(0) { };
	virtual ~entHub() { };
};

/*! Connection information for connecting to a server. */
class entServer : public entHPPH
{
	public:
	entServer(const char *n="", entHost *ip=0, entInt *port=0, entWord *pass=0) :
		entHPPH(n, ip, port, pass, 0) { };
	virtual options::event *set(const char *ip, const char *port, const char *pass="", const bool justTest=0);
	virtual ~entServer() { };
};

/*! Multi-entity-storage. This entity stores multiple other entities which don't have to be of the
 * same type. It maintains a list of them. This is used for  storing IRC servers as well as modules.
 */
class entMult : public ent
{
	private:
	ptrlist<ent> list;	//!< The list of entities to store.

	public:
	entMult(const char *n="") : ent(n) { };
	virtual options::event *setValue(const char *arg1, const char *arg2, bool justTest=0);
	virtual const char *getValue() const	{ return NULL; };
	virtual void reset();
	virtual bool isDefault() const		{ return true; };
	virtual bool isPrintable() const	{ return false; };
	virtual void add(ent *e);
	virtual ~entMult() { };
};

/*! Storage for loaded modules. All modules currently loaded are represented by an instance of this
 * class. The class itself does not know wether the module is loaded in debug mode or not.
 */
class entLoadModules : public ent
{
	public:
	entLoadModules(const char *n="", bool needValidMD5=1) : ent(n), m_md5(needValidMD5) { };
	virtual ~entLoadModules() { };

	virtual options::event *setValue(const char *arg1, const char *arg2, bool justTest=0);
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	virtual entLoadModules &operator=(const entLoadModules &e);
	ptrlist<Module>::iterator findModule(const char *str);
	//bool rehash
	bool unload(const char *str);

	/*! Pointer to a function that returns a pstring<> */
	typedef pstring<> (*sfp)();

	/*! Pointer to a voif function */
	typedef void (*vfp);

	/*! Module load function pointer */
	typedef Module* (*mfp)(void *, const char *, const char *, time_t, const char *);

	private:
	pstring<> m_file;	//!< File containing the module.
	pstring<> m_md5sum;	//!< Modules md5 hash.
	time_t m_loadDate;	//!< The time the module was loaded.
	void *m_handle;		//!< dl*-handle to access more functions inside the module.
	bool m_md5;		//!< Wether or not to save the md5 sum.

	virtual options::event *_setValue(const char *arg1, const char *arg2, const char *arg3, bool justTest=0);


};

class entChattr : public ent
{
	private:
	int checkArg(const char *args);
	/* generated by checkArg() (p/m)Flags, key and limit  */
	char gmFlags[32];
	char gpFlags[32];
	char gKey[CHAN_LEN];
	long int gLimit;
	/* current key/limit */
	char Key[CHAN_LEN];
	long int Limit;
	/* default flags */
	char dKey[CHAN_LEN];
	long int dLimit;
	char dmFlags[32];
	char dpFlags[32];

	void setFlags();
	void setFlag(int plusFlag, const char flag);

	public:
	char mFlags[32];
	char pFlags[32];
	bool hasFlag(int minusFlag, const char flag, int Gen) const;
	const char *getKey() const;
	long int getLimit() const;
	virtual options::event *setValue(const char *arg1, const char *arg2, const bool justTest=0);
	virtual const char *getValue() const;
	virtual void reset();
	virtual bool isDefault() const;
	entChattr(const char *n="", const char *modes="") : ent(n)
	{
		memset(pFlags, 0, sizeof(pFlags));
		memset(mFlags, 0, sizeof(mFlags));
		memset(Key, 0, sizeof(Key));
		Limit = 0;

		if(strlen(modes))
		{
			setValue(n, modes, 1);
			strcpy(dpFlags, pFlags);
			strcpy(dmFlags, mFlags);
			strcpy(dKey, Key);
			dLimit = Limit;
		}
		else
		{
			memset(dpFlags, 0, sizeof(dpFlags));
			memset(dmFlags, 0, sizeof(dmFlags));
			memset(dKey, 0, sizeof(dKey));
			dLimit = 0;
		}
	}
	virtual ~entChattr() { };
};

#endif
