
/*

Copyright (C) 2013 Adam Rudd

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to
deal in the Software without restriction, including without limitation the
rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.

*/

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
