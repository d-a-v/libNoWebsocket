#ifndef _BASE64_H
#define _BASE64_H

static inline int base64_enc_len(int n)
{
	return (n + 2 - ((n + 2) % 3)) / 3 * 4;
}

int base64_encode(char* output, const char* input, int inputLen);

#endif // _BASE64_H
