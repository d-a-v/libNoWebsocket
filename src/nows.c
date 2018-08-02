
#ifndef DEBUG
#define DEBUG 0
#endif

#if DEBUG
#include <stdio.h>
#define DBG(x...) do { x; } while (0)
#else
#define DBG(...) do { (void)0; } while (0)
#endif
#define D(fmt, ...) DBG(fprintf(stderr, fmt, ##__VA_ARGS__); fflush(stderr))

#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "wsc.h"
#include "nows.h"

// This is a random copy-paste of a browser websocket session's key request.
// It is used to simulate a websocket client side (a browser) with
// ws_simulate_client() that should be called once before other nows/read/write().
// May hold an entire valid header if necessary (\r\n between lines, \r\n\r\n at the end).
#define WS_PROTO_INPUT_FAKE	"Sec-WebSocket-Key: YHzu62+PVuCVNksRHhgTdg==\r\n\r\n"

// buffer size to hold the full single line 'Sec-WebSocket-Key: xxxx...' (53 chars?)
#define WS_PROTO_INPUT_SIZE     64

#define MIN2(a,b)   ({ __typeof__(a) _a = (a); __typeof__(b) _b = (b); _b < _a? _b: _a; })
#define MIN3(a,b,c) (MIN2(a, MIN2(b,c)))

struct wsock
{
	int handshake_processing;

	////////////////////////
	// input data

	char input_buffer [WS_PROTO_INPUT_SIZE]; // hold handshake lines and chunks headers
	size_t input_buffer_len;	// valid data length in input buffer
	char mask [WS_MASK_SIZE];	// decoding mask of current chunk
	size_t user_recv_len;		// whole size of user data in current chunk
	size_t user_recv_offs;		// consumed size of user data in current ws chunk
	
	////////////////////////
	// output data

	const char* output;		// pointer to data
	size_t outputlen;		// whole size to send
	off_t  outputoffs;		// offset of next char to send
	char   output_header [WS_PROTO_HEADER_MAX_OUTPUT]; // max size of chunk header
};

#define S2W_ALLOC_BLOCK 8	// allocate wsock 8 by 8

struct s2w
{
	int socket;
	struct wsock* ws;
};

static struct s2w* s2w = NULL;
static int s2wsize = 0;
static int s2walloc = 0;

static struct s2w* wsockentry (int socket, int create)
{
	int i;
	
	// search initialized
	for (i = 0; i < s2wsize; i++)
		if (s2w[i].socket == socket)
			// found
			return &s2w[i];
	
	if (!create)
		return NULL;
	
	// search unused
	for (i = 0; i < s2wsize; i++)
		if (!s2w[i].ws)
			// found, reuse
			break;
	
	if (i == s2wsize)
	{
		// not found, use new place
		if (s2wsize == s2walloc)
		{	
			// need more room
			s2walloc += S2W_ALLOC_BLOCK;
			struct s2w* n = (struct s2w*)realloc(s2w, s2walloc * sizeof(struct s2w));
			if (!n)
				// oom
				return NULL;
			s2w = n;
			// clear new
			memset(&s2w[s2wsize], 0, (s2walloc - s2wsize) * sizeof(struct s2w));
		}
		
	}

	// i is a place for us
	struct wsock* w = s2w[i].ws = (struct wsock*)malloc(sizeof(struct wsock));
	if (!w)
		// oom
		return NULL;
	s2w[i].socket = socket;
	memset(w, 0, sizeof(*w));
	w->handshake_processing = 1;
	if (i == s2wsize)
		s2wsize++;
	return &s2w[i];
}

static struct wsock* wsock (int socket, int create)
{
	struct s2w* s2w = wsockentry(socket, create);
	return s2w? s2w->ws: NULL;
}

int nowsclose (int sock)
{
	struct s2w* s2w = wsockentry(sock, 0);
	if (s2w)
	{
		free(s2w->ws);
		s2w->socket = -1;
		s2w->ws = NULL;
	}
	return close(sock);
}

int nows_simulate_client (int sock)
{
	struct wsock* w = wsock(sock, 1);
	if (!w)
	{
		errno = ENOMEM;
		return -1;
	}
	
	// (start to) send fake browser websocket header
	
	static const char fake [] = WS_PROTO_INPUT_FAKE;
	
	ssize_t ret = write(sock, fake, sizeof fake - 1);
	if (ret == -1)
		// errno is set
		return ret;
	
	w->output = fake + ret;
	w->outputlen = sizeof fake - 1 - ret;
	return 0;
}

#if DEBUG
static void dump (const void* data, size_t len)
{
	D("->%d:", (int)len);
	for (size_t i = 0; i < len; i++)
	{
		unsigned char c = ((char*)data)[i];
		D("%c",(c>=32&&c<=127)? c: '.');
	}
	D("<-");
}
#endif

static int ws_internal_send (int sock, struct wsock* w)
{
	D("(nows(%d)/intwrite:", sock);
	ssize_t ret = write(sock, w->output + w->outputoffs, w->outputlen - w->outputoffs);
	D("%d)", (int)ret);

	if (ret > 0)
	{
		w->outputoffs += ret;
		if ((size_t)w->outputoffs == w->outputlen)
		{
			// all data are sent
			w->outputoffs = 0;
			w->outputlen = 0;

			if (w->output != w->output_header)
			{
				// output was pointing to allocated handshake response
				// release its memory
				free((void*)w->output);
				w->output = w->output_header;
			}
			return 0;
		}
		
		// protocol data not fully sent
		errno = EAGAIN;
		return -1;
	}
	
	return ret;
}

ssize_t nowswrite (int sock, const void* data, size_t len)
{
	if (len == 0)
		return 0;

	struct wsock* w = wsock(sock, 1);
	if (!w)
	{
		errno = ENOMEM;
		return -1;
	}

	if (w->handshake_processing)
	{
		// in blocking mode, a call to nowsread will allow handshake to be processed.
		(void)nowsread(sock, NULL, 0);

		if (w->handshake_processing)
		{
			errno = EAGAIN;
			return -1;
		}
	}

	D("\nnowswrite(%d):", sock);

	if (!w->outputlen)
	{
		// prepare protocol header
		w->output = w->output_header;
		w->outputoffs = 0;
		w->outputlen = ws_set_output_preamble(len, w->output_header);
	}

	if (ws_internal_send(sock, w) < 0)
		// currently dealing with protocol details
		// come back later
		// errno is set
		return -1;
		
	// send user data
	D("(nowswrite:sock=%d,len=%d,ret=", sock, (int)len);
	ssize_t ret = write(sock, data, len);
	D("%d)\n", (int)ret);
	return ret;
}

static const char* mystrnstr (const char* big, const char* needle, size_t len)
{
	size_t needlelen = strlen(needle);
	if (needlelen == 0)
		return big;
	if (needlelen > len)
		return NULL;
	const char* after = big + len - needlelen + 1;
	for (const char* test = big; test != after; test++)
		if (memcmp(test, needle, needlelen) == 0)
			return test;
	return NULL;
}

static void wscheckheader (struct wsock* w)
{
	// decode + extract length from it
	char* userdata;
	size_t userlen = ws_decode_header(w->input_buffer, w->input_buffer_len, &userdata, w->mask);

	DBG(if (w->input_buffer_len && !userlen) dump(w->input_buffer, w->input_buffer_len));
	if (userlen)
	{
		D("(newchunklen=%d,has=%d)", (int)userlen, (int)w->input_buffer_len);

		// full data header decoded
		w->user_recv_len = userlen;
		w->user_recv_offs = 0;

		// move user data to the beginning of the header buffer,
		// user data in header is handled above
		w->input_buffer_len -= userdata - w->input_buffer;
		memmove(w->input_buffer, userdata, w->input_buffer_len);
		D("(decoded:remain=%d)", (int)w->input_buffer_len);
	}
}

static void wsextractheaderdata (struct wsock* w, void* data, size_t len, off_t* dataoffs)
{
	size_t swallow = MIN3(w->input_buffer_len, len - *dataoffs, w->user_recv_len - w->user_recv_offs);
	D("(input_buffer_len=%d datalen=%d chunkremain=%d -> %d)", (int)w->input_buffer_len, (int)(len - *dataoffs), (int)(w->user_recv_len - w->user_recv_offs), (int)swallow);
	if (!swallow)
		return;

	memcpy(data + *dataoffs, w->input_buffer, swallow);
	D("(unmask in header %d)", (int)swallow);
	ws_unmask(data + *dataoffs, swallow, w->user_recv_offs, w->mask);
	*dataoffs += swallow;
	if ((w->user_recv_offs += swallow) == w->user_recv_len)
	{
		w->user_recv_offs = 0;
		w->user_recv_len = 0;
	}
	// move next header data to beginning of w->input_buffer
	if ((w->input_buffer_len -= swallow))
		memmove(w->input_buffer, w->input_buffer + swallow, w->input_buffer_len);
}

ssize_t nowsread (int sock, void* data, size_t len)
{
	struct wsock* w = wsock(sock, 1);
	if (!w)
	{
		errno = ENOMEM;
		return -1;
	}
	
	if (len == 0 && !w->handshake_processing)
		return 0;
	
	D("\nnowsread(sock=%d,len=%d)\n", sock, (int)len);

	while (w->handshake_processing)
	{
		if (w->outputlen)
			// manage protocol data first
			// * initial handshake must be sent prior beeing able to receive data
			// * user's requests to nowswrite() must be sent asap
			(void)ws_internal_send(sock, w);

		D("(nows(%d):handshake)", sock);
		// search presence of crlf
		const char* crlf = NULL;
		while (1)
		{
			if ((crlf = mystrnstr(w->input_buffer, "\r\n", w->input_buffer_len)))
				break;

			// need to read full crlf line in buffer
			size_t maxlen = WS_PROTO_INPUT_SIZE - w->input_buffer_len;
			if (maxlen == 0)
			{
				// the interesting header is in a small line
				// current line is too big thus not interesting,
				// we can clear it and patiently wait for crlf
				w->input_buffer_len = 0;
				maxlen = WS_PROTO_INPUT_SIZE;
				D("(handshake:discard)");
			}

			D("(read handshake(max%d)=", (int)maxlen);
			ssize_t ret = read(sock, w->input_buffer + w->input_buffer_len, maxlen);
			D("%d)", (int)ret);
			if (ret <= 0)
				// end of file, or non-blocking-nothing-to-read
				return ret;
			w->input_buffer_len += ret;
			DBG(dump(w->input_buffer, w->input_buffer_len));
		}
		// crlf is found
		D("(handshake:crlf)");

		if (!w->output)
		{
			// handshake response not processed yet
			// is curent line the key to update ?
			const char* handshake = ws_handshake(w->input_buffer);

			if (handshake)
			{
				D("(handshake:answer:->%s<-)", handshake);
				// set up output with handshake response
				w->output = handshake;
				w->outputlen = strlen(handshake);
				w->outputoffs = 0;
				// now wait for an empty line
			}
		}
		
		// remove current line and its crlf
		w->input_buffer_len -= crlf + 2 - w->input_buffer;
		memmove(w->input_buffer, crlf + 2, w->input_buffer_len);

		if (w->output && crlf == w->input_buffer)
		{
			// we are done with handshake:
			// - output has handshake response
			// - last line was empty
			w->handshake_processing = 0;
			
			if (w->input_buffer_len == 0)
			{
				// In blocking mode, nowsread() can have
				// been called on user POLLIN without any
				// payload sent by peer, but because of
				// handshake taking place at first.  Now
				// that handshake is processed, *and*
				// nothing is in buffer, we shall get out
				// telling to come back later.
				errno = EAGAIN;
				return -1;
				
				// otherwise, continue with data already
				// present in input buffer.
			}
			
			break;
		}

		// continue with handshake
	}
			
	// handshake is behind us
	
	off_t dataoffs = 0; // output buffer (=user data)'s write offset
	while (1)
	{
		if (w->outputlen)
			// manage protocol data first
			// * initial handshake must be sent prior beeing able to receive data
			// * user's requests to nowswrite() must be sent asap
			(void)ws_internal_send(sock, w);

		if (w->user_recv_len)
		{
			// data chunk already started
			
			D("(chunk:%d/%d)", (int)w->user_recv_offs, (int)w->user_recv_len);
		
			if (w->input_buffer_len)
			{
				// process header or remaining data in header buffer

				D("(have %d in buffers)", (int)w->input_buffer_len);
				wsextractheaderdata(w, data, len, &dataoffs);

				if (w->input_buffer_len && !w->user_recv_len)
					// still data in buffer but current chunk emptied
					// then we have a new chunk (with possibly enough header)
					wscheckheader(w);

				if ((size_t)dataoffs == len)
					// user buffer full
					return dataoffs;

				if (w->input_buffer_len)
					D("(process new header)");
					// process (new?) header
					continue;
			}

			assert(!w->input_buffer_len);
			assert((size_t)dataoffs != len);
			assert(w->user_recv_len > 0 && w->user_recv_offs < w->user_recv_len);

			// read directly in user's buffer
			size_t allowed_len = MIN2(len - dataoffs, w->user_recv_len - w->user_recv_offs);
			D("read(%d)=", (int)allowed_len);
			ssize_t ret = read(sock, data + dataoffs, allowed_len);
			D("%d)", (int)ret);
			if (ret <= 0)
			{
				assert(ret == -1 || dataoffs);
				return dataoffs?: ret;
			}
			
			// unmask
			D("(unmask %d@+%d)", (int)ret, (int)w->user_recv_offs);
			ws_unmask(data + dataoffs, ret, w->user_recv_offs, w->mask);
			
			w->user_recv_offs += ret;
			dataoffs += ret;

			if (w->user_recv_offs == w->user_recv_len)
			{
				// done with this chunk
				w->user_recv_len = 0;
				w->user_recv_offs = 0;

				// return chunk when fully loaded even if user asked for more data
				// (this breaks posix's retval in blocking mode)
				D("(full chunk given, ret=%d)", (int)dataoffs);
				DBG(dump(data, len));
				assert(dataoffs != 0);
				return dataoffs;
			}
			
			if ((size_t)dataoffs == len)
			{
				// user's buffer full
				D("(filled user buffer, still %d/%d in chunk)", (int)w->user_recv_offs, (int)w->user_recv_len);
				DBG(dump(data, len));
				assert(dataoffs != 0);
				return dataoffs;
			}

			D("(reading more)");
		}
		else
		{
			D("(nochunk:has=%d,read(%d):", (int)w->input_buffer_len, (int)(WS_PROTO_INPUT_SIZE - w->input_buffer_len));
			
			// read ws chunk's header
			ssize_t ret = read(sock, w->input_buffer + w->input_buffer_len, WS_PROTO_INPUT_SIZE - w->input_buffer_len);
			D("%d)", (int)ret);
			if (ret <= 0)
				return dataoffs?: ret;

			w->input_buffer_len += ret;
			// decode + extract length from it
			wscheckheader(w);
		}
	}
}
