#ifndef HTTP_HPP
#define HTTP_HPP 

#include "Inetconn.hpp"

class http
{
	public:
	class url
	{
		public:
		char *link;
		char *host;
		char *file;
		char *filepath;
		char *ip;
		int port;

		url(const char *u);
		~url();
		int ok();

#ifdef HAVE_DEBUG
		void print();
#endif
	};

	enum errorTypes {
		ERR_OK        =  0,
		ERR_EOF       = -1,
		ERR_URL       = -2,
		ERR_DST       = -3,
		ERR_CONN      = -4,
		ERR_HEADER    = -5,
		ERR_DST_WRITE = -6,
		ERR_READ      = -7
	};

	inetconn www;
	char *data;

	int get(const char *link, int estSize=4096);
	int get(const char *link, const char *file);

	int get(url &u, int estSize=4096);
	int get(url &u, const char *file);

	static char *error(int n);

	http();
	~http();

	private:
	int sendRequest(url &u);
	int processHeader();

};

#endif /* HTTP_HPP */
