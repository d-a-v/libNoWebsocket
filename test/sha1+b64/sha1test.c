
#include <stdio.h>
#include <string.h>

#include <wssha1.h>

int main (void)
{
	const char in [] = "abc";

	if (sha1init() == 0)
		return 1;
	
	sha1update(in, strlen(in));
	
	const char* r = sha1result();
	
	printf("    ");
	for (int i = 0; i < SHA1LENGTH; i++)
		printf("%02x", (unsigned char)r[i]);
	printf(" - should be:\n(0x)a9993e364706816aba3e25717850c26c9cd0d89d (echo -n abc | openssl sha1)\n");
	
	return 0;
}
