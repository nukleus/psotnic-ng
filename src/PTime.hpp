#ifndef PTIME_HPP
#define PTIME_HPP 

class ptime
{
	public:
	struct timeval tv;
	ptime();
	ptime(const char *s, const char *n);
	ptime(time_t s, time_t n);
	char *print();
	char *ctime();
	int operator==(ptime &p);
};

#endif /* PTIME_HPP */
