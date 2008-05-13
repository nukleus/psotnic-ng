#include <stdio.h>
#include "seed.h"

void gen_seed_function(unsigned char *seed, const char *name)
{
	unsigned char crc = 0xff;
	int i=0;

	for(i=0; i<16; ++i)
		crc ^= seed[i];

	int mult = (crc % 16)  + 5;

	printf("void gen_%s_seed(unsigned char *out)\n", name);
	printf("{\n");
	printf("    int i;\n");

	for(i=0; i<16; ++i)
		printf("    out[%d] = %d;\n", i, seed[i] ^ crc);

	printf("\n");

	printf("    for(i=0; i<16; ++i)\n");
	printf("    {\n");
	printf("        out[i] ^= %d;\n", crc);
	printf("    }\n");
	printf("}\n\n");
}

int main()
{
	gen_seed_function(cfg_seed, "cfg");
	gen_seed_function(ul_seed, "ul");
	
	return 0;
}
