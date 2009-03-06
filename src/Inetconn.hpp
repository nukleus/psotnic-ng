#ifndef INETCONN_HPP
#define INETCONN_HPP 

#ifdef HAVE_SSL
	#include <openssl/ssl.h>
	#include <openssl/err.h>
#endif

#include <netinet/in.h> // AF_INET

#include "structs.h"

class CBlowFish;
struct HANDLE;

class inetconn
{
	private:
	void _close(const char *reason=NULL);
	CBlowFish *blowfish;
	void lock();
	void unlock();

	public:
	int fd;
	int status;
	char *name;
	char *pass;
	char *tmpstr;
	char *origin;
	int tmpint;
	int killTime;
	int lastPing;
#ifdef HAVE_SSL
	SSL_CTX *ssl_ctx;
	SSL *ssl;
#endif

	IOBUF read;
	IOBUF write;

	HANDLE *handle;

	int enableLameCrypt();
	int enableCrypt(const unsigned char *key, int len=-1);
	int enableCrypt(const char *key, int len=-1);
	int disableCrypt();
#ifdef HAVE_SSL
	bool enableSSL();
	void SSLHandshake();
#endif
	int send(const char *lst, ...);
	int va_send(va_list ap, const char *lst);
	int readln(char *buf, int len, int *ok=NULL);
	int open(const char *pathname, int flags, mode_t mode=0);
	void close(const char *reason=NULL);

	int checkFlag(int flag, int where=GLOBAL);

	int sendPing();
	int timedOut();
	unsigned int getPeerIp4();
	unsigned int getMyIp4();
	const char *getPeerIpName();
	const char *getMyIpName();
	char *getPeerPortName();
	char *getMyPortName();

	//int getPort();
	void echo(int n, char *app=NULL);
	inetconn();
	~inetconn();

	int isSlave();
	int isLeaf();
	int isMain();
	int isRegBot();
	int isRegOwner();
	int isRegUser();
	int isRedir();
	int isReg();
	int isConnected();
	int isBot();
	int isSSL();
	
	void writeBufferedData();

	friend class inet;// closeConn(int fd);
};

#endif /* INETCONN_HPP */
