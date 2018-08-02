
#include <stdio.h>
#include <string.h>

#include <wsb64enc.h>

int main (void)
{
	const char in [] = "abcdef";
	char b64 [base64_enc_len(strlen(in)) + 1];
	
	if (!base64_encode(b64, in, strlen(in)))
		return 1;
	printf("%s - should be:\nYWJjZGVm (echo -n abcdef | base64)\n", b64);
	
	return 0;
}
