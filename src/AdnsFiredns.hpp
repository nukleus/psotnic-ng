#ifndef ADNSFIREDNS_HPP
#define ADNSFIREDNS_HPP 

#ifdef HAVE_ADNS_FIREDNS

#include "Adns.hpp"

class adns_firedns : public adns
{
	private:
	time_t last_check;
	bool shouldWeCheck();

	public:
	virtual void resolv(const char *host);
	virtual host2ip *getIp(const char *host);
	adns_firedns();
	virtual ~adns_firedns();
	
	void expire(time_t t, time_t now);

#ifdef HAVE_DEBUG
	void display();
#endif

	int fillFDSET(fd_set *set);
	void processResultSET(fd_set *set);
	void closeAllConnections();
};
#endif

#endif /* ADNSFIREDNS_HPP */
