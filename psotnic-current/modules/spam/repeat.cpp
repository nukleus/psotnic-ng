
#include "prots.h"
#include "global-var.h"
#include "repeat.h"

repeat::repeat() : CustomDataObject(), when(0), creation(NOW)
{
}

repeat::~repeat()
{
}

bool repeat::hit(const char *s)
{
	if(when + 10 >= NOW && !strcmp(str, s))
	{
		when = NOW;
		str = s;
		return true;
	}
	str =  s;
	when = NOW;

	return false;
}

