
#include "prots.h"
#include "global-var.h"

void MD5Hash(unsigned char digest[16], const char *data, int datalen, const unsigned char *key, int keylen)
{
	CUSTOM_MD5_CTX md5;
	unsigned char p[64];
	int n;

	if(keylen > 64) keylen = 64;


	MD5Init(&md5);

	for(n=0; datalen; )
	{
		if(datalen > 64)
		{
			memcpy(p, data, 64);
			datalen -= 64;
		}
		else
		{
			memset(p, 0, 64);
			memcpy(p, data, datalen);
			datalen = 0;
		}
		MD5Update(&md5, p, 64);
		data += 64;
	}

	if(keylen)
	{
		memset(p, 0, 64);
		memcpy(p, key, keylen);
		MD5Update(&md5, p, 64);
	}

	MD5Final(digest, &md5);
}

void MD5HexHash(char digest[33], const char *data, int datalen, const unsigned char *key, int keylen)
{
	unsigned char buf[16];

	memset(digest, 0, 33);
	MD5Hash(buf, data, datalen, key, keylen);

	quoteHexStr(buf, digest, 16);
}

void MD5CreateAuthString(char *str, int len)
{
	int i;

	srand();
	memset(str, 0, len);

	/* len must be even */
	for(i=0; i<len/2; ++i)
		sprintf(str, "%s%02x", str, abs(rand() % 255+1));

	str[len] = '\0';
}

int MD5HexValidate(const char digest[33], const char *data, int datalen, const unsigned char *key, int keylen)
{
	char hash[33];

	MD5HexHash(hash, data, datalen, key, keylen);
	if(!strcmp(hash, digest))
		return 1;
	else return 0;
}

int MD5Validate(const unsigned char digest[16], const char *data, int datalen, const unsigned char *key, int keylen)
{
	unsigned char hash[16];
	int i;

	MD5Hash(hash, data, datalen, key, keylen);
	for(i=0; i<16; ++i)
		if(hash[i] != digest[i]) return 0;

	return 1;
}


int MD5HashFile(unsigned char digest[16], const char *file, const unsigned char *key, int keylen)
{
	int fd;

	if((fd = open(file, O_RDONLY)) == -1)
		 return -1;

	return MD5HashFile(digest, fd, key, keylen);
}

int MD5HashFile(unsigned char digest[16], int fd, const unsigned char *key, int keylen)
{
	unsigned int len;
	unsigned char buf[64];
	CUSTOM_MD5_CTX md5;

	MD5Init(&md5);

	while(1)
	{
		len = read(fd, buf, 64);
		MD5Update(&md5, buf, len);
		if(len != 64) break;
	}
	if(keylen)
	{
		if(keylen > 64) keylen = 64;
		memset(buf, 0, 64);
		memcpy(buf, key, keylen);
	 	MD5Update(&md5, buf, keylen);
	}
	MD5Final(digest, &md5);
	close(fd);
	return 1;
}

char itoa_4bit(int x)
{
	if(x < 0) return '!';

	if(x < 10)
		return '0' + x;

	if(x < 16)
		return 'a' + x - 10;

	return '#';
}

char* itoa_8bit(int n, char *buf)
{
	buf[0] = itoa_4bit((n & (255 - 15)) >> 4);
	buf[1] = itoa_4bit(n & 15);
	buf[2] = '\0';

	return buf;
}

unsigned char *quoteHex(const char *str, unsigned char *hex)
{
	char buf[3];
	unsigned char *ret = hex;
	while(*str)
	{
		buf[0] = *str++;
		buf[1] = *str++;
		buf[2] = '\0';
		
		*hex++ = (unsigned char) strtol(buf, 0, 16);
	}
	return ret;
}

char *quoteHexStr(const unsigned char *hex, char *str, int len)
{
	memset(str, 0, len*2);

	for(int i=0; i<len; ++i)
		itoa_8bit(abs(hex[i]), str+i*2);

	return str;
}
