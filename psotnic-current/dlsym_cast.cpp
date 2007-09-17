#include "prots.h"

union __cast_dlsym_union
{
	void *ptr;
	DLSYM_FUNCTION fun;
};

DLSYM_FUNCTION dlsym_cast(void *handle, const char *symbol)
{
	__cast_dlsym_union u;
	
	u.ptr = dlsym(handle, symbol);
	return u.fun;
}

