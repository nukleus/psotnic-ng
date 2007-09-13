// blowfish.h     interface file for blowfish.cpp
// _THE BLOWFISH ENCRYPTION ALGORITHM_
// by Bruce Schneier
// Revised code--3/20/94
// Converted to C++ class 5/96, Jim Conger

#ifndef BRUCE_SCHNEIER_BLOWFISH_H
#define BRUCE_SCHNEIER_BLOWFISH_H 1

#define MAXKEYBYTES 	56		// 448 bits max
#define NPASS           16		// SBox passes

#define DWORD  		uint32_t
#define WORD  		uint16_t
#define BYTE  		uint8_t

class CBlowFish
{
	protected:
	DWORD *PArray;
	DWORD (*SBoxes)[256];
	void Blowfish_encipher (DWORD *xl, DWORD *xr);
	void Blowfish_decipher (DWORD *xl, DWORD *xr);
	char buf[8];
	int pos;
#ifdef HAVE_BIG_ENDIAN
	void mix(BYTE *,unsigned long);
#endif

	public:
	CBlowFish();
	~CBlowFish();
	void Initialize (BYTE key[], int keybytes);
	DWORD GetOutputLength (DWORD lInputLong);
	DWORD Encode (BYTE * pInput, BYTE * pOutput, DWORD lSize);
	void Decode (BYTE * pInput, BYTE * pOutput, DWORD lSize);
	char *smartDecode(char c, char *output);
};

#ifndef HAVE_BIG_ENDIAN
	union aword {
	  DWORD dword;
	  BYTE byte [4];
	  struct {
	    unsigned int byte3:8;
	    unsigned int byte2:8;
	    unsigned int byte1:8;
	    unsigned int byte0:8;
	  } w;
	};
#else
	union aword {
	  DWORD dword;
	  BYTE byte [4];
	  struct {
	    unsigned int byte0:8;
	    unsigned int byte1:8;
	    unsigned int byte2:8;
	    unsigned int byte3:8;
	  } w;
	};

#endif

#endif

