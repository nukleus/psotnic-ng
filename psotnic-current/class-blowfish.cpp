// blowfish.cpp   C++ class implementation of the BLOWFISH encryption algorithm
// _THE BLOWFISH ENCRYPTION ALGORITHM_
// by Bruce Schneier
// Revised code--3/20/94
// Converted to C++ class 5/96, Jim Conger

#include "prots.h"
#include "global-var.h"
#include "blowfish.h2"

#define S(x,i) (SBoxes[i][x.w.byte##i])
#define bf_F(x) (((S(x,0) + S(x,1)) ^ S(x,2)) + S(x,3))
#define ROUND(a,b,n) (a.dword ^= bf_F(b) ^ PArray[n])

CBlowFish::CBlowFish ()
{
	PArray = new DWORD [18] ;
	SBoxes = new DWORD [4][256] ;
	pos = 0;
}

CBlowFish::~CBlowFish ()
{
	delete [] PArray ;
	delete [] SBoxes ;
}

// the low level (private) encryption function
void CBlowFish::Blowfish_encipher (DWORD *xl, DWORD *xr)
{
	union aword  Xl, Xr ;

	Xl.dword = *xl ;
	Xr.dword = *xr ;

	Xl.dword ^= PArray [0];
	ROUND (Xr, Xl, 1) ;  ROUND (Xl, Xr, 2) ;
	ROUND (Xr, Xl, 3) ;  ROUND (Xl, Xr, 4) ;
	ROUND (Xr, Xl, 5) ;  ROUND (Xl, Xr, 6) ;
	ROUND (Xr, Xl, 7) ;  ROUND (Xl, Xr, 8) ;
	ROUND (Xr, Xl, 9) ;  ROUND (Xl, Xr, 10) ;
	ROUND (Xr, Xl, 11) ; ROUND (Xl, Xr, 12) ;
	ROUND (Xr, Xl, 13) ; ROUND (Xl, Xr, 14) ;
	ROUND (Xr, Xl, 15) ; ROUND (Xl, Xr, 16) ;
	Xr.dword ^= PArray [17] ;

	*xr = Xl.dword ;
	*xl = Xr.dword ;
}

// the low level (private) decryption function
void CBlowFish::Blowfish_decipher (DWORD *xl, DWORD *xr)
{
	union aword  Xl ;
	union aword  Xr ;

	Xl.dword = *xl ;
	Xr.dword = *xr ;

	Xl.dword ^= PArray [17] ;
	ROUND (Xr, Xl, 16) ;  ROUND (Xl, Xr, 15) ;
	ROUND (Xr, Xl, 14) ;  ROUND (Xl, Xr, 13) ;
	ROUND (Xr, Xl, 12) ;  ROUND (Xl, Xr, 11) ;
	ROUND (Xr, Xl, 10) ;  ROUND (Xl, Xr, 9) ;
	ROUND (Xr, Xl, 8) ;   ROUND (Xl, Xr, 7) ;
	ROUND (Xr, Xl, 6) ;   ROUND (Xl, Xr, 5) ;
	ROUND (Xr, Xl, 4) ;   ROUND (Xl, Xr, 3) ;
	ROUND (Xr, Xl, 2) ;   ROUND (Xl, Xr, 1) ;
	Xr.dword ^= PArray[0];

	*xl = Xr.dword;
	*xr = Xl.dword;
}

//constructs the enctryption sieve
void CBlowFish::Initialize (BYTE key[], int keybytes)
{
	int  		i, j ;
	DWORD  		data, datal, datar ;
	union aword temp ;

	// first fill arrays from data tables
	for (i = 0 ; i < 18 ; i++)
		PArray [i] = bf_P [i] ;

	for (i = 0 ; i < 4 ; i++)
	{
		for (j = 0 ; j < 256 ; j++)
			SBoxes [i][j] = bf_S [i][j] ;
	}


	j = 0 ;
	for (i = 0 ; i < NPASS + 2 ; ++i)
	{
		temp.dword = 0 ;
		temp.w.byte0 = key[j];
		temp.w.byte1 = key[(j+1) % keybytes] ;
		temp.w.byte2 = key[(j+2) % keybytes] ;
		temp.w.byte3 = key[(j+3) % keybytes] ;
		data = temp.dword ;
		PArray [i] ^= data ;
		j = (j + 4) % keybytes ;
	}

	datal = 0 ;
	datar = 0 ;

	for (i = 0 ; i < NPASS + 2 ; i += 2)
	{
		Blowfish_encipher (&datal, &datar) ;
		PArray [i] = datal ;
		PArray [i + 1] = datar ;
	}

	for (i = 0 ; i < 4 ; ++i)
	{
		for (j = 0 ; j < 256 ; j += 2)
		{
		  Blowfish_encipher (&datal, &datar) ;
		  SBoxes [i][j] = datal ;
		  SBoxes [i][j + 1] = datar ;
		}
	}
}

#ifdef HAVE_BIG_ENDIAN
void CBlowFish::mix(BYTE *str, unsigned long lSize)
{
	unsigned int dx;
	BYTE d0,d1,d2,d3;
	for (dx=0;dx<lSize;dx+=4)
	{
		d0=str[dx];
		d1=str[dx+1];
		d2=str[dx+2];
		d3=str[dx+3];
		str[dx]=d3;
		str[dx+1]=d2;
		str[dx+2]=d1;
		str[dx+3]=d0;
	}
}
#endif

// Encode pIntput into pOutput.  Input length in lSize.  Returned value
// is length of output which will be even MOD 8 bytes.  Input buffer and
// output buffer can be the same, but be sure buffer length is even MOD 8.
DWORD CBlowFish::Encode (BYTE * pInput, BYTE * pOutput, DWORD lSize)
{
	DWORD lCount, lOutSize, dx;
	int SameDest=(pInput==pOutput ? 1 : 0);

	// if input differs from output copy input to output
	if (!SameDest) for (dx=0;dx<lSize;dx++) pOutput[dx]=pInput[dx];
	// if the size isn't dividable by 8 pad the end of output with zeros
	if (lSize%8)
	{
		for (dx=0;dx<8-lSize%8;dx++) pOutput[lSize+dx]=0;
		lOutSize=lSize+dx;
	}
	else lOutSize=lSize;
	// big endian hotfix, reverse the byte order of every 4 byte block
#ifdef HAVE_BIG_ENDIAN
	BYTE *temp=pOutput;
	mix(pOutput,lOutSize);
#endif
	// now we have the correct thing to encode
	for (lCount=0;lCount<lOutSize;lCount+=8)
	{
		Blowfish_encipher((DWORD *)pOutput, (DWORD *)(pOutput+4));
		pOutput+=8;
	}
	// change the order back
#ifdef HAVE_BIG_ENDIAN
	mix(temp,lOutSize);
#endif
	return lOutSize ;
}

// Decode pIntput into pOutput.  Input length in lSize.  Input buffer and
// output buffer can be the same, but be sure buffer length is even MOD 8.
void CBlowFish::Decode (BYTE * pInput, BYTE * pOutput, DWORD lSize)
{
	DWORD lCount,dx;
	int SameDest=(pInput==pOutput ? 1 : 0);

	if (!SameDest) for (dx=0;dx<lSize;dx++) pOutput[dx]=pInput[dx];
#ifdef HAVE_BIG_ENDIAN
	BYTE *temp=pOutput;
	mix(pOutput,lSize);
#endif
	for (lCount=0;lCount<lSize;lCount+=8)
	{
		Blowfish_decipher((DWORD *)pOutput, (DWORD *)(pOutput+4));
		pOutput+=8;
	}
#ifdef HAVE_BIG_ENDIAN
	mix(temp,lSize);
#endif
}

char *CBlowFish::smartDecode(char c, char *output)
{
	buf[pos++] = c;
	if(pos == 8)
	{
		Decode((unsigned char *) buf, (unsigned char *) output, 8);
		pos = 0;
		return output;
	}
	else return NULL;
}
