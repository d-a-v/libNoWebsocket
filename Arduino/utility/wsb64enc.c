
#include "wsb64enc.h"

#if 0
static const char b64_alphabet[] =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
#define alphabet(x) b64_alphabet[x]
#else

static inline char alphabet(char x)
{
	return x < 26 ? x + 'A' : x < 52 ? x - 26 + 'a' : x < 62 ? x - 52 + '0' : x == 62 ? '+' : '/';
}

#endif

void a3_to_a4(unsigned char* a4, unsigned char* a3)
{
	a4[0] = (a3[0] & 0xfc) >> 2;
	a4[1] = ((a3[0] & 0x03) << 4) + ((a3[1] & 0xf0) >> 4);
	a4[2] = ((a3[1] & 0x0f) << 2) + ((a3[2] & 0xc0) >> 6);
	a4[3] = (a3[2] & 0x3f);
}

int base64_encode(char* output, const char* input, int inputLen)
{
	int i = 0, j = 0;
	int encLen = 0;
	unsigned char a3[3];
	unsigned char a4[4];

	while (inputLen--)
	{
		a3[i++] = *(input++);
		if (i == 3)
		{
			a3_to_a4(a4, a3);

			for (i = 0; i < 4; i++)
				output[encLen++] = alphabet(a4[i]);

			i = 0;
		}
	}

	if (i)
	{
		for (j = i; j < 3; j++)
			a3[j] = '\0';

		a3_to_a4(a4, a3);

		for (j = 0; j < i + 1; j++)
			output[encLen++] = alphabet(a4[j]);

		while (i++ < 3)
			output[encLen++] = '=';
	}
	output[encLen] = '\0';
	return encLen;
}
