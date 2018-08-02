#ifndef Sha1_h
#define Sha1_h

#include <stddef.h> // size_t

#define SHA1LENGTH 20

int    sha1init(void);		// return 0 on failure
void   sha1update(const void*, size_t);
void*  sha1result(void);
void   sha1deinit(void);

#endif
