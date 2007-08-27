/*
 * This is an OpenSSL-compatible implementation of the RSA Data Security,
 * Inc. MD5 Message-Digest Algorithm.
 *
 * Written by Solar Designer <solar@openwall.com> in 2001, and placed in
 * the public domain.  See md5c.c for more information.
 */

#ifndef _MD5_H
#define _MD5_H

/* Any 32-bit or wider integer data type will do */
typedef unsigned long MD5_u32plus;

struct CUSTOM_MD5_CTX {
	MD5_u32plus lo, hi;
	MD5_u32plus a, b, c, d;
	unsigned char buffer[64];
	MD5_u32plus block[16];
};

void MD5Init(CUSTOM_MD5_CTX *ctx);
void MD5Update(CUSTOM_MD5_CTX *ctx, const unsigned char *data, unsigned long size);
void MD5Final(unsigned char *result, CUSTOM_MD5_CTX *ctx);

void MD5Hash(unsigned char digest[16], const char *data, int datalen, const unsigned char *key=NULL, int keylen=0);
void MD5HexHash(char digest[33], const char *data, int datalen, const unsigned char *key=NULL, int keylen=0);
void MD5CreateAuthString(char *str, int len);
int MD5HexValidate(const char digest[33], const char *data, int datalen, const unsigned char *key=NULL, int keylen=0);
int MD5Validate(const unsigned char digest[16], const char *data, int datalen, const unsigned char *key=NULL, int keylen=0);
int MD5HashFile(unsigned char digest[16], const char *file, const unsigned char *key=NULL, int keylen=0);
int MD5HashFile(unsigned char digest[16], int fd, const unsigned char *key=NULL, int keylen=0);


#endif
