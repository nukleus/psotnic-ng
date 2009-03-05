#ifndef ADNSPTHREAD_HPP
#define ADNSPTHREAD_HPP 

#ifdef HAVE_ADNS_PTHREAD

#include "Adns.hpp"

class adns_pthread : public adns
{
	private:
	bool die;

	pthread_mutex_t data_mutex;
	pthread_mutex_t condition_mutex;
	pthread_cond_t condition;

	pthread_t *th;
	int poolSize;

	void work();
	void removePool();
	void killThreads();

	public:
	virtual void resolv(const char *host);
	virtual host2ip *getIp(const char *host);
	void setupPool(int n);
	adns_pthread(int n);
	virtual ~adns_pthread();
	
	void expire(time_t t, time_t now);
	void lock_data();
	void unlock_data();

#ifdef HAVE_DEBUG
	void display();
#endif

	friend void *__adns_work(void *);
	friend class client;
};
#endif

#endif /* ADNSPTHREAD_HPP */
