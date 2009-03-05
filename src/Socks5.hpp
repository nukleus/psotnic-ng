#ifndef SOCKS5_HPP
#define SOCKS5_HPP 

class asyn_socks5
{
	char proxyip[16];
	unsigned short proxyport;
	char remotehost[256];
	unsigned short remoteport;
	int step;

	int i;
	int toRead;
	int fd;
	char buf[515];
	unsigned char len;
	unsigned char atyp;

	public:
	asyn_socks5();
	void setup(const char *pip, unsigned short pport, const char *rhost, unsigned short rport);
	int connect();
	void disconnect();
	int work(char byte);
	int use();
};

#endif /* SOCKS5_HPP */
