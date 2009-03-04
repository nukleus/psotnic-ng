#ifndef OPTIONS_HPP
#define OPTIONS_HPP 

#include "pstring.h"
#include "ptrlist.h"

class inetconn;
class ent;

class options
{
	public:
	class event
	{
		public:
		pstring<> reason;
		bool ok;
		bool notFound;
		ent *entity;

		void setOk(ent *e, const char *format, ...);
		void setError(ent *e, const char *format, ...);
		void setError(ent *e);
		void setNotFound(const char *format, ...);
		event();
	};

	ptrlist<ent> list;

	options();

	event *setVariable(const char *var, const char *value);
	const char *getValue(const char *var);
	void sendToOwner(const char *owner, const char *var, const char *prefix);
	bool parseUser(const char *from, const char *var, const char *value, const char *prefix, const char *prefix2="");
	void reset();
	void sendToFile(inetconn *c, pstring<> prefix);

#ifdef HAVE_DEBUG
	void display();
#endif

	protected:
	void registerObject(const ent &e);
	int maxVarLen;
};

extern options::event _event;

#endif /* OPTIONS_HPP */
