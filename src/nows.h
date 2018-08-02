
// websocket API is flawed,
// * unnecessary complexity, any packet analyzer can unmask frames
// * connected socket could also be a stream
// * we have standard socket APIs, why not making them available in browsers too?
// interesting links found afterwards:
// https://news.ycombinator.com/item?id=3377406
// https://samsaffron.com/archive/2015/12/29/websockets-caution-required

#ifndef __WSPOSIX_H
#define __WSPOSIX_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h> // *size_t

ssize_t nowsread  (int sock,       void* data, size_t len);
ssize_t nowswrite (int sock, const void* data, size_t len);
int     nowsclose (int sock);

int nows_simulate_client (int sock);

#ifdef __cplusplus
} // extern "C"
#endif

#endif // __WSPOSIX_H
