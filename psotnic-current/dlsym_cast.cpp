#include "prots.h"

#ifdef HAVE_MODULES
union __cast_dlsym_union
{
	DLSYM_OBJECT obj;
	DLSYM_FUNCTION fun;
};

DLSYM_FUNCTION obj2fun(DLSYM_OBJECT o)
{
	__cast_dlsym_union u;
	u.obj = o;
	return u.fun;
}

DLSYM_OBJECT fun2obj(DLSYM_FUNCTION f)
{
	__cast_dlsym_union u;
	u.fun = f;
	return u.obj;
}

DLSYM_FUNCTION dlsym_cast(void *handle, const char *symbol)
{
	return obj2fun(dlsym(handle, symbol));
}

#endif
